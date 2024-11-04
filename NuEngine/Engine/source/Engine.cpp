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
	class EscapeKeyConsumer : public IKeyboardInputConsumer
	{
	public:
		EscapeKeyConsumer(Engine* engine)
		{
			VerifyElseCrash(engine != nullptr);
			m_engine = engine;
		}

		bool OnKeyDown(Key key)
		{
			if (key == Key::Escape)
			{
				m_engine->StopGame();
				return true;
			}

			return false;
		}

		bool OnKeyUp(Key key)
		{
			return false;
		}

	private:
		class Engine* m_engine;
	};

	class WindowResizeConsumer : public IWindowResizeConsumer
	{
	public:
		WindowResizeConsumer(Engine* engine)
		{
			VerifyElseCrash(engine != nullptr);
			m_engine = engine;
		}

		void OnWindowResize(uint16_t x, uint16_t y)
		{
			m_engine->SetDesiredRendererSize(x, y);
		}

	private:
		class Engine* m_engine;
	};

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

		game.SetEngine(this);
		game.BeginPlay();

		EscapeKeyConsumer keyConsumer(this);
		WindowResizeConsumer resizeConsumer(this);
		ConsoleEventStream eventStream;
		eventStream.RegisterKeyboardInputConsumer(&keyConsumer);
		eventStream.RegisterWindowResizeConsumer(&resizeConsumer);
		eventStream.RegisterKeyboardInputConsumer(&game);
		eventStream.RegisterWindowResizeConsumer(&game);

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
		eventStream.UnregisterWindowResizeConsumer(&game);
		eventStream.UnregisterKeyboardInputConsumer(&game);
		eventStream.UnregisterWindowResizeConsumer(&resizeConsumer);
		eventStream.UnregisterKeyboardInputConsumer(&keyConsumer);

		// Reset the timer precision to the default
		::timeEndPeriod(1);
	}

	void Engine::StopGame()
	{
		m_shouldStopGame = true;
	}

	void Engine::SetDesiredRendererSize(uint16_t x, uint16_t y) noexcept
	{
		m_renderSizeX = x;
		m_renderSizeY = y;
	}
} // namespace engine
} // namespace nu
