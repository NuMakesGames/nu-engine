#include "NuEngine/Engine.h"

#include "Benchmark.h"
#include "Snowflakes.h"

int main()
{
	std::ios_base::sync_with_stdio(false);
	nu::engine::Engine engine;
	Benchmark game;
	engine.StartGame(game);
}