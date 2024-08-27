#pragma once

#include <chrono>

#include "NuEngine/ConsoleRenderer.h"

namespace nu
{
namespace engine
{
	// Interface for games run by the engine
	class Game
	{
	public:
		Game();

		// Called when the game is starting
		virtual void BeginPlay() = 0;

		// Called when the game is ending
		virtual void EndPlay() = 0;

		// Called each frame to update the game simulation
		virtual void Tick(std::chrono::duration<double> deltaTime) = 0;

		// Called each frame to render the game
		virtual void Render(nu::console::ConsoleRenderer& renderer) = 0;

	protected:
		// Non-null when the game is being run by the engine
		class Engine* GetEngine();

	private:
		// Delete copy/move construction and assignment
		Game(Game&) = delete;
		Game(Game&&) = delete;
		Game& operator=(Game&) = delete;
		Game& operator=(Game&&) = delete;

	private:
		// Set/cleared by the engine when play starts/stops
		void SetEngine(class Engine* engine);

		// The engine running the game
		class Engine* m_engine;

		friend class Engine;
	};
} // namespace engine
} // namespace nu
