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
		bool Draw(
			uint16_t x,
			uint16_t y,
			char character,
			std::string_view foregroundColor = vt::color::ForegroundWhite,
			std::string_view backgroundColor = vt::color::BackgroundBlack);

		// Draws a UTF-8 character to the provided position
		// NOTE: Assumes that the u8string represents exactly one character
		bool Draw(
			uint16_t x,
			uint16_t y,
			std::u8string_view character,
			std::string_view foregroundColor = vt::color::ForegroundWhite,
			std::string_view backgroundColor = vt::color::BackgroundBlack);

		// Draws a string to the provided position
		bool Draw(
			uint16_t x,
			uint16_t y,
			std::string_view text,
			std::string_view foregroundColor = vt::color::ForegroundWhite,
			std::string_view backgroundColor = vt::color::BackgroundBlack);

		// Renders the current buffer to the console
		void Present();

		// Resizes the renderer to the desired width and height
		void Resize(uint16_t sizeX, uint16_t sizeY);

		// Returns the current width of the renderer
		uint16_t GetWidth() const noexcept
		{
			return m_sizeX;
		}

		// Returns the current height of the renderer
		uint16_t GetHeight() const noexcept
		{
			return m_sizeY;
		};

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
			std::u8string character = std::u8string{ ' ' };
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
		// True if the buffers were resized since last present
		bool m_shouldDrawAllGlyphs = true;

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