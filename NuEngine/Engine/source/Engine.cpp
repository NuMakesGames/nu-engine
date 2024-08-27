#include "NuEngine/Engine.h"

#include <chrono>
#include <thread>

#include "NuEngine/ConsoleRenderer.h"
#include "NuEngine/Game.h"
#include "NuEngine/Stopwatch.h"

using namespace nu::console;
using namespace std::chrono_literals;

namespace nu
{
namespace engine
{
	Engine::Engine()
	{
	}

	void Engine::StartGame(Game& game)
	{
		game.SetEngine(this);
		game.BeginPlay();

		Stopwatch frameTimer;
		ConsoleRenderer renderer;
		while (!m_shouldStopGame)
		{
			auto deltaTime = frameTimer.ElapsedSeconds();
			frameTimer.Restart();

			// Update the simulation
			game.Tick(deltaTime);

			// Render the frame
			renderer.Clear();
			game.Render(renderer);
			renderer.Present();

			// Busy wait until the next frame
			constexpr int targetFramesPerSecond = 60;
			constexpr double targetFrameTime = 1.f / targetFramesPerSecond;
			while (targetFrameTime > frameTimer.ElapsedSeconds().count())
			{
				// Spin as sleep functions do not have necessary precision (~15ms on Windows)
			}

			frameTimer.Stop();
		}

		game.EndPlay();
		game.SetEngine(nullptr);
	}

	void Engine::StopGame()
	{
		m_shouldStopGame = true;
	}
} // namespace engine
} // namespace nu
