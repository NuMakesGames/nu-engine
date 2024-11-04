#include "NuEngine/ConsoleEventStream.h"

#include <algorithm>
#include <array>

#include "NuEngine/Assertions.h"
#include "NuEngine/Console.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include "Windows.h"

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
			default:
				return { false, Key::Escape };
		}
	}
} // namespace console
} // namespace nu