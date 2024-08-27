#include <chrono>
#include <deque>
#include <format>

#include "NuEngine/ConsoleRenderer.h"
#include "NuEngine/Engine.h"
#include "NuEngine/VirtualTerminalSequences.h"

using namespace nu::console;
using namespace std::chrono_literals;

class Snake : public nu::engine::Game
{
public:
	Snake()
	{
	}

	void BeginPlay() override
	{
	}

	void Tick(std::chrono::duration<double> deltaTime) override
	{
		m_totalTime += deltaTime;
		if (m_frameTimes.size() >= 1)
		{
			m_frameTimes.pop_front();
		}
		m_frameTimes.push_back(deltaTime);
		if (m_totalTime > 30s)
		{
			GetEngine()->StopGame();
		}
	}

	void Render(nu::console::ConsoleRenderer& renderer) override
	{
		int y = 0;
		for (std::chrono::duration<double> frameTime : m_frameTimes)
		{
			renderer.Draw(
				0,
				y++,
				std::format(
					"Frame time: {:.3f}ms",
					std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(frameTime).count()),
				// frameTime <= 16.667ms ? vt::color::ForegroundBrightGreen : vt::color::ForegroundBrightRed);
				vt::color::ForegroundRGB(128, 50, 32));
		}
	}

	void EndPlay() override
	{
	}

private:
	// Delete copy/move construction and assignment
	Snake(Snake&) = delete;
	Snake(Snake&&) = delete;
	Snake& operator=(Snake&) = delete;
	Snake& operator=(Snake&&) = delete;

	std::chrono::duration<double> m_totalTime;
	std::deque<std::chrono::duration<double>> m_frameTimes;
};

int main()
{
	nu::engine::Engine engine;
	Snake game;
	engine.StartGame(game);
}