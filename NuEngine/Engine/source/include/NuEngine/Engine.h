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

	private:
		// Delete copy/move construction and assignment
		Engine(Engine&) = delete;
		Engine(Engine&&) = delete;
		Engine& operator=(Engine&) = delete;
		Engine& operator=(Engine&&) = delete;

	private:
		bool m_shouldStopGame = false;
	};
}
} // namespace nu