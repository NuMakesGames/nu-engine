#include "NuEngine/Engine.h"

#include <chrono>
#include <thread>

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

	Engine::Engine()
	{
	}

	void Engine::StartGame(Game& game)
	{
		game.SetEngine(this);
		game.BeginPlay();

		EscapeKeyConsumer keyConsumer(this);
		ConsoleEventStream eventStream;
		eventStream.RegisterKeyboardInputConsumer(&keyConsumer);

		Stopwatch frameTimer;
		ConsoleRenderer renderer;
		while (!m_shouldStopGame)
		{
			auto deltaTime = frameTimer.ElapsedSeconds();
			frameTimer.Restart();

			// Process input and window resize events
			eventStream.ProcessEvents();

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
		eventStream.UnregisterKeyboardInputConsumer(&keyConsumer);
	}

	void Engine::StopGame()
	{
		m_shouldStopGame = true;
	}
} // namespace engine
} // namespace nu
