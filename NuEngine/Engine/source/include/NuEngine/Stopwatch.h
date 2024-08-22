#pragma once

#include <chrono>

namespace nu
{
namespace profiling
{
	class Stopwatch
	{
	public:
		Stopwatch();

		// Starts, or resumes, measuring elapsed time for an interval
		void Start();

		// Stops measuring elapsed time for an interval
		void Stop();

		// Stops time interval measurement and resets the elapsed time to zero
		void Reset();

		// Stops time interval measurement, resets the elapsed time to zero, and starts measuring elapsed time
		void Restart();

		// Returns the elapsed time as a duration (for use with std::chrono::duration_cast)
		auto ElapsedDuration() const;

		// Returns the elapsed time in milliseconds.
		std::chrono::milliseconds ElapsedMilliseconds() const;

		// Checks if the stopwatch is running
		bool IsRunning() const;

	private:
		std::chrono::high_resolution_clock::time_point m_startTime;
		std::chrono::high_resolution_clock::time_point m_endTime;
		bool m_isRunning = false;
	};
} // namespace profiling
} // namespace nu
