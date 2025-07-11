#include <algorithm>
#include <chrono>
#include <deque>
#include <format>
#include <iostream>
#include <string>
#include <string_view>

#include "NuEngine/ConsoleRenderer.h"
#include "NuEngine/Engine.h"
#include "NuEngine/VirtualTerminalSequences.h"

using namespace nu::console;
using namespace std::chrono_literals;
using namespace std::literals;

class Snow : public nu::engine::Game
{
public:
	Snow()
	{
	}

	void OnWindowResize(uint16_t width, uint16_t height) override
	{
		m_x = width / 2;
		m_snowflakes.resize(width);
	}

	bool OnKeyDown(Key key) override
	{
		switch (key)
		{
			case Key::Space:
				// Spawn snowflake
				m_snowflakes[m_x] = { 1, 0ms };
				return true;
			case Key::A:
			case Key::Left:
				// Move spawner left
				if (m_currentMovement > -1)
				{
					--m_currentMovement;
				}
				return true;
			case Key::D:
			case Key::Right:
				// Move spawner right
				if (m_currentMovement < 1)
				{
					++m_currentMovement;
				}
				return true;
			case Key::P:
				// Toggle autoplay
				m_autoplayEnabled = !m_autoplayEnabled;
				return true;
			default:
				return false;
		}
	}

	bool OnKeyUp(Key key) override
	{
		switch (key)
		{
			case Key::A:
			case Key::Left:
				// Stop moving the spawner left
				if (m_currentMovement < 1)
				{
					++m_currentMovement;
				}
				return true;
			case Key::D:
			case Key::Right:
				// Stop moving the spawner right
				if (m_currentMovement > -1)
				{
					--m_currentMovement;
				}
				return true;
			default:
				return false;
		}
	}

	bool OnLineInput(const std::u8string& line)
	{
		if (line == u8"autoplay")
		{
			m_autoplayEnabled = !m_autoplayEnabled;
			return true;
		}

		return false;
	}

	void BeginPlay() override
	{
		GetEngine()->SetTargetFramesPerSecond(240);
		auto [width, height] = GetEngine()->GetRendererSize();
		m_x = width / 2;
		m_snowflakes.resize(width);
		std::ranges::fill(m_snowflakes, Snowflake{ -1, 0ms });
	}

	void Tick(std::chrono::duration<double> deltaTime) override
	{
		if (m_autoplayEnabled)
		{
			TickAutoplay(deltaTime);
		}

		m_x += m_currentMovement;
		m_x = std::clamp(m_x, 0, static_cast<int>(m_snowflakes.size() - 1));

		// Move all snowflakes forward
		for (auto& snowflake : m_snowflakes)
		{
			// Ignore inactive snowflakes
			if (snowflake.y == -1)
			{
				continue;
			}

			// Move the snowflake
			const auto timePerMovement = 100ms;
			snowflake.accruedTime += std::chrono::duration_cast<std::chrono::milliseconds>(deltaTime);
			if (snowflake.accruedTime < timePerMovement)
			{
				continue;
			}

			snowflake.accruedTime -= timePerMovement;
			++snowflake.y;

			// Deactivate snowflake if it falls off the screen
			if (snowflake.y - 5 >= GetEngine()->GetRendererHeight())
			{
				snowflake.y = -1;
				snowflake.accruedTime = 0ms;
			}
		}
	}

