#pragma once

#include <format>
#include <iostream>
#include <ostream>
#include <string>
#include <string_view>

namespace nu
{
namespace console
{
	// Virtual Terminal Sequences
	// https://learn.microsoft.com/en-us/windows/console/console-virtual-terminal-sequences
	// Current implementation covers most output sequences, but does not currently include input sequences
	namespace vt
	{
		// Control Sequence Introducer prefix
		inline constexpr std::string_view CSI = "\x1b[";

		// Code: DECSTR
		// Reset certain terminal settings to their defaults.
		//   - Cursor visibility: visible (DECTEM)
		//   - Numeric Keypad: Numeric Mode (DECNKM)
		//   - Cursor Keys Mode: Normal Mode (DECCKM)
		//   - Top and Bottom Margins: Top = 1, Bottom = Console height (DECSTBM)
		//   - Character Set: US ASCII
		//   - Graphics Rendition: Default / Off(SGR)
		//   - Save cursor state : Home position (0, 0) (DECSC)
		inline constexpr std::string_view SoftReset = "\x1b[!p";

		// Switches to a new alternate screen buffer
		inline constexpr std::string_view UseAlternateScreenBuffer = "\x1b[?1049h";

		// Switches to the main screen buffer
		inline constexpr std::string_view UseMainScreenBuffer = "\x1b[?1049l";

		// Enables DEC Line Drawing Mode
		// See http://vt100.net/docs/vt220-rm/table2-4.html for a listing of all of the characters represented by the
		// DEC Special Graphics Character Set.
		//   Hex  ASCII DEC Line Drawing
		//   0x6a j     ┘
		//
		//   0x6b k     ┐
		//
		//   0x6c l     ┌
		//
		//   0x6d m     └
		//
		//   0x6e n     ┼
		//
		//   0x71 q     ─
		//
		//   0x74 t     ├
		//
		//   0x75 u     ┤
		//
		//   0x76 v     ┴
		//
		//   0x77 w     ┬
		//
		//   0x78 x     │
		inline constexpr std::string_view DrawingMode = "\x1b(0";

		// Enables ASCII Mode (Default)
		inline constexpr std::string_view ASCIIMode = "\x1b(B";

		// Sets the window's title to <string>
		inline void SetWindowTitle(std::string_view title, std::ostream& stream)
		{
			stream << "\x1b]0;" << title << "\x1b\\";
		}

		// Sets the window's title to <string>
		inline std::string SetWindowTitle(std::string_view title)
		{
			std::string result{ "\x1b]0;" };
			result += title;
			result += "\x1b\\";
			return result;
		}

		// Set Graphic Rendition (SGR) sequences
		// Generally equivalent to calling SetConsoleTextAttribute Windows API
		namespace color
		{
			// Applies provided RGB color as foreground color
			inline void ForegroundRGB(uint8_t r, uint8_t g, uint8_t b, std::ostream& stream)
			{
				stream << CSI << "38;2;" << static_cast<int>(r) << ';' << static_cast<int>(g) << ';'
					   << static_cast<int>(b) << 'm';
			}

			// Applies provided RGB color as foreground color
			inline std::string ForegroundRGB(uint8_t r, uint8_t g, uint8_t b)
			{
				std::string result{ CSI };
				result += "38;2;";
				result += std::to_string(static_cast<int>(r)) + ';' + std::to_string(static_cast<int>(g)) + ';'
				        + std::to_string(static_cast<int>(b)) + 'm';
				return result;
			}

			// Applies provided RGB color as background color
			inline void BackgroundRGB(uint8_t r, uint8_t g, uint8_t b, std::ostream& stream)
			{
				stream << CSI << "48;2;" << static_cast<int>(r) << ';' << static_cast<int>(g) << ';'
					   << static_cast<int>(b) << 'm';
			}

			// Applies provided RGB color as background color
			inline std::string BackgroundRGB(uint8_t r, uint8_t g, uint8_t b)
			{
				std::string result{ CSI };
				result += "48;2;";
				result += std::to_string(static_cast<int>(r)) + ';' + std::to_string(static_cast<int>(g)) + ';'
				        + std::to_string(static_cast<int>(b)) + 'm';
				return result;
			}

