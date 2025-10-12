#include "NuEngine/Engine.h"

#include <chrono>
#include <string_view>
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
using namespace std::literals;

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

			// Make sure input mode is set correctly based on whether the commander is currently enabled
			if (m_isCommanderEnabled && eventStream.GetKeyInputMode() == KeyInputMode::Keys)
			{
				eventStream.SetKeyInputMode(KeyInputMode::Lines);
			}
			else if (!m_isCommanderEnabled && eventStream.GetKeyInputMode() == KeyInputMode::Lines)
			{
				eventStream.SetKeyInputMode(KeyInputMode::Keys);
			}

			// Process input and window resize events
			eventStream.ProcessEvents();

			// Check if commander should be dismissed; this can't use OnKeyDown events, because commander disables them
			if (m_isCommanderEnabled)
			{
				const auto& rawLine = eventStream.GetCurrentLineRaw();
				if (rawLine.find(static_cast<wchar_t>(Key::Escape)) != std::wstring::npos
				    || rawLine.find(static_cast<wchar_t>(Key::GraveAccent)) != std::wstring::npos)
				{
					m_isCommanderEnabled = false;
				}
			}

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

			// Draw the commander
			if (m_isCommanderEnabled)
			{
				for (uint16_t x = 0; x < m_renderSizeX; ++x)
				{
					renderer.DrawChar(x, m_renderSizeY - 1, ' ', vt::color::ForegroundBlack, vt::color::BackgroundBrightBlue);
				}

				renderer.DrawString(0, m_renderSizeY - 1, "> ", vt::color::ForegroundBrightWhite, vt::color::BackgroundBrightBlue);
				renderer.DrawU8String(2, m_renderSizeY - 1, eventStream.GetCurrentLine(), vt::color::ForegroundBrightWhite, vt::color::BackgroundBrightBlue);
			}

			// Render FPS counter, if enabled
			if (m_showFps)
			{
				int fps = static_cast<int>(std::round(1.f / m_lastFrameTimings.totalFrameTime.count()));
				std::string fpsString = std::format("{} FPS", fps);
				int x = std::max(0, m_renderSizeX - static_cast<int>(fpsString.size()) - 1);

				constexpr int yOffset = 3;
				int y = std::max(0, m_renderSizeY / 4 - yOffset);

				renderer.DrawString(x, y, fpsString, vt::color::ForegroundBrightCyan);
			}

			// Render frame timing stats, if enabled
			if (m_showFrameTimings)
			{
				constexpr auto frameTimeLabel =   "Frame:   "sv;
				constexpr auto tickTimeLabel =    "Tick:    "sv;
				constexpr auto renderTimeLabel =  "Render:  "sv;
				constexpr auto presentTimeLabel = "Present: "sv;
				constexpr auto idleTimeLabel =    "Idle:    "sv;
				constexpr auto labelLength = static_cast<uint16_t>(frameTimeLabel.size());

				auto toMs = [](const auto& duration) { return std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(duration).count(); };
				auto frameTime = std::format("{:>5.2f}ms", toMs(m_lastFrameTimings.totalFrameTime));
				auto tickTime = std::format("{:>5.2f}ms", toMs(m_lastFrameTimings.tickTime));
				auto renderTime = std::format("{:>5.2f}ms", toMs(m_lastFrameTimings.renderTime));
				auto presentTime = std::format("{:>5.2f}ms", toMs(m_lastFrameTimings.presentTime));
				auto idleTime = std::format("{:>5.2f}ms", toMs(m_lastFrameTimings.idleTime));

				int timingLength = static_cast<int>(std::max({ frameTime.size(), tickTime.size(), renderTime.size(), presentTime.size(), idleTime.size() }));
				int x = std::max(0, m_renderSizeX - static_cast<int>(labelLength) - timingLength - 1);

				constexpr int yOffset = 2;
				int y = std::max(0, m_renderSizeY / 4 - yOffset);

				renderer.DrawString(x, y, frameTimeLabel);
				renderer.DrawString(x + labelLength, y, frameTime, vt::color::ForegroundBrightWhite);

				auto tickColor = tickTime >= renderTime && tickTime >= presentTime ? vt::color::ForegroundBrightYellow : vt::color::ForegroundBrightWhite;
				renderer.DrawString(x, ++y, tickTimeLabel);
				renderer.DrawString(x + labelLength, y, tickTime, tickColor);

				auto renderColor = renderTime >= tickTime && renderTime >= presentTime ? vt::color::ForegroundBrightYellow : vt::color::ForegroundBrightWhite;
				renderer.DrawString(x, ++y, renderTimeLabel);
				renderer.DrawString(x + labelLength, y, renderTime, renderColor);

				auto presentColor = presentTime >= tickTime && presentTime >= renderTime ? vt::color::ForegroundBrightYellow : vt::color::ForegroundBrightWhite;
				renderer.DrawString(x, ++y, presentTimeLabel);
				renderer.DrawString(x + labelLength, y, presentTime, presentColor);

				renderer.DrawString(x, ++y, idleTimeLabel);
				renderer.DrawString(x + labelLength, y, idleTime, vt::color::ForegroundBrightWhite);
			}

			// Present to the console
			presentTimer.Restart();
			renderer.Present();
			presentTimer.Stop();

			// Idle until the next frame. Sleep until within 1.5 ms, then yield until within 0.1 ms, then busy-wait.
			idleTimer.Restart();
			if (m_targetFramesPerSecond > 0)
			{
				auto targetFrameTime = std::chrono::microseconds(static_cast<int>(1.f / m_targetFramesPerSecond * 1'000'000));
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
			}
			idleTimer.Stop();
			frameTimer.Stop();

			m_lastFrameTimings.totalFrameTime = frameTimer.ElapsedSeconds();
			m_lastFrameTimings.tickTime = tickTimer.ElapsedSeconds();
			m_lastFrameTimings.renderTime = renderTimer.ElapsedSeconds();
			m_lastFrameTimings.presentTime = presentTimer.ElapsedSeconds();
			m_lastFrameTimings.idleTime = idleTimer.ElapsedSeconds();
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

		if (key == Key::GraveAccent)
		{
			m_isCommanderEnabled = true;
			return true;
		}

		return false;
	}

	bool Engine::OnKeyUp(Key key)
	{
		return m_game->OnKeyUp(key);
	}

	bool Engine::OnLineInput(const std::u8string& line)
	{
		if (m_game->OnLineInput(line))
		{
			return true;
		}

		if (line == u8"quit" || line == u8"exit")
		{
			StopGame();
			return true;
		}

		if (line == u8"fps")
		{
			m_showFps = !m_showFps;
			return true;
		}

		if (line == u8"stats" || line == u8"timings")
		{
			m_showFrameTimings = !m_showFrameTimings;
			return true;
		}

		return false;
	}

	void Engine::SetDesiredRendererSize(uint16_t x, uint16_t y) noexcept
	{
		m_renderSizeX = x;
		m_renderSizeY = y;
	}
} // namespace engine
} // namespace nu
