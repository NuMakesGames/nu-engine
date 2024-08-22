#include <iostream>

#include "NuEngine/Engine.h"
#include "NuEngine/Assertions.h"
#include "NuEngine/Console.h"
#include "NuEngine/ConsoleRenderer.h"
#include "NuEngine/Stopwatch.h"

#include <thread>
#include <chrono>

using namespace nu::console;
using namespace std::literals::chrono_literals;

int main()
{
	nu::profiling::Stopwatch sw;
	std::vector<std::chrono::milliseconds> frameTimes{};
	std::cout << "Starting simulation.\n";



	{
		ConsoleRenderer renderer;
		for (int i = 0; i < 20; ++i)
		{
			sw.Restart();
			renderer.Clear();
			renderer.Draw(i % 5, i, '*', vt::color::ForegroundBrightCyan);
			renderer.Draw((i + 1) % 5, i, 'o', vt::color::ForegroundBrightGreen);
			renderer.Present();
			sw.Stop();

			frameTimes.push_back(sw.ElapsedMilliseconds());
			std::this_thread::sleep_for(20ms - sw.ElapsedMilliseconds());
		}
	}

	for (const auto& frameTime : frameTimes)
	{
		std::cout << frameTime << "\n";
	}

	fnEngine();
}