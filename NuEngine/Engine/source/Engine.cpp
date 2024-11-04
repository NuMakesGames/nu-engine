#include "NuEngine/Engine.h"

#include <chrono>
#include <thread>

#include "NuEngine/Assertions.h"
#include "NuEngine/ConsoleEventStream.h"
#include "NuEngine/ConsoleRenderer.h"
#include "NuEngine/Game.h"
#include "NuEngine/Stopwatch.h"

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
			m_engine->SetDesiredRenderSize(x, y);
		}

	private:
		class Engine* m_engine;
	};

	Engine::Engine()
	{
	}

	void Engine::StartGame(Game& game)
	{
		game.SetEngine(this);
		game.BeginPlay();

		EscapeKeyConsumer keyConsumer(this);
		WindowResizeConsumer resizeConsumer(this);
		ConsoleEventStream eventStream;
		eventStream.RegisterKeyboardInputConsumer(&keyConsumer);
		eventStream.RegisterWindowResizeConsumer(&resizeConsumer);

		ConsoleRenderer renderer;
		m_renderSizeX = renderer.GetWidth();
		m_renderSizeY = renderer.GetHeight();

		Stopwatch frameTimer;
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
		eventStream.UnregisterWindowResizeConsumer(&resizeConsumer);
		eventStream.UnregisterKeyboardInputConsumer(&keyConsumer);

	}

	void Engine::StopGame()
	{
		m_shouldStopGame = true;
	}

	void Engine::SetDesiredRenderSize(uint16_t x, uint16_t y) noexcept
	{
		m_renderSizeX = x;
		m_renderSizeY = y;
	}
} // namespace engine
} // namespace nu
