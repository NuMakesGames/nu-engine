#include "NuEngine/Engine.h"

#include <chrono>
#include <thread>

#include "NuEngine/Assertions.h"
#include "NuEngine/ConsoleEventStream.h"
#include "NuEngine/ConsoleRenderer.h"
#include "NuEngine/Game.h"
#include "NuEngine/Stopwatch.h"

#define NOMINMAX
#include "Windows.h"

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
		// Set the timer precision to 1ms during play
		::timeBeginPeriod(1);

		ConsoleRenderer renderer;
		m_renderSizeX = renderer.GetWidth();
		m_renderSizeY = renderer.GetHeight();

		m_game = &game;
		game.SetEngine(this);
		game.BeginPlay();

		ConsoleEventStream eventStream;
		eventStream.RegisterKeyboardInputConsumer(this);
		eventStream.RegisterWindowResizeConsumer(this);

		Stopwatch frameTimer;
		Stopwatch tickTimer;
		Stopwatch renderTimer;
		Stopwatch presentTimer;
		Stopwatch idleTimer;
		while (!m_shouldStopGame)
		{
			auto deltaTime = frameTimer.ElapsedSeconds();
			frameTimer.Restart();

			// Process input and window resize events
			eventStream.ProcessEvents();

			// Resize the renderer if necessary
			if (renderer.GetWidth() != m_renderSizeX || renderer.GetHeight() != m_renderSizeY)
			{
				renderer.Resize(m_renderSizeX, m_renderSizeY);
				m_game->OnWindowResize(m_renderSizeX, m_renderSizeY);
			}

			// Update the simulation
			tickTimer.Restart();
			game.Tick(deltaTime);
			tickTimer.Stop();

			// Draw the game
			renderTimer.Restart();
			renderer.Clear();
			game.Render(renderer);
			renderTimer.Stop();

			// Present to the console
			presentTimer.Restart();
			renderer.Present();
			presentTimer.Stop();

			// Idle until the next frame. Sleep until within 1.5 ms, then yield until within 0.1 ms, then busy-wait.
			idleTimer.Restart();
			auto targetFrameTime =
				std::chrono::microseconds(static_cast<int>(1.f / m_targetFramesPerSecond * 1'000'000));
			while (targetFrameTime > frameTimer.ElapsedDuration())
			{
				auto remainingTime = targetFrameTime - frameTimer.ElapsedMilliseconds();
				constexpr auto spinThreshold = 1.5ms;
				constexpr auto yieldThreshold = 0.1ms;
				if (remainingTime > spinThreshold)
				{
					std::this_thread::sleep_for(remainingTime - spinThreshold);
				}
				else if (remainingTime > yieldThreshold)
				{
					std::this_thread::yield();
				}
			}
			idleTimer.Stop();
			frameTimer.Stop();

			m_lastFrameTime = frameTimer.ElapsedSeconds();
			m_lastTickTime = tickTimer.ElapsedSeconds();
			m_lastRenderTime = renderTimer.ElapsedSeconds();
			m_lastPresentTime = presentTimer.ElapsedSeconds();
			m_lastIdleTime = idleTimer.ElapsedSeconds();
		}

		game.EndPlay();
		game.SetEngine(nullptr);
		m_game = nullptr;

		eventStream.UnregisterWindowResizeConsumer(this);
		eventStream.UnregisterKeyboardInputConsumer(this);

		// Reset the timer precision to the default
		::timeEndPeriod(1);
	}

	void Engine::StopGame()
	{
		m_shouldStopGame = true;
	}

	void Engine::OnWindowResize(uint16_t x, uint16_t y)
	{
		SetDesiredRendererSize(x, y);
	}

	bool Engine::OnKeyDown(Key key)
	{
		if (m_game->OnKeyDown(key))
		{
			return true;
		}

		if (key == Key::Escape)
		{
			StopGame();
			return true;
		}

		return false;
	}

	bool Engine::OnKeyUp(Key key)
	{
		return m_game->OnKeyUp(key);
	}

	void Engine::SetDesiredRendererSize(uint16_t x, uint16_t y) noexcept
	{
		m_renderSizeX = x;
		m_renderSizeY = y;
	}
} // namespace engine
} // namespace nu
