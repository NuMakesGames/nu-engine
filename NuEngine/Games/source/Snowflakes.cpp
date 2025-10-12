#include "Snowflakes.h"

#include <algorithm>
#include <chrono>
#include <format>
#include <string>

#include "NuEngine/ConsoleRenderer.h"
#include "NuEngine/Engine.h"
#include "NuEngine/VirtualTerminalSequences.h"

using namespace nu::console;
using namespace std::chrono_literals;
using namespace std::literals;

void Snowflakes::BeginPlay()
{
	GetEngine()->SetTargetFramesPerSecond(240);
	auto [width, height] = GetEngine()->GetRendererSize();
	m_position = width / 2;
	m_columns.resize(width);
}

void Snowflakes::Tick(std::chrono::duration<double> deltaTime)
{
	if (m_autoplayEnabled)
	{
		TickAutoplay(deltaTime);
	}

	m_position += m_velocity;
	m_position = std::clamp(m_position, 0, static_cast<int>(m_columns.size() - 1));

	// Move all snowflakes forward
	for (auto& snowflakes : m_columns)
	{
		for (auto& snowflake : snowflakes)
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
}

void Snowflakes::Render(nu::console::ConsoleRenderer& renderer)
{
	// Draw spawner
	renderer.DrawU8Char(m_position, 0, u8"▼", vt::color::ForegroundBrightWhite);

	// Draw snowflakes
	for (uint16_t x = 0; x < m_columns.size(); ++x)
	{
		for (const auto& snowflake : m_columns[x])
		{
			{
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
		}
	}
}

void Snowflakes::OnWindowResize(uint16_t width, uint16_t height)
{
	m_position = std::min(m_position, width - 1);
	m_columns.resize(width);
}

bool Snowflakes::OnKeyDown(Key key)
{
	switch (key)
	{
		case Key::Space:
			// Spawn snowflake
			m_columns[m_position].emplace_back(1, 0ms);
			return true;
		case Key::A:
		case Key::Left:
			// Move spawner left
			if (m_velocity > -1)
			{
				--m_velocity;
			}
			return true;
		case Key::D:
		case Key::Right:
			// Move spawner right
			if (m_velocity < 1)
			{
				++m_velocity;
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

bool Snowflakes::OnKeyUp(Key key)
{
	switch (key)
	{
		case Key::A:
		case Key::Left:
			// Stop moving the spawner left
			if (m_velocity < 1)
			{
				++m_velocity;
			}
			return true;
		case Key::D:
		case Key::Right:
			// Stop moving the spawner right
			if (m_velocity > -1)
			{
				--m_velocity;
			}
			return true;
		default:
			return false;
	}
}

bool Snowflakes::OnLineInput(const std::u8string& line)
{
	if (line == u8"autoplay")
	{
		m_autoplayEnabled = !m_autoplayEnabled;
		return true;
	}

	return false;
}

void Snowflakes::TickAutoplay(std::chrono::duration<double> deltaTime)
{
	static std::chrono::milliseconds timeSinceLastMovement = 0ms;
	timeSinceLastMovement += std::chrono::duration_cast<std::chrono::milliseconds>(deltaTime);
	if (timeSinceLastMovement < 50ms)
	{
		return; // Wait for 100ms before moving again
	}
	timeSinceLastMovement = 0ms;

	if (m_position <= 0)
	{
		m_velocity = 1; // Move right
		return;
	}

	if (m_position >= GetEngine()->GetRendererWidth() - 1)
	{
		m_velocity = -1; // Move left
		return;
	}

	// Roll a random number between 0 and 1
	const auto movementRandom = static_cast<double>(rand()) / RAND_MAX;
	if (m_velocity == 0 && movementRandom < 0.2)
	{
		m_velocity = movementRandom < 0.1 ? -1 : 1; // Randomly choose direction
	}
	else if (movementRandom < 0.2)
	{
		m_velocity = 0; // Stop moving
	}
	else if (movementRandom < 0.21)
	{
		m_velocity = -m_velocity; // Reverse direction
	}

	const auto snowflakeRandom = static_cast<double>(rand()) / RAND_MAX;
	if (snowflakeRandom < 0.1)
	{
		// Spawn a snowflake at the current position
		m_columns[m_position].emplace_back(1, 0ms);
	}
}
