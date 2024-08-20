#include <iostream>

#include "NuEngine/Engine.h"
#include "NuEngine/Assertions.h"
#include "NuEngine/Console.h"
#include "NuEngine/ConsoleRenderer.h"

#include <thread>
#include <chrono>

using namespace nu::console;
using namespace std::literals::chrono_literals;

int main()
{
	ConsoleRenderer renderer;
	
	for (int i = 0; i < 20; ++i)
	{
		renderer.Clear();
		renderer.Draw(i % 5, i, '*', vt::color::ForegroundBrightCyan);
		renderer.Draw((i + 1) % 5, i, 'o', vt::color::ForegroundBrightGreen);
		renderer.Present();

		std::this_thread::sleep_for(20ms);
	}

	fnEngine();
}