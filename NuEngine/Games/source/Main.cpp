#include "NuEngine/Engine.h"

#include "Benchmark.h"
#include "Snowflakes.h"

int main()
{
	nu::engine::Engine engine;
	Benchmark game;
	engine.StartGame(game);
}