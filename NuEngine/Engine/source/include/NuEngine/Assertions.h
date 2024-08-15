#pragma once

#include <source_location>

namespace nu
{
namespace asserts
{
	namespace details
	{
		// Crashes the program if the assertion is false.
		void VerifyElseCrashImpl(
			bool bAssert,
			const char* condition,
			std::source_location loc = std::source_location::current());

	} // namespace details

	// Crashes the program if executed. Use to annotate code locations expected to be unreachable.
	void VerifyNotReached(std::source_location loc = std::source_location::current());

} // namespace asserts
} // namespace nu

// Macro version that automatically echoes the assertion into the message
#define VerifyElseCrash(bAssert)                                                                       \
	do                                                                                                 \
	{                                                                                                  \
		nu::asserts::details::VerifyElseCrashImpl(bAssert, #bAssert, std::source_location::current()); \
	}                                                                                                  \
	while (false);