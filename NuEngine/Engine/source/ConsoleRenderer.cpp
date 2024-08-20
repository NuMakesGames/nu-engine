#include "NuEngine/ConsoleRenderer.h"

#include <algorithm>
#include <iostream>
#include <sstream>

#include "NuEngine/Assertions.h"
#include "NuEngine/Console.h"

namespace nu
{
namespace console
{
	ConsoleRenderer::ConsoleRenderer()
	{
		m_cachedConsoleState = CacheConsoleState();

		VerifyElseCrash(EnableVirtualTerminalProcessing());
		std::cout << vt::UseAlternateScreenBuffer;

		auto [x, y] = GetConsoleScreenSize();
		m_sizeX = x;
		m_sizeY = y;

		for (auto& buffer : m_buffers)
		{
			buffer.resize(y * x);
		}
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

	void ConsoleRenderer::Draw(
		uint8_t x,
		uint8_t y,
		char character,
		std::string_view foregroundColor,
		std::string_view backgroundColor)
	{
		VerifyElseCrash(x < m_sizeX && y < m_sizeY);
		auto& glyph = GetBackBuffer()[y * m_sizeX + x];
		glyph.character = character;
		glyph.foregroundColor = std::string(foregroundColor);
		glyph.backgroundColor = std::string(backgroundColor);
	}

	void ConsoleRenderer::Present()
	{
		// Get references to the buffers
		const auto& backBuffer = GetBackBuffer();
		const auto& frontBuffer = GetFrontBuffer();
		VerifyElseCrash(backBuffer.size() == frontBuffer.size());

		// Update any positions on the console that have changed
		// Don't send straight to std::cout to avoid the update being visibile in an inconsistent state
		std::string builder;
		for (int i = 0; i < backBuffer.size(); ++i)
		{
			const auto& backGlyph = backBuffer[i];
			const auto& frontGlyph = frontBuffer[i];
			if (backGlyph != frontGlyph || isFirstPresent)
			{
				const int x = i % m_sizeX + 1;
				const int y = i / m_sizeX + 1;

				builder += vt::cursor::SetPosition(x, y);
				builder += backGlyph.foregroundColor;
				builder += backGlyph.backgroundColor;
				builder += backGlyph.character;
			}
		}
		isFirstPresent = false;

		// Flush to make sure the console actually updates
		if (!builder.empty())
		{
			std::cout << builder;
			std::cout.flush();
		}

		// Flip the buffers for next frame
		m_currentBufferIndex = (m_currentBufferIndex + 1) % m_buffers.size();
	}

	std::vector<ConsoleRenderer::Glyph>& ConsoleRenderer::GetBackBuffer()
	{
		VerifyElseCrash(m_currentBufferIndex < m_buffers.size());
		return m_buffers[m_currentBufferIndex];
	}

	std::vector<ConsoleRenderer::Glyph>& ConsoleRenderer::GetFrontBuffer()
	{
		VerifyElseCrash(m_currentBufferIndex < m_buffers.size());
		auto frontBufferIndex = (m_currentBufferIndex + 1) % m_buffers.size();
		return m_buffers[frontBufferIndex];
	}
} // namespace console
} // namespace nu
