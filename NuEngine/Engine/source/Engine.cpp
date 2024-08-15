#include "NuEngine/Engine.h"

#include <string>

#include "NuEngine/Assertions.h"

// TODO: This is an example of a library function
std::string fnEngine()
{
	VerifyElseCrash(3 > 5);
	return "Hello from Engine!";
}
