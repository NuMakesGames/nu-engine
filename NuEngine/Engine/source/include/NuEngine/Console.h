#pragma once

#include <utility>

namespace nu
{
namespace console
{
	// Original console configuration to be restored
	// Does not use actual Windows types to avoid needing windows.h in consumers.
	struct CachedConsoleState
	{
		void* hOut = nullptr;
		unsigned long dwMode = 0;
		short cursorPositionX = 0;
		short cursorPositionY = 0;
		unsigned short wTextAttributes = 0;
		unsigned long dwCursorSize = 0;
		bool bCursorVisible = true;
	};

	// Retrieves current console configuration using Windows APIs.
	CachedConsoleState CacheConsoleState();

	// Restores console configuration.
	void RestoreConsoleState(const CachedConsoleState& state);

	// Returns the screen size of the console
	std::pair<uint16_t, uint16_t> GetConsoleScreenSize();

	// Attempts to enable virtual terminal processing on attached console
	bool EnableVirtualTerminalProcessing();
} // namespace console
} // namespace nu
