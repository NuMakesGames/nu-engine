#pragma once

#include "NuEngine/Game.h"

namespace nu
{
namespace engine
{
	class Engine
	{
	public:
		Engine();

		// Starts the provided game
		void StartGame(class Game& game);

		// Shuts down the engine; call during exit
		void StopGame();

		// Desired size for the renderer; called when the window is resized
		void SetDesiredRenderSize(uint16_t x, uint16_t y) noexcept;

	private:
		// Delete copy/move construction and assignment
		Engine(Engine&) = delete;
		Engine(Engine&&) = delete;
		Engine& operator=(Engine&) = delete;
		Engine& operator=(Engine&&) = delete;

	private:
		bool m_shouldStopGame = false;
		uint16_t m_renderSizeX = 0;
		uint16_t m_renderSizeY = 0;
	};
}
} // namespace nu