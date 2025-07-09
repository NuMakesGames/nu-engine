#include "NuEngine/ConsoleEventStream.h"

#include <algorithm>
#include <array>

#include "NuEngine/Assertions.h"
#include "NuEngine/Console.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include "Windows.h"
#include "conio.h"

namespace nu
{
namespace console
{
	ConsoleEventStream::ConsoleEventStream()
	{
		m_cachedConsoleState = CacheConsoleState();
		VerifyElseCrash(EnableInputRecords());
	}

	ConsoleEventStream::~ConsoleEventStream()
	{
		RestoreConsoleState(m_cachedConsoleState);
	}

	void ConsoleEventStream::ProcessEvents()
	{
		if (m_keyInputMode == KeyInputMode::Lines)
		{
			// In Lines input mode, process new keyboard input without blocking.
			// This limits the richness of standard text input, but std::getline and comparables
			// block until input is available, which would halt engine progress.
			while (_kbhit())
			{
				wint_t ch = _getwch();

				// Check for surrogate pair in UTF-16 input
				if (ch >= 0xD800 && ch <= 0xDFFF)
				{
					m_currentLine += ch;
					m_currentLine += _getwch(); // Read the next surrogate pair character
					m_isCurrentLineUtf8Valid = false;
					continue;
				}

				if (ch == VK_RETURN)
				{
					for (auto* consumer : m_keyConsumers)
					{
						if (consumer->OnLineInput(GetCurrentLine()))
						{
							break;
						}
					}

					m_currentLine.clear();
					m_isCurrentLineUtf8Valid = false;
					continue;
				}

				m_currentLine += ch;
				m_isCurrentLineUtf8Valid = false;
			}
		}

		// Process input events
		constexpr DWORD eventsPerLoop = 512;
		std::array<INPUT_RECORD, eventsPerLoop> inputRecords;
		DWORD eventsToRead = 0;
		while (::GetNumberOfConsoleInputEvents(m_cachedConsoleState.hIn, &eventsToRead) && eventsToRead > 0)
		{
			eventsToRead = std::min(eventsToRead, eventsPerLoop);
			DWORD eventsRead = 0;
			if (!::ReadConsoleInput(m_cachedConsoleState.hIn, inputRecords.data(), eventsToRead, &eventsRead))
			{
				break;
			}

			for (size_t i = 0; i < eventsRead; ++i)
			{
				switch (inputRecords[i].EventType)
				{
					case WINDOW_BUFFER_SIZE_EVENT:
					{
						const auto& resizeEvent = inputRecords[i].Event.WindowBufferSizeEvent;
						uint16_t newWidth = resizeEvent.dwSize.X;
						uint16_t newHeight = resizeEvent.dwSize.Y;

						for (auto* consumer : m_resizeConsumers)
						{
							consumer->OnWindowResize(newWidth, newHeight);
						}
						break;
					}
					case KEY_EVENT:
					{
						// Discard key events if not in Keys input mode
						if (m_keyInputMode != KeyInputMode::Keys)
						{
							break;
						}

						const auto& keyEvent = inputRecords[i].Event.KeyEvent;
						auto [wasKeyMapped, key] = TryMapKey(keyEvent.wVirtualKeyCode);
						if (!wasKeyMapped)
						{
							break;
						}

						if (keyEvent.bKeyDown)
						{
							for (auto* consumer : m_keyConsumers)
							{
								if (consumer->OnKeyDown(key))
								{
									break;
								}
							}
						}
						else
						{
							for (auto* consumer : m_keyConsumers)
							{
								if (consumer->OnKeyUp(key))
								{
									break;
								}
							}
						}
						break;
					}
					default:
						break;
				}
			}
		}
	}

	void ConsoleEventStream::RegisterKeyboardInputConsumer(IKeyboardInputConsumer* consumer)
	{
		m_keyConsumers.emplace_back(consumer);
	}

	void ConsoleEventStream::UnregisterKeyboardInputConsumer(IKeyboardInputConsumer* consumer)
	{
		std::erase(m_keyConsumers, consumer);
	}

	void ConsoleEventStream::RegisterWindowResizeConsumer(IWindowResizeConsumer* consumer)
	{
		m_resizeConsumers.emplace_back(consumer);
	}

