#pragma once

#include <random>

#include "NuEngine/Engine.h"
#include "NuEngine/Game.h"

class Benchmark : public nu::engine::Game
{
public:
	Benchmark() = default;

	void BeginPlay() override;
	void EndPlay() override;
	void Tick(std::chrono::duration<double> deltaTime) override;
	void Render(nu::console::ConsoleRenderer& renderer) override;
	void OnWindowResize(uint16_t width, uint16_t height) override;
	bool OnKeyDown(nu::console::Key key) override;

	// Delete copy/move construction and assignment
private:
	Benchmark(Benchmark&) = delete;
	Benchmark(Benchmark&&) = delete;
	Benchmark& operator=(Benchmark&) = delete;
	Benchmark& operator=(Benchmark&&) = delete;

	void Restart();

private:
	uint32_t m_rngSeed = 42;
	std::mt19937 m_rng{ std::random_device{}() };

	uint64_t m_currentFrame = 0;
	uint8_t m_changePercent = 10;
	uint8_t m_phase = 0;

	std::chrono::microseconds m_accruedTime;

	std::vector<std::pair<char, uint8_t>> m_noise;
	std::vector<std::pair<char, uint8_t>> m_noiseOriginal;

	struct PhaseResult
	{
		uint64_t frames = 0;
		nu::engine::FrameTimings averageFrameTimings;
	};

	std::vector<std::vector<nu::engine::FrameTimings>> m_phaseFrameTimings;
	std::vector<PhaseResult> m_phaseResults;
};