#include "NuEngine/Game.h"

#include "NuEngine/Engine.h"

namespace nu
{
namespace engine
{
	Game::Game()
	{
	}

	void Game::SetEngine(class Engine* engine)
	{
		m_engine = engine;
	}

	Engine* Game::GetEngine()
	{
		return m_engine;
	}

} // namespace engine
} // namespace nu
