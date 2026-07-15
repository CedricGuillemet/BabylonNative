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

# Collect logcat + result/diff PNGs into ./android-results regardless of how
# the run ends (crash, timeout, or pixel failures) so the upload step always
# has artifacts. The emulator only lives for the duration of the action, so
# collection must happen here.
collect() {
  mkdir -p android-results
  adb logcat -d > android-results/logcat-full.txt 2>/dev/null || true
  adb logcat -d -s BabylonNative > android-results/logcat-babylonnative.txt 2>/dev/null || true
  adb root >/dev/null 2>&1 || true
  adb wait-for-device >/dev/null 2>&1 || true
  adb pull "/data/data/$PKG/cache/Results" android-results/ 2>/dev/null || true
  adb pull "/data/data/$PKG/cache/Errors" android-results/ 2>/dev/null || true
}
trap collect EXIT

# Start clean: force-stop any prior instance and clear the log buffer so a
# stale "Run complete" line can't be mistaken for this run.
adb shell am force-stop "$PKG" || true
adb logcat -c || true
adb uninstall "$PKG" >/dev/null 2>&1 || true

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

# Poll logcat for the runner's summary line. The app does not self-terminate
# on Android, so we detect completion from the log.
DEADLINE=$(( $(date +%s) + 2400 ))
SUMMARY=""
while [ "$(date +%s)" -lt "$DEADLINE" ]; do
  SUMMARY=$(adb logcat -d -s BabylonNative 2>/dev/null | grep -m1 'Run complete\.' || true)
  [ -n "$SUMMARY" ] && break
  # Bail early if the app process vanished before writing a summary.
  if ! adb shell pidof "$PKG" >/dev/null 2>&1; then
    sleep 5
    SUMMARY=$(adb logcat -d -s BabylonNative 2>/dev/null | grep -m1 'Run complete\.' || true)
    [ -n "$SUMMARY" ] && break
    echo "App process exited before writing a run summary — likely a crash." >&2
    adb logcat -d -s BabylonNative 2>/dev/null | tail -50 || true
    exit 3
  fi
  sleep 10
done

if [ -z "$SUMMARY" ]; then
  echo "Timed out waiting for the validation suite to finish." >&2
  adb logcat -d -s BabylonNative 2>/dev/null | tail -50 || true
  exit 1
fi

echo "$SUMMARY"
failed=$(echo "$SUMMARY" | sed -n 's/.*failed=\([0-9]*\).*/\1/p')
echo "Parsed failed=$failed"
[ "${failed:-1}" = "0" ]
