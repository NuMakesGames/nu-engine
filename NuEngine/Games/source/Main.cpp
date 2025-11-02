#include "NuEngine/Engine.h"
#include "NuEngine/ConsoleRenderer.h"

#include "Benchmark.h"
#include "Snowflakes.h"

int main()
{
	nu::engine::Engine engine;
	Snowflakes game;
	engine.StartGame(game);
}