	void Render(nu::console::ConsoleRenderer& renderer) override
	{
		// Draw spawner
		renderer.DrawU8Char(m_x, 0, u8"▼", vt::color::ForegroundBrightWhite);

		// Draw snowflakes
		for (uint16_t x = 0; x < m_snowflakes.size(); ++x)
		{
			const auto& snowflake = m_snowflakes[x];
			if (snowflake.y == -1)
			{
				continue;
			}

			renderer.DrawU8Char(x, snowflake.y, u8"❄", vt::color::ForegroundBrightCyan);
			for (int j = 1; snowflake.y - j >= 1 && j < 5; ++j)
			{
				renderer.DrawU8Char(x, snowflake.y - j, u8"•", vt::color::ForegroundCyan);
			}
		}

		// Draw stats
		constexpr auto frameTimeLabel = "Frame: "sv;
		constexpr auto tickTimeLabel = "Tick: "sv;
		constexpr auto renderTimeLabel = "Render: "sv;
		constexpr auto presentTimeLabel = "Present: "sv;
		constexpr auto idleTimeLabel = "Idle: "sv;
		constexpr auto x = static_cast<uint16_t>(presentTimeLabel.size());

		auto renderStat =
			[&renderer, x](uint16_t y, std::string_view label, auto time, auto budget, bool shouldInvertColor)
		{
			auto color = time <= budget ? vt::color::ForegroundBrightGreen : vt::color::ForegroundBrightRed;
			if (shouldInvertColor)
			{
				color = color == vt::color::ForegroundBrightGreen ? vt::color::ForegroundBrightRed
				                                                  : vt::color::ForegroundBrightGreen;
			}

			renderer.DrawString(0, y, label, vt::color::ForegroundWhite);
			renderer.DrawString(x, y, std::format("{:>5.2f}ms", time.count()), color);
		};

		auto yMax = std::max(renderer.GetHeight() - 1, 4);
		renderStat(yMax - 4, frameTimeLabel, GetEngine()->GetLastFrameTimeMs(), 16.67ms, false);
		renderStat(yMax - 3, tickTimeLabel, GetEngine()->GetLastTickTimeMs(), 5.0ms, false);
		renderStat(yMax - 2, renderTimeLabel, GetEngine()->GetLastRenderTimeMs(), 5.0ms, false);
		renderStat(yMax - 1, presentTimeLabel, GetEngine()->GetLastPresentTimeMs(), 5.0ms, false);
		renderStat(yMax, idleTimeLabel, GetEngine()->GetLastIdleTimeMs(), 1.0ms, true);
	}

	void EndPlay() override
	{
	}

private:
	void TickAutoplay(std::chrono::duration<double> deltaTime)
	{
		static std::chrono::milliseconds timeSinceLastMovement = 0ms;
		timeSinceLastMovement += std::chrono::duration_cast<std::chrono::milliseconds>(deltaTime);
		if (timeSinceLastMovement < 50ms)
		{
			return; // Wait for 100ms before moving again
		}
		timeSinceLastMovement = 0ms;

		if (m_x <= 0)
		{
			m_currentMovement = 1; // Move right
			return;
		}
		
		if (m_x >= GetEngine()->GetRendererWidth() - 1)
		{
			m_currentMovement = -1; // Move left
			return;
		}

		// Roll a random number between 0 and 1
		const auto movementRandom = static_cast<double>(rand()) / RAND_MAX;
		if (m_currentMovement == 0 && movementRandom < 0.2)
		{
			m_currentMovement = movementRandom < 0.1 ? -1 : 1; // Randomly choose direction
		}
		else if (movementRandom < 0.2)
		{
			m_currentMovement = 0; // Stop moving
		}
		else if (movementRandom < 0.21)
		{
			m_currentMovement = -m_currentMovement; // Reverse direction
		}

		const auto snowflakeRandom = static_cast<double>(rand()) / RAND_MAX;
		if (snowflakeRandom < 0.1)
		{
			// Spawn a snowflake at the current position
			m_snowflakes[m_x] = { 1, 0ms };
		}
	}

	struct Snowflake
	{
		int y;
		std::chrono::milliseconds accruedTime;
	};

	int m_x;
	int m_currentMovement = 0;
	bool m_autoplayEnabled = false;
	std::vector<Snowflake> m_snowflakes;

	// Delete copy/move construction and assignment
private:
	Snow(Snow&) = delete;
	Snow(Snow&&) = delete;
	Snow& operator=(Snow&) = delete;
	Snow& operator=(Snow&&) = delete;
};

int main()
{
	nu::engine::Engine engine;
	Snow game;
	engine.StartGame(game);
}