			// Returns all attributes to the default state prior to modification
			inline constexpr std::string_view Default = "\x1b[0m";

			// Applies brightness/intensity flag to foreground color
			inline constexpr std::string_view Bright = "\x1b[1m";

			// Removes brightness/intensity flag from foreground color
			inline constexpr std::string_view Dim = "\x1b[2m";

			// Adds underline
			inline constexpr std::string_view Underline = "\x1b[4m";

			// Removes underline
			inline constexpr std::string_view NoUnderline = "\x1b[24m";

			// Swaps foreground and background colors
			inline constexpr std::string_view Negative = "\x1b[7m";

			// Returns foreground and background colors to normal (no negative)
			inline constexpr std::string_view Positive = "\x1b[27m";

			// Applies non-bold/bright black to foreground
			inline constexpr std::string_view ForegroundBlack = "\x1b[30m";

			// Applies non-bold/bright red to foreground
			inline constexpr std::string_view ForegroundRed = "\x1b[31m";

			// Applies non-bold/bright green to foreground
			inline constexpr std::string_view ForegroundGreen = "\x1b[32m";

			// Applies non-bold/bright yellow to foreground
			inline constexpr std::string_view ForegroundYellow = "\x1b[33m";

			// Applies non-bold/bright blue to foreground
			inline constexpr std::string_view ForegroundBlue = "\x1b[34m";

			// Applies non-bold/bright magenta to foreground
			inline constexpr std::string_view ForegroundMagenta = "\x1b[35m";

			// Applies non-bold/bright cyan to foreground
			inline constexpr std::string_view ForegroundCyan = "\x1b[36m";

			// Applies non-bold/bright white to foreground
			inline constexpr std::string_view ForegroundWhite = "\x1b[37m";

			// Applies non-bold/bright black to background
			inline constexpr std::string_view BackgroundBlack = "\x1b[40m";

			// Applies non-bold/bright red to background
			inline constexpr std::string_view BackgroundRed = "\x1b[41m";

			// Applies non-bold/bright green to background
			inline constexpr std::string_view BackgroundGreen = "\x1b[42m";

			// Applies non-bold/bright yellow to background
			inline constexpr std::string_view BackgroundYellow = "\x1b[43m";

			// Applies non-bold/bright blue to background
			inline constexpr std::string_view BackgroundBlue = "\x1b[44m";

			// Applies non-bold/bright magenta to background
			inline constexpr std::string_view BackgroundMagenta = "\x1b[45m";

			// Applies non-bold/bright cyan to background
			inline constexpr std::string_view BackgroundCyan = "\x1b[46m";

			// Applies non-bold/bright white to background
			inline constexpr std::string_view BackgroundWhite = "\x1b[47m";

			// Applies bold/bright black to foreground
			inline constexpr std::string_view ForegroundBrightBlack = "\x1b[90m";

			// Applies bold/bright red to foreground
			inline constexpr std::string_view ForegroundBrightRed = "\x1b[91m";

			// Applies bold/bright green to foreground
			inline constexpr std::string_view ForegroundBrightGreen = "\x1b[92m";

			// Applies bold/bright yellow to foreground
			inline constexpr std::string_view ForegroundBrightYellow = "\x1b[93m";

			// Applies bold/bright blue to foreground
			inline constexpr std::string_view ForegroundBrightBlue = "\x1b[94m";

			// Applies bold/bright magenta to foreground
			inline constexpr std::string_view ForegroundBrightMagenta = "\x1b[95m";

			// Applies bold/bright cyan to foreground
			inline constexpr std::string_view ForegroundBrightCyan = "\x1b[96m";

			// Applies bold/bright white to foreground
			inline constexpr std::string_view ForegroundBrightWhite = "\x1b[97m";

			// Applies bold/bright black to background
			inline constexpr std::string_view BackgroundBrightBlack = "\x1b[100m";

			// Applies bold/bright red to background
			inline constexpr std::string_view BackgroundBrightRed = "\x1b[101m";

			// Applies bold/bright green to background
			inline constexpr std::string_view BackgroundBrightGreen = "\x1b[102m";

			// Applies bold/bright yellow to background
			inline constexpr std::string_view BackgroundBrightYellow = "\x1b[103m";

