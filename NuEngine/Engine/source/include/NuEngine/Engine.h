#pragma once

#include "NuEngine/Game.h"
#include "NuEngine/ConsoleEventStream.h"

namespace nu
{
namespace engine
{
	struct FrameTimings
	{
		std::chrono::duration<double> totalFrameTime = std::chrono::duration<double>::zero();
		std::chrono::duration<double> tickTime = std::chrono::duration<double>::zero();
		std::chrono::duration<double> renderTime = std::chrono::duration<double>::zero();
		std::chrono::duration<double> presentTime = std::chrono::duration<double>::zero();
		std::chrono::duration<double> idleTime = std::chrono::duration<double>::zero();
	};

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

		// Callback for IKeyboardInputConsumer when a line of input is available
		virtual bool OnLineInput(const std::u8string& line);

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

		// Returns the frame timings of the last frame
		const FrameTimings& GetLastFrameTimings() const noexcept
		{
			return m_lastFrameTimings;
		}

		// Returns the total frame time of the last frame
		std::chrono::duration<double> GetLastFrameTime() const noexcept
		{
			return m_lastFrameTimings.totalFrameTime;
		}

		// Returns the total frame time of the last frame in milliseconds
		std::chrono::duration<double, std::milli> GetLastFrameTimeMs() const noexcept
		{
			return std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(m_lastFrameTimings.totalFrameTime);
		}

		// Returns the time spent sleeping last frame
		std::chrono::duration<double> GetLastTickTime() const noexcept
		{
			return m_lastFrameTimings.tickTime;
		}

		// Returns the total frame time of the last frame in milliseconds
		std::chrono::duration<double, std::milli> GetLastTickTimeMs() const noexcept
		{
			return std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(m_lastFrameTimings.tickTime);
		}

		// Returns the time spent sleeping last frame
		std::chrono::duration<double> GetLastRenderTime() const noexcept
		{
			return m_lastFrameTimings.renderTime;
		}

		// Returns the total frame time of the last frame in milliseconds
		std::chrono::duration<double, std::milli> GetLastRenderTimeMs() const noexcept
		{
			return std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(m_lastFrameTimings.renderTime);
		}

		// Returns the time spent sleeping last frame
		std::chrono::duration<double> GetLastPresentTime() const noexcept
		{
			return m_lastFrameTimings.presentTime;
		}

		// Returns the total frame time of the last frame in milliseconds
		std::chrono::duration<double, std::milli> GetLastPresentTimeMs() const noexcept
		{
			return std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(m_lastFrameTimings.presentTime);
		}

		// Returns the time spent sleeping last frame
		std::chrono::duration<double> GetLastIdleTime() const noexcept
		{
			return m_lastFrameTimings.idleTime;
		}

		// Returns the total frame time of the last frame in milliseconds
		std::chrono::duration<double, std::milli> GetLastIdleTimeMs() const noexcept
		{
			return std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(m_lastFrameTimings.idleTime);
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
		bool m_isCommanderEnabled = false;
		bool m_showFps = false;
		uint16_t m_renderSizeX = 0;
		uint16_t m_renderSizeY = 0;
		uint16_t m_targetFramesPerSecond = 60;
		FrameTimings m_lastFrameTimings;
	};
} // namespace engine
} // namespace nu