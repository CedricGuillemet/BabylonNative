#pragma once

#include <optional>
#include <string>
#include <vector>

struct PlaygroundOptions
{
    bool ParseError = false;
    std::string ErrorMessage;

    bool ShowHelp = false;
    bool ListTests = false;
    bool Headless = false;
    bool BreakOnFail = false;
    bool GenerateReferences = false;
    bool RunOnce = false;
    bool IncludeExcluded = false;

    std::optional<bool> SaveResults;
    std::optional<bool> DebugTrace;
    std::optional<std::string> PerfTrace;

    // Benchmark mode.
    //
    // When `Frames > 0`, the host's render loop exits after that many
    // rendered frames (counted post-warmup by the BenchTimer) and emits a
    // single line of the form:
    //   BENCH scene=<name> frames=N wall_ms=X min_ms=X avg_ms=X max_ms=X p95_ms=X
    // on stdout, parsed by tools/bench/run-bench.mjs. Setting `Frames`
    // implicitly enables `NoVsync` so the timing reflects actual GPU/CPU
    // throughput rather than the display refresh cap.
    //
    // `NoVsync` may also be set independently of `Frames` for interactive
    // perf testing.
    int Frames = 0;
    bool NoVsync = false;

    // 1-based frame index at which to call TestUtils.captureNextFrame()
    // (RenderDoc capture trigger). When set, the runner extends each test's
    // render budget so the .rdc finalizes. Requires renderdoc.dll to be
    // injected externally (e.g. via `renderdoccmd capture` or
    // `rdc capture --trigger`); bgfx auto-adopts an already-loaded DLL.
    std::optional<int> CaptureFrame;

    std::vector<std::string> TestFilters;
    std::vector<int> TestIndices;

    // Positional script arguments (URLs / paths).
    std::vector<std::string> Scripts;
};

namespace CommandLine
{
    // Parses argv (argv[0] is treated as the program name and ignored).
    // On a parse error, returns an options struct with ParseError == true
    // and ErrorMessage populated.
    PlaygroundOptions Parse(int argc, const char* const* argv);

    // Prints usage / help text to stdout. argv0 may be nullptr.
    void PrintUsage(const char* argv0);
}
