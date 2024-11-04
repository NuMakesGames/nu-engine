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
		m_x += m_currentMovement;

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
		renderer.Draw(m_x, 0, u8"▼", vt::color::ForegroundBrightWhite);

		// Draw snowflakes
		for (uint16_t x = 0; x < m_snowflakes.size(); ++x)
		{
			const auto& snowflake = m_snowflakes[x];
			if (snowflake.y == -1)
			{
				continue;
			}

			renderer.Draw(x, snowflake.y, u8"❄", vt::color::ForegroundBrightCyan);
			for (int j = 1; snowflake.y - j >= 1 && j < 5; ++j)
			{
				renderer.Draw(x, snowflake.y - j, u8"•", vt::color::ForegroundCyan);
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

			renderer.Draw(0, y, label, vt::color::ForegroundWhite);
			renderer.Draw(x, y, std::format("{:>5.2f}ms", time.count()), color);
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
	struct Snowflake
	{
		int y;
		std::chrono::milliseconds accruedTime;
	};

	uint16_t m_x;
	int m_currentMovement = 0;
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