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
		short bufferSizeX = 0;
		short bufferSizeY = 0;
		short maximumWindowSizeX = 0;
		short maximumWindowSizeY = 0;
		short windowRectTop = 0;
		short windowRectLeft = 0;
		short windowRectBottom = 0;
		short windowRectRight = 0;
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

	// Attempts to resize the console buffer and window.
	// Note: This fails with Windows Terminal. See https://github.com/microsoft/terminal/issues/5094 for details.
	bool SetConsoleScreenSize(uint16_t sizeX, uint16_t sizeY);

	// Attempts to enable virtual terminal processing on attached console
	bool EnableVirtualTerminalProcessing();

	// Attempts to configure the console for input records
	// Disables standard input echo and line input processing
	// Enables input records for window resize events
	bool EnableInputRecords();
} // namespace console
} // namespace nu
