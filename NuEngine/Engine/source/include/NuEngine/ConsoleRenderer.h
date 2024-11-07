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

		// Constructor sets up the console for virtual terminal processing and attempts to resize the console buffer/window
		ConsoleRenderer(uint16_t sizeX, uint16_t sizeY);

		// Destructor restores original console state
		~ConsoleRenderer();

		// Clears the current buffer
		void Clear();

		// Draws a character to the provided position
		bool DrawChar(
			uint16_t x,
			uint16_t y,
			char character,
			std::string_view foregroundColor = vt::color::ForegroundWhite,
			std::string_view backgroundColor = vt::color::BackgroundBlack);

		// Draws a character to the provided position
		template<typename T>
		bool DrawChar(
			T&& position,
			char character,
			std::string_view foregroundColor = vt::color::ForegroundWhite,
			std::string_view backgroundColor = vt::color::BackgroundBlack)
		{
			return DrawChar(position.x, position.y, character, foregroundColor, backgroundColor);
		}

		// Draws a character to the provided positions
		template<std::ranges::input_range R>
		bool DrawChar(
			R&& range,
			char character,
			std::string_view foregroundColor = vt::color::ForegroundWhite,
			std::string_view backgroundColor = vt::color::BackgroundBlack)
		{
			bool result = true;
			for (const auto& position : range)
			{
				result = result && DrawChar(position, character, foregroundColor, backgroundColor);
			}
			return result;
		}

		// Draws a UTF-8 character to the provided position
		// NOTE: Assumes that the u8string represents exactly one character
		bool DrawU8Char(
			uint16_t x,
			uint16_t y,
			std::u8string_view character,
			std::string_view foregroundColor = vt::color::ForegroundWhite,
			std::string_view backgroundColor = vt::color::BackgroundBlack);

		// Draws a UTF-8 character to the provided position
		// NOTE: Assumes that the u8string represents exactly one character
		template<typename T>
		bool DrawU8Char(
			T&& position,
			std::u8string_view character,
			std::string_view foregroundColor = vt::color::ForegroundWhite,
			std::string_view backgroundColor = vt::color::BackgroundBlack)
		{
			return DrawU8Char(position.x, position.y, character, foregroundColor, backgroundColor);
		}

		// Draws a UTF-8 character to the provided position
		// NOTE: Assumes that the u8string represents exactly one character
		template<std::ranges::input_range R>
		bool DrawU8Char(
			R&& range,
			std::u8string_view character,
			std::string_view foregroundColor = vt::color::ForegroundWhite,
			std::string_view backgroundColor = vt::color::BackgroundBlack)
		{
			bool result = true;
			for (const auto& position : range)
			{
				result = result && DrawU8Char(position, character, foregroundColor, backgroundColor);
			}
			return result;
		}

		// Draws a string to the provided position
		bool DrawString(
			uint16_t x,
			uint16_t y,
			std::string_view text,
			std::string_view foregroundColor = vt::color::ForegroundWhite,
			std::string_view backgroundColor = vt::color::BackgroundBlack);

		// Draws a string to the provided position
		template<typename T>
		bool DrawString(
			T&& position,
			std::string_view text,
			std::string_view foregroundColor = vt::color::ForegroundWhite,
			std::string_view backgroundColor = vt::color::BackgroundBlack)
		{
			return DrawString(position.x, position.y, text, foregroundColor, backgroundColor);
		}

		// Draws a string to the provided positions
		template<std::ranges::input_range R>
		bool DrawString(
			R&& range,
			std::string_view text,
			std::string_view foregroundColor = vt::color::ForegroundWhite,
			std::string_view backgroundColor = vt::color::BackgroundBlack)
		{
			bool result = true;
			for (const auto& position : range)
			{
				result = result && DrawString(position, text, foregroundColor, backgroundColor);
			}
			return result;
		}

		// Renders the current buffer to the console
		void Present();

		// Resizes the renderer to the desired width and height and optionally attempts to resize window
		void Resize(uint16_t sizeX, uint16_t sizeY, bool shouldResizeWindow = false);

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

		// Whether incremental drawing is enabled. When enabled, Present will copy the front buffer to the back buffer.
		bool IsIncrementalDrawingEnabled() const noexcept
		{
			return m_enableIncrementalDrawing;
		}

		// Enables or disables incremental drawing. Enable to preserve draw calls across Present calls at a small
		// performance cost. Only enable if you do not intend to redraw every frame (i.e. you aren't clearing and
		// drawing in full every frame).
		void SetIncrementalDrawingEnabled(bool enableIncrementalDrawing) noexcept
		{
			m_enableIncrementalDrawing = enableIncrementalDrawing;
		}

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

		// True if the front buffer should be copied to the back buffer after Present
		bool m_enableIncrementalDrawing = false;

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