// BenchTimer — per-frame wall-time collector used by Playground bench mode.
//
// Mirrors the DawnTest framework/bench.h API so the emitted "BENCH …" line
// is identical across both apps and tools/bench/run-bench.mjs can parse a
// single format.

#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace Bench
{
    struct FrameStats
    {
        int    frameCount = 0;   // included frames (excludes warmup)
        double wallMs     = 0;
        double minMs      = 0;
        double avgMs      = 0;
        double maxMs      = 0;
        double p95Ms      = 0;
    };

    class FrameTimer
    {
    public:
        // Default warmup is 1 frame (shader compile + first-use upload is
        // almost always an outlier).
        void SetWarmupFrames(int n);
        void Reserve(std::size_t n);

        void StartFrame();
        void EndFrame();

        FrameStats Finish() const;

        // Emits exactly:
        //   BENCH scene=<sceneName> frames=N wall_ms=X min_ms=X avg_ms=X max_ms=X p95_ms=X\n
        // on stdout, then flushes.
        void PrintBenchLine(const std::string& sceneName) const;

    private:
        int                 m_warmupFrames = 1;
        int                 m_seenFrames   = 0;
        double              m_startMs      = 0;
        std::vector<double> m_deltas;
    };

    double MonotonicMillis();
}
