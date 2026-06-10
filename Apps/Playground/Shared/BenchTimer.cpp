#include "BenchTimer.h"

#include <algorithm>
#include <chrono>
#include <cstdio>

namespace Bench
{
    double MonotonicMillis()
    {
        using namespace std::chrono;
        static const auto kEpoch = steady_clock::now();
        return duration<double, std::milli>(steady_clock::now() - kEpoch).count();
    }

    void FrameTimer::SetWarmupFrames(int n)
    {
        m_warmupFrames = n < 0 ? 0 : n;
    }

    void FrameTimer::Reserve(std::size_t n)
    {
        m_deltas.reserve(n);
    }

    void FrameTimer::StartFrame()
    {
        m_startMs = MonotonicMillis();
        ++m_seenFrames;
    }

    void FrameTimer::EndFrame()
    {
        if (m_seenFrames <= m_warmupFrames)
        {
            return;
        }
        m_deltas.push_back(MonotonicMillis() - m_startMs);
    }

    FrameStats FrameTimer::Finish() const
    {
        FrameStats s{};
        if (m_deltas.empty())
        {
            return s;
        }

        std::vector<double> sorted(m_deltas);
        std::sort(sorted.begin(), sorted.end());

        double sum = 0.0;
        for (double d : sorted)
        {
            sum += d;
        }

        const std::size_t n = sorted.size();
        std::size_t pIdx = static_cast<std::size_t>(0.95 * n);
        if (pIdx >= n)
        {
            pIdx = n - 1;
        }

        s.frameCount = static_cast<int>(n);
        s.wallMs     = sum;
        s.minMs      = sorted.front();
        s.avgMs      = sum / static_cast<double>(n);
        s.maxMs      = sorted.back();
        s.p95Ms      = sorted[pIdx];
        return s;
    }

    void FrameTimer::PrintBenchLine(const std::string& sceneName) const
    {
        const FrameStats s = Finish();
        std::fprintf(stdout,
            "BENCH scene=%s frames=%d wall_ms=%.3f min_ms=%.3f avg_ms=%.3f max_ms=%.3f p95_ms=%.3f\n",
            sceneName.c_str(),
            s.frameCount,
            s.wallMs,
            s.minMs,
            s.avgMs,
            s.maxMs,
            s.p95Ms);
        std::fflush(stdout);
    }
}
