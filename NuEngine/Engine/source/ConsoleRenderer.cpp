#include "NuEngine/ConsoleRenderer.h"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <sstream>

#include "NuEngine/Assertions.h"
#include "NuEngine/Console.h"

using namespace std::chrono_literals;

namespace nu
{
namespace console
{
	ConsoleRenderer::ConsoleRenderer()
	{
		m_cachedConsoleState = CacheConsoleState();

		VerifyElseCrash(EnableVirtualTerminalProcessing());
		std::cout << vt::UseAlternateScreenBuffer << vt::cursor::HideCursor;

		auto [x, y] = GetConsoleScreenSize();
		Resize(x, y, false /*shouldResizeWindow*/);
	}

	ConsoleRenderer::ConsoleRenderer(uint16_t sizeX, uint16_t sizeY)
	{
		m_cachedConsoleState = CacheConsoleState();

		VerifyElseCrash(EnableVirtualTerminalProcessing());
		std::cout << vt::UseAlternateScreenBuffer << vt::cursor::HideCursor;

		Resize(sizeX, sizeY, true /*shouldResizeWindow*/);
	}

	ConsoleRenderer::~ConsoleRenderer()
	{
		std::cout << vt::UseMainScreenBuffer;
		RestoreConsoleState(m_cachedConsoleState);
	}

	void ConsoleRenderer::Clear()
	{
		std::ranges::fill(GetBackBuffer(), Glyph{});
	}

	bool ConsoleRenderer::DrawChar(
		uint16_t x,
		uint16_t y,
		char character,
		std::string_view foregroundColor,
		std::string_view backgroundColor)
	{
		if (x >= m_sizeX || y >= m_sizeY)
		{
			return false;
		}

		Glyph& glyph = GetBackBuffer()[y * m_sizeX + x];
		glyph.character = character;
		glyph.foregroundColor = std::string(foregroundColor);
		glyph.backgroundColor = std::string(backgroundColor);
		return true;
	}

	bool ConsoleRenderer::DrawU8Char(
		uint16_t x,
		uint16_t y,
		std::u8string_view character,
		std::string_view foregroundColor,
		std::string_view backgroundColor)
	{
		if (x >= m_sizeX || y >= m_sizeY)
		{
			return false;
		}

		auto [extractedCharacter, remainingView] = ReadNextU8Char(character);
		VerifyElseCrash(remainingView.empty()); // Ensure that the input is exactly one character

		Glyph& glyph = GetBackBuffer()[y * m_sizeX + x];
		glyph.character = extractedCharacter;
		glyph.foregroundColor = std::string(foregroundColor);
		glyph.backgroundColor = std::string(backgroundColor);
		return true;
	}

	bool ConsoleRenderer::DrawString(
		uint16_t x,
		uint16_t y,
		std::string_view text,
		std::string_view foregroundColor,
		std::string_view backgroundColor)
	{
		if (x >= m_sizeX || y >= m_sizeY)
		{
			return false;
		}

		bool result = true;
		for (uint16_t i = 0; i < text.size(); ++i)
		{
			if (x + i < m_sizeX)
			{
				result = result && DrawChar(x + i, y, text[i], foregroundColor, backgroundColor);
			}
		}
		return result;
	}

	bool ConsoleRenderer::DrawU8String(
		uint16_t x,
		uint16_t y,
		std::u8string_view text,
		std::string_view foregroundColor,
		std::string_view backgroundColor)
	{
		bool result = true;
		auto [extractedCharacter, remainingView] = ReadNextU8Char(text);
		while (!extractedCharacter.empty())
		{
			result = result && DrawU8Char(x, y, extractedCharacter, foregroundColor, backgroundColor);
			std::tie(extractedCharacter, remainingView) = ReadNextU8Char(remainingView);
			++x;
		}

		return result;
	}