			// Applies bold/bright blue to background
			inline constexpr std::string_view BackgroundBrightBlue = "\x1b[104m";

			// Applies bold/bright magenta to background
			inline constexpr std::string_view BackgroundBrightMagenta = "\x1b[105m";

			// Applies bold/bright cyan to background
			inline constexpr std::string_view BackgroundBrightCyan = "\x1b[106m";

			// Applies bold/bright white to background
			inline constexpr std::string_view BackgroundBrightWhite = "\x1b[107m";
		} // namespace color

		// Cursor positioning sequences
		// Generally equivalent to SetConsolecursorPosition Windows API
		namespace cursor
		{
			// Code: ATT160
			// Start the cursor blinking
			inline constexpr std::string_view StartBlinking = "\x1b[?12h";

			// Code: ATT160
			// Stop the cursor blinking
			inline constexpr std::string_view StopBlinking = "\x1b[?12l";

			// Code: DECTCEM
			// Show the cursor
			inline constexpr std::string_view ShowCursor = "\x1b[?25h";

			// Code: DECTCEM
			// Hide the cursor
			inline constexpr std::string_view HideCursor = "\x1b[?25l";

			// Code: DECSCUSR
			// Default cursor shape configured by the user
			inline constexpr std::string_view UserShape = "\x1b[0 q";

			// Code: DECSCUSR
			// Blinking block cursor shape
			inline constexpr std::string_view BlinkingBlock = "\x1b[1 q";

			// Code: DECSCUSR
			// Steady block cursor shape
			inline constexpr std::string_view SteadyBlock = "\x1b[2 q";

			// Code: DECSCUSR
			// Blinking underline cursor shape
			inline constexpr std::string_view BlinkingUnderline = "\x1b[3 q";

			// Code: DECSCUSR
			// Steady underline cursor shape
			inline constexpr std::string_view SteadyUnderline = "\x1b[4 q";

			// Code: DECSCUSR
			// Blinking blinking cursor shape
			inline constexpr std::string_view BlinkingBar = "\x1b[5 q";

			// Code: DECSCUSR
			// Steady bar cursor shape
			inline constexpr std::string_view SteadyBar = "\x1b[6 q";

			// Code: RI
			// Reverse Index – Performs the reverse operation of \n, moves cursor up one line, maintains horizontal
			// position, scrolls buffer if necessary
			inline constexpr std::string_view ReverseIndex = "\x1bM";

			// Code: DECSC
			// Save Cursor Position in Memory
			inline constexpr std::string_view SaveCursorPosition =
				"\x1b"
				"7"; // concat to avoid single hex number compiler interpretation

			// Code: DECSR
			// Restore Cursor Position in Memory
			inline constexpr std::string_view RestoreCursorPosition =
				"\x1b"
				"8"; // concat to avoid single hex number compiler interpretation

			// Code: CUU
			// Cursor up
			inline constexpr std::string_view MoveUp = "\x1b[1A";

			// Code: CUU
			// Cursor up by <n>
			inline void MoveUpN(int n, std::ostream& stream)
			{
				stream << CSI << n << 'A';
			}

			// Code: CUU
			// Cursor up by <n>
			inline std::string MoveUpN(int n)
			{
				std::string result{ CSI };
				result += std::to_string(n);
				result += 'A';
				return result;
			}

			// Code: CUD
			// Cursor down
			inline constexpr std::string_view MoveDown = "\x1b[1B";

			// Code: CUD
			// Cursor down by <n>
			inline void MoveDownN(int n, std::ostream& stream)
			{
				stream << CSI << n << 'B';
			}

			// Code: CUF
			// Cursor forward
			inline constexpr std::string_view MoveForward = "\x1b[1C";

			// Code: CUF
			// Cursor forward (Right) by <n>
			inline void MoveForwardN(int n, std::ostream& stream)
			{
				stream << CSI << n << 'C';
			}

			// Code: CUF
			// Cursor forward (Right) by <n>
			inline std::string MoveForwardN(int n)
			{
				std::string result{ CSI };
				result += std::to_string(n);
				result += 'C';
				return result;
			}

			// Code: CUB
			// Cursor forward
			inline constexpr std::string_view MoveBackward = "\x1b[1D";

