#include "NuEngine/Assertions.h"

#include <cassert>
#include <exception>
#include <iostream>

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

			std::cerr << "\x1b[31m"
					  << "Assertion failed"
					  << "\x1b[37m"
					  << ": "
					  << "\x1b[91m" << condition << "\x1b[37m"
					  << "\n  in "
					  << "\x1b[36m" << loc.function_name() << "\x1b[37m"
					  << "\n  at "
					  << "\x1b[33m" << loc.file_name() << "\x1b[37m"
					  << "("
					  << "\x1b[33m" << loc.line() << "\x1b[37m"
					  << ")"
					  << "\x1b[0m" << '\n';

			std::terminate();
		}
	} // namespace details

	void VerifyNotReached(std::source_location loc)
	{
		details::VerifyElseCrashImpl(false, "false", loc);
	}
} // namespace asserts
} // namespace nu