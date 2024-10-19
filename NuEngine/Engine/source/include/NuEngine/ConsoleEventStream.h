#pragma once

#include <vector>

#include "NuEngine/Console.h"

namespace nu
{
namespace console
{
	enum class Key : uint8_t
	{
		Shift = 0x10,
		Control = 0x11,
		Alt = 0x12,
		Space = 0x20,
		Left = 0x25,
		Up = 0x26,
		Right = 0x27,
		Down = 0x28,
		Escape = 0x1b,
		Zero = 0x30,
		One = 0x31,
		Two = 0x32,
		Three = 0x33,
		Four = 0x34,
		Five = 0x35,
		Six = 0x36,
		Seven = 0x37,
		Eight = 0x38,
		Nine = 0x39,
		A = 0x41,
		B = 0x42,
		C = 0x43,
		D = 0x44,
		E = 0x45,
		F = 0x46,
		G = 0x47,
		H = 0x48,
		I = 0x49,
		J = 0x4a,
		K = 0x4b,
		L = 0x4c,
		M = 0x4d,
		N = 0x4e,
		O = 0x4f,
		P = 0x50,
		Q = 0x51,
		R = 0x52,
		S = 0x53,
		T = 0x54,
		U = 0x55,
		V = 0x56,
		X = 0x57,
		Y = 0x58,
		Z = 0x59,
	};

	// Interface for consumers of keyboard input
	class IKeyboardInputConsumer
	{
	public:
		// Called when a key is pressed; return true if input is handled
		virtual bool OnKeyDown(Key key) = 0;

		// Called when a key is released; return true if input is handled
		virtual bool OnKeyUp(Key key) = 0;
	};

	// Provides hooks for events coming out of the console input stream
	class ConsoleEventStream
	{
	public:
		// Constructor sets up the console for processing the event stream
		ConsoleEventStream();

		// Destructor restores the console's original mode
		~ConsoleEventStream();

		// Processes all events in the input stream
		void ProcessEvents();

		// Registers a consumer of keyboard input
		// Consumers called in order of registration
		void RegisterKeyboardInputConsumer(IKeyboardInputConsumer* consumer);

		// Unregisters a consumer of keyboard input
		void UnregisterKeyboardInputConsumer(IKeyboardInputConsumer* consumer);

		// Delete copy/move construction and assignment
	private:
		ConsoleEventStream(ConsoleEventStream&) = delete;
		ConsoleEventStream(ConsoleEventStream&&) = delete;
		ConsoleEventStream& operator=(ConsoleEventStream&) = delete;
		ConsoleEventStream& operator=(ConsoleEventStream&&) = delete;

	private:
		// Console configuration at construction. Restored at destruction.
		CachedConsoleState m_cachedConsoleState;

		// Registered consumers of keyboard input
		std::vector<IKeyboardInputConsumer*> m_keyConsumers;
	};
} // namespace console
} // namespace nu