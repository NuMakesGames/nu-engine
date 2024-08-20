#include "NuEngine/Assertions.h"

#include <cassert>
#include <exception>
#include <iostream>

#include "NuEngine/VirtualTerminalSequences.h"

namespace nu
{
namespace asserts
{
	namespace details
	{
		void VerifyElseCrashImpl(bool bAssert, const char* condition, std::source_location loc)
		{
			if (bAssert)
			{
				return;
			}

			assert(bAssert);

			using namespace nu::console::vt;
			std::cerr << color::ForegroundRed << "Assertion failed" << color::ForegroundWhite << ": "
					  << color::ForegroundBrightRed << condition << color::ForegroundWhite << "\n  in "
					  << color::ForegroundCyan << loc.function_name() << color::ForegroundWhite << "\n  at "
					  << color::ForegroundYellow << loc.file_name() << color::ForegroundWhite << "("
					  << color::ForegroundBrightYellow << loc.line() << color::ForegroundWhite << ")" << color::Default
					  << '\n';

			std::terminate();
		}
	} // namespace details

	void VerifyNotReached(std::source_location loc)
	{
		details::VerifyElseCrashImpl(false, "false", loc);
	}
} // namespace asserts
} // namespace nu