#include "NuEngine/Engine.h"

#include "Benchmark.h"
#include "Snowflakes.h"

int main()
{
	std::ios_base::sync_with_stdio(false);
	nu::engine::Engine engine;
	Snowflakes game;
	engine.StartGame(game);
}