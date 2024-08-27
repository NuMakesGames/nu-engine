#pragma once

#include <array>
#include <string>
#include <vector>

#include "NuEngine/Console.h"
#include "NuEngine/VirtualTerminalSequences.h"

namespace nu
{
namespace console
{
	// Rendering interface for drawing to the console
	class ConsoleRenderer
	{
	public:
		// Constructor sets up the console for virtual terminal processing
		ConsoleRenderer();

		// Destructor restores the console's original mode
		~ConsoleRenderer();

		// Clears the current buffer
		void Clear();

		// Draws a character to the provided position
		void Draw(
			uint16_t x,
			uint16_t y,
			char character,
			std::string_view foregroundColor = vt::color::ForegroundWhite,
			std::string_view backgroundColor = vt::color::BackgroundBlack);

		// Draws a string to the provided position
		void Draw(
			uint16_t x,
			uint16_t y,
			std::string_view text,
			std::string_view foregroundColor = vt::color::ForegroundWhite,
			std::string_view backgroundColor = vt::color::BackgroundBlack);

		// Renders the current buffer to the console
		void Present();

		// Delete copy/move construction and assignment
	private:
		ConsoleRenderer(ConsoleRenderer&) = delete;
		ConsoleRenderer(ConsoleRenderer&&) = delete;
		ConsoleRenderer& operator=(ConsoleRenderer&) = delete;
		ConsoleRenderer& operator=(ConsoleRenderer&&) = delete;

	private:
		// Stores character and color used to render a position in the console buffer
		struct Glyph
		{
			char character = ' ';
			std::string foregroundColor = std::string(vt::color::ForegroundWhite);
			std::string backgroundColor = std::string(vt::color::BackgroundBlack);

			auto operator<=>(const Glyph&) const = default;
		};

	private:
		// Retrieves the back buffer for drawing
		std::vector<Glyph>& GetBackBuffer();

		// Retrieves the front buffer that was last presented
		std::vector<Glyph>& GetFrontBuffer();

	private:
		// False after first call to present
		bool isFirstPresent = true;

		// Horizontal size
		uint16_t m_sizeX;

		// Vertical size
		uint16_t m_sizeY;

		// Buffers drawn to by Draw functions. Rendered to console on Present.
		std::array<std::vector<Glyph>, 2> m_buffers;

		// Current buffer being drawn to. Flips on Present.
		size_t m_currentBufferIndex = 0;

		// Console configuration at construction. Restored at destruction.
		CachedConsoleState m_cachedConsoleState;
	};
} // namespace console
} // namespace nu