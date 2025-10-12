#pragma once

#include "NuEngine/Game.h"

class Snowflakes : public nu::engine::Game
{
public:
	Snowflakes() = default;

	// Called when the window is resized
	void OnWindowResize(uint16_t width, uint16_t height) override;

	// Called when a key is pressed in Keys input mode
	bool OnKeyDown(nu::console::Key key) override;

	// Called when a key is released  in Keys input mode
	bool OnKeyUp(nu::console::Key key) override;

	// Called when a line of text is completed in Lines input mode
	bool OnLineInput(const std::u8string& line) override;

	// Called when the game is starting
	void BeginPlay() override;

	// Called each frame to update the game simulation
	void Tick(std::chrono::duration<double> deltaTime) override;

	// Called each frame to render the game
	void Render(nu::console::ConsoleRenderer& renderer) override;

private:
	// Drives the simulation when autoplay is enabled
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
		int y = -1;
		std::chrono::milliseconds accruedTime = std::chrono::milliseconds::zero();
	};

	int m_position = 0;
	int m_velocity = 0;
	bool m_autoplayEnabled = false;
	std::vector<std::vector<Snowflake>> m_columns;
};