	void ConsoleEventStream::UnregisterWindowResizeConsumer(IWindowResizeConsumer* consumer)
	{
		std::erase(m_resizeConsumers, consumer);
	}

	void ConsoleEventStream::SetKeyInputMode(KeyInputMode mode)
	{
		if (m_keyInputMode == mode)
		{
			return;
		}

		if (m_keyInputMode == KeyInputMode::Lines)
		{
			m_currentLine.clear();
			m_currentLineUtf8.clear();
		}

		m_keyInputMode = mode;
	}

	const std::u8string& ConsoleEventStream::GetCurrentLine()
	{
		if (m_isCurrentLineUtf8Valid)
		{
			return m_currentLineUtf8;
		}

		m_currentLineUtf8.clear();
		if (m_currentLine.empty())
		{
			m_isCurrentLineUtf8Valid = true;
			return m_currentLineUtf8;
		}

		// Convert from UTF-16 wide string to UTF-8
		int requiredSize = ::WideCharToMultiByte(
			CP_UTF8, 0, m_currentLine.c_str(), static_cast<int>(m_currentLine.size()), nullptr, 0, nullptr, nullptr);
		VerifyElseCrash(requiredSize > 0);

		m_currentLineUtf8.resize(requiredSize);
		::WideCharToMultiByte(
			CP_UTF8,
			0,
			m_currentLine.c_str(),
			static_cast<int>(m_currentLine.size()),
			reinterpret_cast<char*>(m_currentLineUtf8.data()),
			requiredSize,
			nullptr,
			nullptr);

		m_isCurrentLineUtf8Valid = true;
		return m_currentLineUtf8;
	}

	/*static*/ std::pair<bool, Key> ConsoleEventStream::TryMapKey(uint16_t virtualKeyCode)
	{
		switch (virtualKeyCode)
		{
			case VK_BACK:
				return { true, Key::Backspace };
			case VK_TAB:
				return { true, Key::Tab };
			case VK_RETURN:
				return { true, Key::Enter };
			case VK_SHIFT:
				return { true, Key::Shift };
			case VK_CONTROL:
				return { true, Key::Control };
			case VK_MENU:
				return { true, Key::Alt };
			case VK_ESCAPE:
				return { true, Key::Escape };
			case VK_SPACE:
				return { true, Key::Space };
			case VK_LEFT:
				return { true, Key::Left };
			case VK_UP:
				return { true, Key::Up };
			case VK_RIGHT:
				return { true, Key::Right };
			case VK_DOWN:
				return { true, Key::Down };
			case '0':
				return { true, Key::Digit0 };
			case '1':
				return { true, Key::Digit1 };
			case '2':
				return { true, Key::Digit2 };
			case '3':
				return { true, Key::Digit3 };
			case '4':
				return { true, Key::Digit4 };
			case '5':
				return { true, Key::Digit5 };
			case '6':
				return { true, Key::Digit6 };
			case '7':
				return { true, Key::Digit7 };
			case '8':
				return { true, Key::Digit8 };
			case '9':
				return { true, Key::Digit9 };
			case 'A':
				return { true, Key::A };
			case 'B':
				return { true, Key::B };
			case 'C':
				return { true, Key::C };
			case 'D':
				return { true, Key::D };
			case 'E':
				return { true, Key::E };
			case 'F':
				return { true, Key::F };
			case 'G':
				return { true, Key::G };
			case 'H':
				return { true, Key::H };
			case 'I':
				return { true, Key::I };
			case 'J':
				return { true, Key::J };
			case 'K':
				return { true, Key::K };
			case 'L':
				return { true, Key::L };
			case 'M':
				return { true, Key::M };
			case 'N':
				return { true, Key::N };
			case 'O':
				return { true, Key::O };
			case 'P':
				return { true, Key::P };
			case 'Q':
				return { true, Key::Q };
			case 'R':
				return { true, Key::R };
			case 'S':
				return { true, Key::S };
			case 'T':
				return { true, Key::T };
			case 'U':
				return { true, Key::U };
			case 'V':
				return { true, Key::V };
			case 'W':
				return { true, Key::W };
			case 'X':
				return { true, Key::X };
			case 'Y':
				return { true, Key::Y };
			case 'Z':
				return { true, Key::Z };
			case VK_OEM_3:
				return { true, Key::GraveAccent };
			default:
				return { false, Key::Escape };
		}
	}
} // namespace console
} // namespace nu