			// Code: CUB
			// Cursor backward (Left) by <n>
			inline void MoveBackwardN(int n, std::ostream& stream)
			{
				stream << CSI << n << 'D';
			}

			// Code: CUB
			// Cursor backward (Left) by <n>
			inline std::string MoveBackwardN(int n)
			{
				std::string result{ CSI };
				result += std::to_string(n);
				result += 'D';
				return result;
			}

			// Code: CNL
			// Cursor next line
			inline constexpr std::string_view MoveNextLine = "\x1b[1E";

			// Code: CNL
			// Cursor down <n> lines from current position
			inline void MoveNextLines(int n, std::ostream& stream)
			{
				stream << CSI << n << 'E';
			}

			// Code: CNL
			// Cursor down <n> lines from current position
			inline std::string MoveNextLines(int n)
			{
				std::string result{ CSI };
				result += std::to_string(n);
				result += 'E';
				return result;
			}

			// Code: CPL
			// Cursor previous line
			inline constexpr std::string_view MovePreviousLine = "\x1b[1F";

			// Code: CPL
			// Cursor up <n> lines from current position
			inline void MovePreviousLines(int n, std::ostream& stream)
			{
				stream << CSI << n << 'F';
			}

			// Code: CPL
			// Cursor up <n> lines from current position
			inline std::string MovePreviousLines(int n)
			{
				std::string result{ CSI };
				result += std::to_string(n);
				result += 'F';
				return result;
			}

			// Code: CHT
			// Advance the cursor to the next column (in the same row) with a tab stop. If there are no more tab stops,
			// move to the last column in the row. If the cursor is in the last column, move to the first column of the
			// next row.
			inline constexpr std::string_view TabForward = "\x1b[1I";

			// Code: CHT
			// Advance the cursor to the next column (in the same row) with a tab stop. If there are no more tab stops,
			// move to the last column in the row. If the cursor is in the last column, move to the first column of the
			// next row.
			inline void TabForwardN(int n, std::ostream& stream)
			{
				stream << CSI << n << 'I';
			}

			// Code: CHT
			// Advance the cursor to the next column (in the same row) with a tab stop.
			// If there are no more tab stops, move to the last column in the row.
			// If the cursor is in the last column, move to the first column of the next row.
			inline std::string TabForwardN(int n)
			{
				std::string result{ CSI };
				result += std::to_string(n);
				result += 'I';
				return result;
			}

			// Code: CBT
			// Move the cursor to the previous column (in the same row) with a tab stop. If there are no more tab stops,
			// moves the cursor to the first column. If the cursor is in the first column, doesn’t move the cursor.
			inline constexpr std::string_view TabBackward = "\x1b[1Z";

			// Code: CBT
			// Move the cursor to the previous column (in the same row) with a tab stop. If there are no more tab stops,
			// moves the cursor to the first column. If the cursor is in the first column, doesn’t move the cursor.
			inline void TabBackwardN(int n, std::ostream& stream)
			{
				stream << CSI << n << 'Z';
			}

			// Code: CBT
			// Move the cursor to the previous column (in the same row) with a tab stop.
			// If there are no more tab stops, move the cursor to the first column.
			// If the cursor is in the first column, doesn’t move the cursor.
			inline std::string TabBackwardN(int n)
			{
				std::string result{ CSI };
				result += std::to_string(n);
				result += 'Z';
				return result;
			}

			// Code: CHA
			// Cursor moves to <n>th position horizontally in the current line
			inline void SetHorizontalPosition(int n, std::ostream& stream)
			{
				stream << CSI << n << 'G';
			}

			// Code: CHA
			// Cursor moves to <n>th position horizontally in the current line
			inline std::string SetHorizontalPosition(int n)
			{
				std::string result{ CSI };
				result += std::to_string(n);
				result += "G";
				return result;
			}

			// Code: VPA
			// Cursor moves to <n>th position vertically in the current column
			inline void SetVerticalPosition(int n, std::ostream& stream)
			{
				stream << CSI << n << 'd';
			}

			// Code: VPA
			// Cursor moves to <n>th position vertically in the current column
			inline std::string SetVerticalPosition(int n)
			{
				std::string result{ CSI };
				result += std::to_string(n);
				result += "d";
				return result;
			}

