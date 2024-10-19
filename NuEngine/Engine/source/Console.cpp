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

	void RestoreConsoleState(const CachedConsoleState& state)
	{
		if (state.hOut == INVALID_HANDLE_VALUE || state.hIn == INVALID_HANDLE_VALUE)
		{
			return;
		}

		::SetConsoleMode(state.hOut, state.dwOutMode);
		::SetConsoleMode(state.hIn, state.dwInMode);
		::SetConsoleCursorPosition(state.hOut, COORD{ state.cursorPositionX, state.cursorPositionY });
		::SetConsoleTextAttribute(state.hOut, state.wTextAttributes);

		CONSOLE_CURSOR_INFO cursorInfo{ .dwSize = state.dwCursorSize, .bVisible = state.bCursorVisible };
		::SetConsoleCursorInfo(state.hOut, &cursorInfo);

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