	void ConsoleRenderer::Present()
	{
		// Get references to the buffers
		auto& backBuffer = GetBackBuffer();
		auto& frontBuffer = GetFrontBuffer();
		VerifyElseCrash(backBuffer.size() == frontBuffer.size());

		// Update any positions on the console that have changed
		// Don't send straight to std::cout to avoid the update being visibile in an inconsistent state
		std::u8string builder;
		int cursorX = 0;
		int cursorY = 0;
		std::string backgroundColor;
		std::string foregroundColor;
		for (int i = 0; i < backBuffer.size(); ++i)
		{
			const auto& backGlyph = backBuffer[i];
			const auto& frontGlyph = frontBuffer[i];
			if (backGlyph == frontGlyph && !m_shouldDrawAllGlyphs)
			{
				continue;
			}

			const int x = i % m_sizeX + 1;
			const int y = i / m_sizeX + 1;
			if (cursorX != x || cursorY != y || i == 0)
			{
				std::string setCursorPosition = vt::cursor::SetPosition(x, y);
				builder += std::u8string{ setCursorPosition.begin(), setCursorPosition.end() };
				cursorX = x;
				cursorY = y;
			}

			if (foregroundColor != backGlyph.foregroundColor || i == 0)
			{
				builder += std::u8string{ backGlyph.foregroundColor.begin(), backGlyph.foregroundColor.end() };
				foregroundColor = backGlyph.foregroundColor;
			}

			if (backgroundColor != backGlyph.backgroundColor || i == 0)
			{
				builder += std::u8string{ backGlyph.backgroundColor.begin(), backGlyph.backgroundColor.end() };
				backgroundColor = backGlyph.backgroundColor;
			}

			builder += backGlyph.character;
			++cursorX;
		}
		m_shouldDrawAllGlyphs = false;

		// Flush to make sure the console actually updates
		if (!builder.empty())
		{
			std::cout << std::string(builder.begin(), builder.end()) << vt::cursor::HideCursor;
			std::cout.flush();
		}

		if (m_enableIncrementalDrawing)
		{
			// Copy the front buffer to the back buffer if incremental drawing is enabled
			backBuffer = frontBuffer;
		}

		// Flip the buffers for next frame
		m_currentBufferIndex = (m_currentBufferIndex + 1) % m_buffers.size();
	}

	void ConsoleRenderer::Resize(uint16_t desiredSizeX, uint16_t desiredSizeY, bool shouldResizeWindow)
	{
		if (desiredSizeX != m_sizeX || desiredSizeY != m_sizeY)
		{
			m_sizeX = desiredSizeX;
			m_sizeY = desiredSizeY;
			for (auto& buffer : m_buffers)
			{
				buffer.resize(m_sizeY * m_sizeX);
				std::ranges::fill(buffer, Glyph{});
			}

			// Force a full redraw on the next present
			m_shouldDrawAllGlyphs = true;
		}

		if (shouldResizeWindow)
		{
			SetConsoleScreenSize(m_sizeX, m_sizeY);
		}
	}

	std::vector<ConsoleRenderer::Glyph>& ConsoleRenderer::GetBackBuffer()
	{
		VerifyElseCrash(m_currentBufferIndex < m_buffers.size());
		return m_buffers[m_currentBufferIndex];
	}

	std::vector<ConsoleRenderer::Glyph>& ConsoleRenderer::GetFrontBuffer()
	{
		VerifyElseCrash(m_currentBufferIndex < m_buffers.size());
		size_t frontBufferIndex = (m_currentBufferIndex + 1) % m_buffers.size();
		return m_buffers[frontBufferIndex];
	}

	std::pair<std::u8string, std::u8string_view> ConsoleRenderer::ReadNextU8Char(std::u8string_view text)
	{
		if (text.empty())
		{
			return { std::u8string{}, text };
		}

		const char8_t* data = text.data();
		size_t length = text.size();

		size_t bytes = 0;
		if ((data[0] & 0b10000000) == 0)
		{
			bytes = 1;
		}
		else if ((data[0] & 0b11100000) == 0b11000000)
		{
			bytes = 2;
		}
		else if ((data[0] & 0b11110000) == 0b11100000)
		{
			bytes = 3;
		}
		else if ((data[0] & 0b11111000) == 0b11110000)
		{
			bytes = 4;
		}

		if (bytes == 0 || length < bytes)
		{
			// Invalid UTF-8 sequence
			return { std::u8string{}, text };
		}

		// Extract the character and the remaining view
		std::u8string character(data, bytes);
		std::u8string_view remainingView(data + bytes, length - bytes);

		return { character, remainingView };
	}
} // namespace console
} // namespace nu