			// Code: CUP
			// Cursor moves to <x>; <y> coordinate within the viewport, where <x> is the column of the <y> line
			// Coordinates are 1-based
			inline void SetPosition(int x, int y, std::ostream& stream)
			{
				stream << CSI << y << ';' << x << 'H';
			}

			// Code: CUP
			// Cursor moves to <x>; <y> coordinate within the viewport, where <x> is the column of the <y> line
			// Coordinates are 1-based
			inline std::string SetPosition(int x, int y)
			{
				std::string result{ CSI };
				result += std::to_string(y);
				result += ';';
				result += std::to_string(x);
				result += 'H';
				return result;
			}
		} // namespace cursor

		// Tab stop sequences
		namespace tabs
		{
			// Code: HTS
			// Sets a tab stop in the current column the cursor is in
			inline constexpr std::string_view HorizontalTabSet = "\x1bH";

			// Code: TBC
			// Clears the tab stop in the current column, if there is one. Otherwise does nothing.
			inline constexpr std::string_view TabClearCurrentColumn = "\x1b[0g";

			// Code: TBC
			// Clears all currently set tab stops
			inline constexpr std::string_view TabClearAll = "\x1b[3g";
		} // namespace tabs

		// Text modification sequences
		// Generally equivalent to FillConsoleOutputCharacter, FillConsoleOutputAttribute, and ScrollConsoleScreenBuffer
		// Windows APIs
		namespace text
		{
			// Code: ICH
			// Insert 1 space at the current cursor position, shifting all existing text to the right. Text exiting
			// the screen to the right is removed.
			inline constexpr std::string_view InsertCharacter = "\x1b[1@";

			// Code: ICH
			// Insert <n> spaces at the current cursor position, shifting all existing text to the right. Text exiting
			// the screen to the right is removed.
			inline void InsertCharacters(int n, std::ostream& stream)
			{
				stream << CSI << n << '@';
			}

			// Code: ICH
			// Insert <n> spaces at the current cursor position, shifting all existing text to the right.
			// Text exiting the screen to the right is removed.
			inline std::string InsertCharacters(int n)
			{
				std::string result{ CSI };
				result += std::to_string(n);
				result += '@';
				return result;
			}

			// Code: DCH
			// Delete 1 character at the current cursor position, shifting in space characters from the right edge of
			// the screen.
			inline constexpr std::string_view DeleteCharacter = "\x1b[1P";

			// Code: DCH
			// Delete <n> characters at the current cursor position, shifting in space characters from the right edge of
			// the screen.
			inline void DeleteCharacters(int n, std::ostream& stream)
			{
				stream << CSI << n << 'P';
			}

			// Code: DCH
			// Delete <n> characters at the current cursor position, shifting in space characters from the right edge of
			// the screen.
			inline std::string DeleteCharacters(int n)
			{
				std::string result{ CSI };
				result += std::to_string(n);
				result += 'P';
				return result;
			}

			// Code: ECH
			// Erase 1 character from the current cursor position by overwriting them with a space character.
			inline constexpr std::string_view EraseCharacter = "\x1b[1X";

			// Code: ECH
			// Erase <n> characters from the current cursor position by overwriting them with a space character.
			inline void EraseCharacters(int n, std::ostream& stream)
			{
				stream << CSI << n << 'X';
			}

			// Code: ECH
			// Erase <n> characters from the current cursor position by overwriting them with a space character.
			inline std::string EraseCharacters(int n)
			{
				std::string result{ CSI };
				result += std::to_string(n);
				result += 'X';
				return result;
			}

			// Code: IL
			// Inserts 1 line into the buffer at the cursor position. The line the cursor is on, and lines below it,
			// will be shifted downwards.
			inline constexpr std::string_view InsertLine = "\x1b[1L";

			// Code: IL
			// Inserts <n> lines into the buffer at the cursor position. The line the cursor is on, and lines below it,
			// will be shifted downwards.
			inline void InsertLines(int n, std::ostream& stream)
			{
				stream << CSI << n << 'L';
			}

