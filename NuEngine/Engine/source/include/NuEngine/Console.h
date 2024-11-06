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
		void* hIn = nullptr;
		unsigned long dwOutMode = 0;
		unsigned long dwInMode = 0;
		short cursorPositionX = 0;
		short cursorPositionY = 0;
		unsigned short wTextAttributes = 0;
		unsigned long dwCursorSize = 0;
		bool bCursorVisible = true;
		unsigned int codePage = 0;
	};

	// Retrieves current console configuration using Windows APIs.
	CachedConsoleState CacheConsoleState();

	// Restores console configuration.
	void RestoreConsoleState(const CachedConsoleState& state, bool shouldRestorePosition = true);

	// Returns the screen size of the console
	std::pair<uint16_t, uint16_t> GetConsoleScreenSize();

	// Attempts to enable virtual terminal processing on attached console
	bool EnableVirtualTerminalProcessing();

	// Attempts to configure the console for input records
	// Disables standard input echo and line input processing
	// Enables input records for window resize events
	bool EnableInputRecords();
} // namespace console
} // namespace nu
