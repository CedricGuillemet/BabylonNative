#!/usr/bin/env bash
#
# Android visualization-test runner for the GitHub-hosted emulator job
# (see .github/workflows/ci.yml -> Android_VisualizationTests).
#
# reactivecircus/android-emulator-runner executes its `script` input
# line-by-line, so multi-line shell constructs (functions, loops,
# backslash continuations) must live in a real script file invoked as a
# single line rather than inline in the workflow.
#
# Runs once the emulator is booted: installs the x86_64 Playground APK,
# launches the pixel-diff suite via an Intent, and derives pass/fail from
# the "Run complete. ... failed=N" logcat line (Android's TestUtils.exit()
# is a no-op, so completion is detected from the log, not an exit code).
set -u

PKG="${PKG:-com.android.babylonnative.playground}"
ACTIVITY="${ACTIVITY:-com.android.babylonnative.playground/.PlaygroundActivity}"
APK="${APK:-playground-android-visualization-tests.apk}"

# Every adb call is wrapped in `timeout` so a wedged emulator (adb "device
# offline" that never recovers) can never block the script — an earlier
# version hung indefinitely in `adb wait-for-device`, so the overall deadline
# never fired and the job ran until the hard job timeout with zero diagnostics.
ADB() { timeout 60 adb "$@"; }

# Collect logcat + result/diff PNGs into ./android-results regardless of how
# the run ends (crash, timeout, or pixel failures) so the upload step always
# has artifacts. The emulator only lives for the duration of the action, so
# collection must happen here.
collect() {
  mkdir -p android-results
  ADB logcat -d > android-results/logcat-full.txt 2>/dev/null || true
  ADB logcat -d -s BabylonNative > android-results/logcat-babylonnative.txt 2>/dev/null || true
  ADB root >/dev/null 2>&1 || true
  ADB wait-for-device >/dev/null 2>&1 || true
  ADB pull "/data/data/$PKG/cache/Results" android-results/ 2>/dev/null || true
  ADB pull "/data/data/$PKG/cache/Errors" android-results/ 2>/dev/null || true
}
trap collect EXIT

# Start clean: force-stop any prior instance and clear the log buffer so a
# stale "Run complete" line can't be mistaken for this run.
ADB shell am force-stop "$PKG" || true
ADB logcat -c || true
ADB uninstall "$PKG" >/dev/null 2>&1 || true

echo "Installing $APK"
adb install -g "$APK"

# Launch the suite. renderWidth/renderHeight are LOGICAL units; the app scales
# them by display density (then the Embedding layer divides back) so the
# backbuffer lands at 600x400 on any emulator density, matching the references.
# continueOnFailure runs the whole suite instead of aborting at the first
# failure.
adb shell am start -n "$ACTIVITY" \
  --es scripts "app:///Scripts/validation_native.js" \
  --ei renderWidth 600 --ei renderHeight 400 \
  --ez continueOnFailure true

# All adb calls in the poll loop are wrapped in `timeout` so a wedged emulator
# (adb "device offline" that never recovers) can never block the loop — a
# previous iteration hung indefinitely in `adb wait-for-device`, so the overall
# deadline never fired and the job ran until the hard 60-min timeout with zero
# diagnostics.
ADB() { timeout 60 adb "$@"; }
get_summary() { ADB logcat -d -s BabylonNative 2>/dev/null | grep -m1 'Run complete\.' || true; }
# Per-test progress: the runner logs "Test 'X' validated" / "Test 'X' failed".
count_done() { ADB logcat -d -s BabylonNative 2>/dev/null | grep -cE "Test '.*' (validated|failed)" 2>/dev/null || echo 0; }
last_line()  { ADB logcat -d -s BabylonNative 2>/dev/null | tail -1 || true; }

# Poll logcat for the runner's summary line. The app does not self-terminate on
# Android, so completion is detected from the log. A heartbeat prints progress
# every iteration so the CI console shows how far the suite got; a stall
# watchdog (no new BabylonNative log line for STALL_LIMIT seconds) catches both
# a crashed app (logs stop) and a wedged emulator without blocking.
DEADLINE=$(( $(date +%s) + 4200 ))   # 70 min overall budget
STALL_LIMIT=600                      # 10 min with no new log => hung/crashed
START=$(date +%s)
STALL_SINCE=$START
LAST_SEEN=""
SUMMARY=""
while [ "$(date +%s)" -lt "$DEADLINE" ]; do
  SUMMARY=$(get_summary)
  [ -n "$SUMMARY" ] && break
  NOW=$(date +%s)
  DONE=$(count_done)
  CUR=$(last_line)
  if [ -n "$CUR" ] && [ "$CUR" != "$LAST_SEEN" ]; then
    LAST_SEEN="$CUR"
    STALL_SINCE=$NOW
  fi
  STALLED=$(( NOW - STALL_SINCE ))
  # Trim the log-line prefix for a compact heartbeat.
  echo "[heartbeat] t=$(( NOW - START ))s done=$DONE stalled=${STALLED}s last: $(printf '%s' "${CUR##*BabylonNative: }" | cut -c1-140)"
  if [ "$STALLED" -ge "$STALL_LIMIT" ]; then
    # One last chance for a summary that landed between polls.
    SUMMARY=$(get_summary)
    [ -n "$SUMMARY" ] && break
    echo "No BabylonNative log progress for ${STALLED}s (last completed test count=$DONE) — emulator/app appears hung or crashed." >&2
    ADB logcat -d -s BabylonNative 2>/dev/null | tail -60 || true
    exit 3
  fi
  sleep 15
done

if [ -z "$SUMMARY" ]; then
  echo "Timed out after $(( $(date +%s) - START ))s waiting for the validation suite to finish (completed test count=$(count_done))." >&2
  ADB logcat -d -s BabylonNative 2>/dev/null | tail -60 || true
  exit 1
fi

echo "$SUMMARY"
failed=$(echo "$SUMMARY" | sed -n 's/.*failed=\([0-9]*\).*/\1/p')
echo "Parsed failed=$failed"
[ "${failed:-1}" = "0" ]
