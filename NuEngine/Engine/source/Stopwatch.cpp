#include "NuEngine/Stopwatch.h"

namespace nu
{
namespace profiling
{

	Stopwatch::Stopwatch() : m_isRunning(false)
	{
	}

	void Stopwatch::Start()
	{
		if (m_isRunning)
		{
			return;
		}

		// Only set start time on first start or after Reset is called
		if (m_startTime == std::chrono::high_resolution_clock::time_point{})
		{
			m_startTime = std::chrono::high_resolution_clock::now();
		}

		m_isRunning = true;
	}

	void Stopwatch::Stop()
	{
		if (!m_isRunning)
		{
			return;
		}

		m_endTime = std::chrono::high_resolution_clock::now();
		m_isRunning = false;
	}

	void Stopwatch::Reset()
	{
		m_isRunning = false;
		m_startTime = std::chrono::high_resolution_clock::time_point{};
		m_endTime = m_startTime;
	}

	void Stopwatch::Restart()
	{
		m_isRunning = true;
		m_startTime = std::chrono::high_resolution_clock::now();
	}

	auto Stopwatch::ElapsedDuration() const
	{
		return m_isRunning ? std::chrono::high_resolution_clock::now() - m_startTime : m_endTime - m_startTime;
	}

	std::chrono::milliseconds Stopwatch::ElapsedMilliseconds() const
	{
		return std::chrono::duration_cast<std::chrono::milliseconds>(ElapsedDuration());
	}

	bool Stopwatch::IsRunning() const
	{
		return m_isRunning;
	}
} // namespace profiling
} // namespace nu