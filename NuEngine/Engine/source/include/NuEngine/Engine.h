#pragma once

#include "NuEngine/Game.h"
#include "NuEngine/ConsoleEventStream.h"

namespace nu
{
namespace engine
{
	class Engine : private nu::console::IKeyboardInputConsumer, private nu::console::IWindowResizeConsumer
	{
	public:
		Engine();

		// Starts the provided game
		void StartGame(class Game& game);

		// Shuts down the engine; call during exit
		void StopGame();

		// Callback for IWindowResizeConsumer when the window is resized
		virtual void OnWindowResize(uint16_t x, uint16_t y);

		// Callback for IKeyboardInputConsumer when a key is pressed
		virtual bool OnKeyDown(nu::console::Key key);

		// Callback for IKeyboardInputConsumer when a key is released
		virtual bool OnKeyUp(nu::console::Key key);

		// Desired size for the renderer; called when the window is resized
		void SetDesiredRendererSize(uint16_t x, uint16_t y) noexcept;

		// Returns the current renderer size
		std::pair<uint16_t, uint16_t> GetRendererSize() const noexcept
		{
			return { m_renderSizeX, m_renderSizeY };
		}

		// Returns the current renderer width
		uint16_t GetRendererWidth() const noexcept
		{
			return m_renderSizeX;
		}

		// Sets the target frames per second
		void SetTargetFramesPerSecond(uint16_t targetFramesPerSecond)
		{
			m_targetFramesPerSecond = targetFramesPerSecond;
		}

		// Returns the current renderer height
		uint16_t GetRendererHeight() const noexcept
		{
			return m_renderSizeY;
		}

		// Gets the current target frames per second
		uint16_t GetTargetFramesPerSecond() const noexcept
		{
			return m_targetFramesPerSecond;
		}

		// Returns the total frame time of the last frame
		std::chrono::duration<double> GetLastFrameTime() const noexcept
		{
			return m_lastFrameTime;
		}

		// Returns the total frame time of the last frame in milliseconds
		std::chrono::duration<double, std::milli> GetLastFrameTimeMs() const noexcept
		{
			return std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(m_lastFrameTime);
		}

		// Returns the time spent sleeping last frame
		std::chrono::duration<double> GetLastTickTime() const noexcept
		{
			return m_lastTickTime;
		}

		// Returns the total frame time of the last frame in milliseconds
		std::chrono::duration<double, std::milli> GetLastTickTimeMs() const noexcept
		{
			return std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(m_lastTickTime);
		}

		// Returns the time spent sleeping last frame
		std::chrono::duration<double> GetLastRenderTime() const noexcept
		{
			return m_lastRenderTime;
		}

		// Returns the total frame time of the last frame in milliseconds
		std::chrono::duration<double, std::milli> GetLastRenderTimeMs() const noexcept
		{
			return std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(m_lastRenderTime);
		}

		// Returns the time spent sleeping last frame
		std::chrono::duration<double> GetLastPresentTime() const noexcept
		{
			return m_lastPresentTime;
		}

		// Returns the total frame time of the last frame in milliseconds
		std::chrono::duration<double, std::milli> GetLastPresentTimeMs() const noexcept
		{
			return std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(m_lastPresentTime);
		}

		// Returns the time spent sleeping last frame
		std::chrono::duration<double> GetLastIdleTime() const noexcept
		{
			return m_lastIdleTime;
		}

		// Returns the total frame time of the last frame in milliseconds
		std::chrono::duration<double, std::milli> GetLastIdleTimeMs() const noexcept
		{
			return std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(m_lastIdleTime);
		}

	private:
		// Delete copy/move construction and assignment
		Engine(Engine&) = delete;
		Engine(Engine&&) = delete;
		Engine& operator=(Engine&) = delete;
		Engine& operator=(Engine&&) = delete;

	private:
		Game* m_game = nullptr;
		bool m_shouldStopGame = false;
		uint16_t m_renderSizeX = 0;
		uint16_t m_renderSizeY = 0;
		uint16_t m_targetFramesPerSecond = 60;

		std::chrono::duration<double> m_lastFrameTime = std::chrono::duration<double>::zero();
		std::chrono::duration<double> m_lastTickTime = std::chrono::duration<double>::zero();
		std::chrono::duration<double> m_lastRenderTime = std::chrono::duration<double>::zero();
		std::chrono::duration<double> m_lastPresentTime = std::chrono::duration<double>::zero();
		std::chrono::duration<double> m_lastIdleTime = std::chrono::duration<double>::zero();
	};
} // namespace engine
} // namespace nu