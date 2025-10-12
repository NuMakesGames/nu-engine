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
		virtual void BeginPlay()
		{
		}

		// Called when the game is ending
		virtual void EndPlay()
		{
		}

		// Called each frame to update the game simulation
		virtual void Tick(std::chrono::duration<double> deltaTime) = 0;

		// Called each frame to render the game
		virtual void Render(nu::console::ConsoleRenderer& renderer) = 0;

		// Called when a key is pressed in Keys input mode
		bool OnKeyDown(nu::console::Key key) override
		{
			return false;
		}

		// Called when a key is released  in Keys input mode
		bool OnKeyUp(nu::console::Key key) override
		{
			return false;
		}

		// Called when a line of text is completed in Lines input mode
		bool OnLineInput(const std::u8string& line) override
		{
			return false;
		}

		// Called when the window is resized
		void OnWindowResize(uint16_t width, uint16_t height) override
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

		// Allow the engine to set/clear itself on the game
		friend class Engine;
	};
} // namespace engine
} // namespace nu
