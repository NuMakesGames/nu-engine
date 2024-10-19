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
		constexpr DWORD eventsPerLoop = 64;
		std::array<INPUT_RECORD, eventsPerLoop> inputRecords;

		DWORD eventsToRead = 0;
		while (::GetNumberOfConsoleInputEvents(m_cachedConsoleState.hIn, &eventsToRead) && eventsToRead > 0)
		{
			eventsToRead = std::min(eventsToRead, eventsPerLoop);
			DWORD eventsRead = 0;
			::ReadConsoleInput(m_cachedConsoleState.hIn, inputRecords.data(), eventsToRead, &eventsRead);
			for (size_t i = 0; i < eventsRead; ++i)
			{
				switch (inputRecords[i].EventType)
				{
					case WINDOW_BUFFER_SIZE_EVENT:
					{
						// auto [x, y] = GetConsoleScreenSize();
						// std::cout << vt::ClearScreen;
						// std::cout << vt::MoveCursorTo(0, 0);
						// std::cout << vt::SetCursorVisible(false);
						// std::cout << vt::SetCursorSize(1);
						// std::cout << vt::SetCursorPosition(0, 0);
						// std::cout << vt::SetScreenSize(x, y);
						break;
					}
					case KEY_EVENT:
					{
						const auto& keyEvent = inputRecords[i].Event.KeyEvent;
						bool wasKeyMapped = true;
						Key key = Key::Escape;
						switch (keyEvent.wVirtualKeyCode)
						{
							case VK_ESCAPE:
								key = Key::Escape;
								break;
							default:
								wasKeyMapped = false;
								break;
						}

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
} // namespace console
} // namespace nu