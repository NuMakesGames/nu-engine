#pragma once

#include "NuEngine/Game.h"

class Snowflakes : public nu::engine::Game
{
public:
	Snowflakes() = default;

	void OnWindowResize(uint16_t width, uint16_t height) override;
	bool OnKeyDown(nu::console::Key key) override;
	bool OnKeyUp(nu::console::Key key) override;
	bool OnLineInput(const std::u8string& line) override;
	void BeginPlay() override;
	void EndPlay() override;
	void Tick(std::chrono::duration<double> deltaTime) override;
	void Render(nu::console::ConsoleRenderer& renderer) override;

private:
	void TickAutoplay(std::chrono::duration<double> deltaTime);

	// Delete copy/move construction and assignment
private:
	Snowflakes(Snowflakes&) = delete;
	Snowflakes(Snowflakes&&) = delete;
	Snowflakes& operator=(Snowflakes&) = delete;
	Snowflakes& operator=(Snowflakes&&) = delete;

private:
	struct Snowflake
	{
		int y;
		std::chrono::milliseconds accruedTime;
	};

	int m_x = 0;
	int m_currentMovement = 0;
	bool m_autoplayEnabled = false;
	std::vector<Snowflake> m_snowflakes;
};