#include "NuEngine/Console.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include "Windows.h"

namespace nu
{
namespace console
{
	CachedConsoleState CacheConsoleState()
	{
		CachedConsoleState state;
		state.hOut = ::GetStdHandle(STD_OUTPUT_HANDLE);
		if (state.hOut == INVALID_HANDLE_VALUE)
		{
			return state;
		}

		state.hIn = ::GetStdHandle(STD_INPUT_HANDLE);
		if (state.hIn == INVALID_HANDLE_VALUE)
		{
			return state;
		}

		::GetConsoleMode(state.hOut, &state.dwOutMode);
		::GetConsoleMode(state.hIn, &state.dwInMode);

		CONSOLE_SCREEN_BUFFER_INFO bufferInfo{ 0 };
		if (::GetConsoleScreenBufferInfo(state.hOut, &bufferInfo))
		{
			state.cursorPositionX = bufferInfo.dwCursorPosition.X;
			state.cursorPositionY = bufferInfo.dwCursorPosition.Y;

			state.wTextAttributes = bufferInfo.wAttributes;

			state.bufferSizeX = bufferInfo.dwSize.X;
			state.bufferSizeY = bufferInfo.dwSize.Y;

			state.maximumWindowSizeX = bufferInfo.dwMaximumWindowSize.X;
			state.maximumWindowSizeY = bufferInfo.dwMaximumWindowSize.Y;

			state.windowRectTop = bufferInfo.srWindow.Top;
			state.windowRectLeft = bufferInfo.srWindow.Left;
			state.windowRectBottom = bufferInfo.srWindow.Bottom;
			state.windowRectRight = bufferInfo.srWindow.Right;
		}

		CONSOLE_CURSOR_INFO cursorInfo{ 0 };
		if (::GetConsoleCursorInfo(state.hOut, &cursorInfo))
		{
			state.bCursorVisible = cursorInfo.bVisible;
			state.dwCursorSize = cursorInfo.dwSize;
		}

		state.codePage = ::GetConsoleOutputCP();

		return state;
	}

	void RestoreConsoleState(const CachedConsoleState& state, bool shouldRestorePosition)
	{
		if (state.hOut == INVALID_HANDLE_VALUE || state.hIn == INVALID_HANDLE_VALUE)
		{
			return;
		}

		::SetConsoleMode(state.hOut, state.dwOutMode);
		::SetConsoleMode(state.hIn, state.dwInMode);
		if (shouldRestorePosition)
		{
			::SetConsoleCursorPosition(state.hOut, COORD{ state.cursorPositionX, state.cursorPositionY });
		}
		::SetConsoleTextAttribute(state.hOut, state.wTextAttributes);

		CONSOLE_CURSOR_INFO cursorInfo{ .dwSize = state.dwCursorSize, .bVisible = state.bCursorVisible };
		::SetConsoleCursorInfo(state.hOut, &cursorInfo);

		CONSOLE_SCREEN_BUFFER_INFOEX bufferInfo{ 0 };
		bufferInfo.cbSize = sizeof(bufferInfo);
		if (::GetConsoleScreenBufferInfoEx(state.hOut, &bufferInfo))
		{
			bufferInfo.dwSize.X = state.bufferSizeX;
			bufferInfo.dwSize.Y = state.bufferSizeY;
			bufferInfo.dwMaximumWindowSize.X = state.maximumWindowSizeX;
			bufferInfo.dwMaximumWindowSize.Y = state.maximumWindowSizeY;
			bufferInfo.srWindow.Top = state.windowRectTop;
			bufferInfo.srWindow.Left = state.windowRectLeft;

			// For reasons that aren't clear, restoring to the original values results in conhost being 1 line too short
			// and introduces scrolling.
			bufferInfo.srWindow.Bottom = state.windowRectBottom + 1;
			bufferInfo.srWindow.Right = state.windowRectRight + 1;

			::SetConsoleScreenBufferInfoEx(state.hOut, &bufferInfo);
		}

		::SetConsoleCP(state.codePage);
	}

	std::pair<uint16_t, uint16_t> GetConsoleScreenSize()
	{
		HANDLE hOut = ::GetStdHandle(STD_OUTPUT_HANDLE);
		if (hOut == INVALID_HANDLE_VALUE)
		{
			return std::make_pair(0u, 0u);
		}

		CONSOLE_SCREEN_BUFFER_INFO info;
		if (!::GetConsoleScreenBufferInfo(hOut, &info))
		{
			return std::make_pair(0u, 0u);
		}

		uint16_t x = info.srWindow.Right - info.srWindow.Left + 1;
		uint16_t y = info.srWindow.Bottom - info.srWindow.Top + 1;
		return std::make_pair(x, y);
	}

	bool SetConsoleScreenSize(uint16_t sizeX, uint16_t sizeY)
	{
		HANDLE hOut = ::GetStdHandle(STD_OUTPUT_HANDLE);
		if (hOut == INVALID_HANDLE_VALUE)
		{
			return false;
		}

		CONSOLE_SCREEN_BUFFER_INFOEX bufferInfo{ 0 };
		bufferInfo.cbSize = sizeof(bufferInfo);
		if (!::GetConsoleScreenBufferInfoEx(hOut, &bufferInfo))
		{
			return false;
		}

		if (bufferInfo.dwSize.X != sizeX || bufferInfo.dwSize.Y != sizeY || bufferInfo.dwMaximumWindowSize.X != sizeX
		    || bufferInfo.dwMaximumWindowSize.Y != sizeY)
		{
			bufferInfo.dwSize.X = sizeX;
			bufferInfo.dwSize.Y = sizeY;

			bufferInfo.dwMaximumWindowSize.X = sizeX;
			bufferInfo.dwMaximumWindowSize.Y = sizeY;

			bufferInfo.srWindow.Left = 0;
			bufferInfo.srWindow.Top = 0;
			bufferInfo.srWindow.Right = sizeX;
			bufferInfo.srWindow.Bottom = sizeY;

			return ::SetConsoleScreenBufferInfoEx(hOut, &bufferInfo);
		}

		return true;
	}

	bool EnableVirtualTerminalProcessing()
	{
		HANDLE hOut = ::GetStdHandle(STD_OUTPUT_HANDLE);
		if (hOut == INVALID_HANDLE_VALUE)
		{
			return false;
		}

		DWORD dwOriginalOutMode = 0;
		if (!::GetConsoleMode(hOut, &dwOriginalOutMode))
		{
			return false;
		}

		DWORD dwMode = dwOriginalOutMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING | DISABLE_NEWLINE_AUTO_RETURN;
		if (!::SetConsoleMode(hOut, dwMode) && ::GetLastError() == ERROR_INVALID_PARAMETER)
		{
			// Try to step down gracefully if DISABLE_NEWLINE_AUTO_RETURN fails
			dwMode = dwOriginalOutMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING;
			if (!::SetConsoleMode(hOut, dwMode))
			{
				return false;
			}
		}

		return ::SetConsoleOutputCP(CP_UTF8);
	}

	bool EnableInputRecords()
	{
		HANDLE hIn = ::GetStdHandle(STD_INPUT_HANDLE);
		if (hIn == INVALID_HANDLE_VALUE)
		{
			return false;
		}

		DWORD dwOriginalInMode = 0;
		if (!::GetConsoleMode(hIn, &dwOriginalInMode))
		{
			return false;
		}

		DWORD dwInMode = dwOriginalInMode & ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT) | ENABLE_WINDOW_INPUT;
		return ::SetConsoleMode(hIn, dwInMode);
	}
} // namespace console
} // namespace nu