			// Code: IL
			// Inserts <n> lines into the buffer at the cursor position.
			// The line the cursor is on, and lines below it, will be shifted downwards.
			inline std::string InsertLines(int n)
			{
				std::string result{ CSI };
				result += std::to_string(n);
				result += 'L';
				return result;
			}

			// Code: DL
			// Deletes 1 lines from the buffer, starting with the row the cursor is on.
			inline constexpr std::string_view DeleteLine = "\x1b[1L";

			// Code: DL
			// Deletes <n> lines from the buffer, starting with the row the cursor is on.
			inline void DeleteLines(int n, std::ostream& stream)
			{
				stream << CSI << n << 'M';
			}

			// Code: DL
			// Deletes <n> lines from the buffer, starting with the row the cursor is on.
			inline std::string DeleteLines(int n)
			{
				std::string result{ CSI };
				result += std::to_string(n);
				result += 'M';
				return result;
			}

			// Code: ED
			// Replace all text in the current viewport/screen from the start thru the cursor with space characters
			inline constexpr std::string_view EraseInDisplayStartThruCursor = "\x1b[1J";

			// Code: ED
			// Replace all text in the current viewport/screen from the cursor thru the end with space characters
			inline constexpr std::string_view EraseInDisplayCursorThruEnd = "\x1b[0J";

			// Code: ED
			// Replace all text in the current viewport/screen with space characters
			inline constexpr std::string_view EraseInDisplayAll = "\x1b[2J";

			// Code: EL
			// Replace all text on the current line start thru the cursor with space characters
			inline constexpr std::string_view EraseInLineStartThruCursor = "\x1b[1K";

			// Code: EL
			// Replace all text on the current line from the cursor thru the end with space characters
			inline constexpr std::string_view EraseInLineCursorThruEnd = "\x1b[0K";

			// Code: EL
			// Replace all text on the current line with space characters
			inline constexpr std::string_view EraseInLineAll = "\x1b[2K";
		} // namespace text

		// Viewport positioning sequences
		// Generally equivalent to calling ScrollConsoleScreenBuffer Windows API
		namespace viewport
		{
			// Code: SU
			// Scroll text up by 1. Also known as pan down, new lines fill in from the bottom of the screen
			inline constexpr std::string_view ScrollUp = "\x1b[1S";

			// Code: SU
			// Scroll text up by <n>. Also known as pan down, new lines fill in from the bottom of the screen
			inline void ScrollUpN(int n, std::ostream& stream)
			{
				stream << CSI << n << 'S';
			}

			// Code: SU
			// Scroll text up by <n>. Also known as pan down, new lines fill in from the bottom of the screen.
			inline std::string ScrollUpN(int n)
			{
				std::string result{ CSI };
				result += std::to_string(n);
				result += 'S';
				return result;
			}

			// Code: SD
			// Scroll text down by 1. Also known as pan up, new lines fill in from the bottom of the screen
			inline constexpr std::string_view ScrollDown = "\x1b[1T";

			// Code: SD
			// Scroll text down by <n>. Also known as pan up, new lines fill in from the top of the screen
			inline void ScrollDownN(int n, std::ostream& stream)
			{
				stream << CSI << n << 'T';
			}

			// Code: SD
			// Scroll text down by <n>. Also known as pan up, new lines fill in from the top of the screen.
			inline std::string ScrollDownN(int n)
			{
				std::string result{ CSI };
				result += std::to_string(n);
				result += 'T';
				return result;
			}

			// Code: DECSTBM
			// Sets the T scrolling margins of the viewport. This is a subset of the rows that are adjusted when the
			// screen would otherwise scroll, for example, on a ‘\n’ or RI. These margins also affect the rows modified
			// by Insert Line (IL) and Delete Line (DL), Scroll Up (SU) and Scroll Down (SD).
			inline void SetScrollingRegion(int t, int b, std::ostream& stream)
			{
				stream << CSI << t << ';' << b << 'r';
			}

			// Code: DECSTBM
			// Sets the T scrolling margins of the viewport.
			inline std::string SetScrollingRegion(int t, int b)
			{
				std::string result{ CSI };
				result += std::to_string(t);
				result += ';';
				result += std::to_string(b);
				result += 'r';
				return result;
			}
		} // namespace viewport
	}     // namespace vt

} // namespace console
} // namespace nu
