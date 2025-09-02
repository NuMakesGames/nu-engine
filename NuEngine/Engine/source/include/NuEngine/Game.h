#pragma once

#include <chrono>

#include "NuEngine/ConsoleEventStream.h"
#include "NuEngine/ConsoleRenderer.h"

namespace nu
{
namespace engine
{
	// Interface for games run by the engine
	class Game : nu::console::IKeyboardInputConsumer, nu::console::IWindowResizeConsumer
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

		// Optional callback when key is pressed
		virtual bool OnKeyDown(nu::console::Key key)
		{
			return false;
		}

		// Optional callback when key is released
		virtual bool OnKeyUp(nu::console::Key key)
		{
			return false;
		}

		// Optional callback when a line of text is completed in Lines input mode
		virtual bool OnLineInput(const std::u8string& line)
		{
			return false;
		}

		// Optional callback when the window is resized
		virtual void OnWindowResize(uint16_t width, uint16_t height)
		{
		}

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
