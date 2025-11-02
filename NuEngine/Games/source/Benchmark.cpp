#include "Benchmark.h"

#include "NuEngine/Assertions.h"
#include "NuEngine/Engine.h"
#include "NuEngine/Game.h"
#include "NuEngine/VirtualTerminalSequences.h"

using namespace std::literals;

constexpr auto numFramesPerPhase = 500;

enum class Color : uint8_t
{
	Red = 0,
	Green,
	Yellow,
	Blue,
	Magenta,
	Cyan,
	White,

	BrightRed,
	BrightGreen,
	BrightYellow,
	BrightBlue,
	BrightMagenta,
	BrightCyan,
	BrightWhite,

	// Custom colors manually created via vt::color::ForegroundRGB
	Custom01,
	Custom02,
	Custom03,
	Custom04,
	Custom05,
	Custom06,
	Custom07,
	Custom08,
	Custom09,
	Custom10,
	Custom11,
	Custom12,

	// Total number of colors
	Size
};

using namespace nu::console;
const std::array<std::string, static_cast<size_t>(Color::Size)> colors{
	std::string(vt::color::ForegroundRed),
	std::string(vt::color::ForegroundGreen),
	std::string(vt::color::ForegroundYellow),
	std::string(vt::color::ForegroundBlue),
	std::string(vt::color::ForegroundMagenta),
	std::string(vt::color::ForegroundCyan),
	std::string(vt::color::ForegroundWhite),
	std::string(vt::color::ForegroundBrightRed),
	std::string(vt::color::ForegroundBrightGreen),
	std::string(vt::color::ForegroundBrightYellow),
	std::string(vt::color::ForegroundBrightBlue),
	std::string(vt::color::ForegroundBrightMagenta),
	std::string(vt::color::ForegroundBrightCyan),
	std::string(vt::color::ForegroundBrightWhite),
	vt::color::ForegroundRGB(0, 0, 0),
	vt::color::ForegroundRGB(50, 50, 50),
	vt::color::ForegroundRGB(100, 100, 100),
	vt::color::ForegroundRGB(150, 150, 150),
	vt::color::ForegroundRGB(100, 100, 100),
	vt::color::ForegroundRGB(150, 150, 150),
	vt::color::ForegroundRGB(200, 200, 200),
	vt::color::ForegroundRGB(250, 250, 250),
	vt::color::ForegroundRGB(255, 255, 255),
	vt::color::ForegroundRGB(255, 0, 0),
	vt::color::ForegroundRGB(0, 255, 0),
	vt::color::ForegroundRGB(0, 0, 255),
};

Benchmark::Benchmark()
{
	m_phaseConfigs = {
		PhaseConfig{ 10, false }, PhaseConfig{ 30, false }, PhaseConfig{ 50, false }, PhaseConfig{ 70, false }, PhaseConfig{ 90, false },
		PhaseConfig{ 10, true },  PhaseConfig{ 30, true },  PhaseConfig{ 50, true },  PhaseConfig{ 70, true },  PhaseConfig{ 90, true },
	};
}

void Benchmark::BeginPlay()
{
	Restart();
}

void Benchmark::Restart()
{
	GetEngine()->SetTargetFramesPerSecond(60);

	auto [width, height] = GetEngine()->GetRendererSize();
	m_noise.resize(width * height);
	m_width = width;
	m_height = height;

	// Seed noise with random characters and colors
	std::uniform_int_distribution charDistribution{ static_cast<int>('0'), static_cast<int>('z') };
	std::uniform_int_distribution colorDistribution{ 0, static_cast<int>(Color::Size) - 1 };
	for (auto& [c, col] : m_noise)
	{
		c = charDistribution(m_rng);
		col = colorDistribution(m_rng);
	}

	// Store the original noise for subsequent test phases
	m_noiseOriginal = m_noise;

	// Reset state
	m_accruedTime = 0us;
	m_currentFrame = 0;
	m_phase = -1;

	// Invalidate any frame timings
	m_phaseFrameTimings.clear();
	m_phaseFrameTimings.resize(m_phaseConfigs.size());
	for (auto& frameTimings : m_phaseFrameTimings)
	{
		frameTimings.clear();
		frameTimings.reserve(numFramesPerPhase);
	}

	// Invalidate any results
	m_phaseResults.clear();
}

void Benchmark::EndPlay()
{
}

void Benchmark::Tick(std::chrono::duration<double> deltaTime)
{
	auto incrementPhase = [this]()
	{
		m_accruedTime = 0us;
		m_currentFrame = 0;
		m_noise = m_noiseOriginal;
		++m_phase;

		if (m_phase >= 0 && m_phase < m_phaseConfigs.size())
		{
			// Disable framerate limit during test phases
			GetEngine()->SetTargetFramesPerSecond(0);
		}
		else
		{
			// Enable framerate limit before and after the test phases
			GetEngine()->SetTargetFramesPerSecond(60);
		}
	};

	if (m_phase == -1)
	{
		m_accruedTime += std::chrono::duration_cast<std::chrono::microseconds>(deltaTime);
		if (m_accruedTime < 3s)
		{
			return;
		}

		incrementPhase();
	}

	if (m_phase == m_phaseConfigs.size())
	{
		if (m_phaseResults.empty())
		{
			m_phaseResults.resize(m_phaseConfigs.size());
		}

		VerifyElseCrash(m_phaseFrameTimings.size() == m_phaseResults.size());
		for (auto i = 0; i < m_phaseResults.size(); ++i)
		{
			auto& phaseResult = m_phaseResults[i];
			auto& phaseFrameTimings = m_phaseFrameTimings[i];
			phaseResult.frames = phaseFrameTimings.size();
			for (const auto& frameTime : phaseFrameTimings)
			{
				phaseResult.averageFrameTimings.totalFrameTime += frameTime.totalFrameTime;
				phaseResult.averageFrameTimings.tickTime += frameTime.tickTime;
				phaseResult.averageFrameTimings.renderTime += frameTime.renderTime;
				phaseResult.averageFrameTimings.presentTime += frameTime.presentTime;
				phaseResult.averageFrameTimings.idleTime += frameTime.idleTime;
			}
			phaseResult.averageFrameTimings.totalFrameTime /= static_cast<double>(phaseResult.frames);
			phaseResult.averageFrameTimings.tickTime /= static_cast<double>(phaseResult.frames);
			phaseResult.averageFrameTimings.renderTime /= static_cast<double>(phaseResult.frames);
			phaseResult.averageFrameTimings.presentTime /= static_cast<double>(phaseResult.frames);
			phaseResult.averageFrameTimings.idleTime /= static_cast<double>(phaseResult.frames);
		}
		return;
	}

	if (m_currentFrame != 0)
	{
		m_phaseFrameTimings[m_phase].emplace_back(GetEngine()->GetLastFrameTimings());
	}

	++m_currentFrame;
	for (auto i = 0u; i < m_noise.size(); ++i)
	{
		if ((m_currentFrame + i) % 100 >= m_phaseConfigs[m_phase].changePercent)
		{
			// Don't change the current noise entry
			continue;
		}

		// Increment the character and color
		auto& [c, col] = m_noise[i];
		if (++c > 'z')
		{
			c = '0';
		}
		col = (col + 1) % colors.size();
	}

	if (m_currentFrame > numFramesPerPhase)
	{
		incrementPhase();
		return;
	}
}

void Benchmark::Render(nu::console::ConsoleRenderer& renderer)
{
	if (m_phase == -1)
	{
		uint16_t y = 0;
		auto charactersLabel = std::format("{}x{}"sv, m_width, m_height);
		renderer.DrawString(0, y, charactersLabel, vt::color::ForegroundBrightCyan);
		renderer.DrawString(charactersLabel.size(), y++, " characters rendered each frame."sv);
		renderer.DrawString(0, y++, std::format("Benchmark will simulate/render {} frames of random symbols and colors:"sv, numFramesPerPhase));
		for (int i = 0; i < m_phaseConfigs.size(); ++i)
		{
			renderer.DrawString(
				0,
				y++,
				std::format(
					"    Test phase {:>2} - {}% of symbols {}change each frame"sv,
					i + 1,
					m_phaseConfigs[i].changePercent,
					m_phaseConfigs[i].renderColor ? "and colors " : ""));
		}
		renderer.DrawString(
			0,
			++y,
			std::format("Starting in {} seconds...", std::chrono::duration_cast<std::chrono::seconds>(3s - m_accruedTime).count()),
			vt::color::ForegroundBrightYellow);
		return;
	}

	if (m_phase == m_phaseConfigs.size())
	{
		if (m_phaseResults.empty())
		{
			return;
		}

		uint16_t y = 0;
		renderer.DrawString(0, y++, "Benchmark complete."sv);
		auto charactersLabel = std::format("{}x{}"sv, m_width, m_height);
		renderer.DrawString(0, y, charactersLabel, vt::color::ForegroundBrightCyan);
		renderer.DrawString(charactersLabel.size(), y++, " characters rendered each frame."sv);

		auto drawResults = [&y, &renderer](uint16_t x, const PhaseResult& phaseResult)
		{
			auto toMs = [](const auto& duration) { return std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(duration).count(); };
			constexpr auto x2 = ("    Average present time: "sv).size();
			renderer.DrawString(x, y, "    Total frames: "sv, vt::color::ForegroundWhite);
			renderer.DrawString(x + x2, y++, std::format("{:>7}", phaseResult.frames), vt::color::ForegroundBrightWhite);
			renderer.DrawString(x, y, "    Average frame time:   "sv, vt::color::ForegroundWhite);
			renderer.DrawString(x + x2, y++, std::format("{:>5.2f}ms", toMs(phaseResult.averageFrameTimings.totalFrameTime)), vt::color::ForegroundBrightWhite);
			renderer.DrawString(x, y, "    Average tick time:    "sv, vt::color::ForegroundWhite);
			renderer.DrawString(x + x2, y++, std::format("{:>5.2f}ms", toMs(phaseResult.averageFrameTimings.tickTime)), vt::color::ForegroundBrightWhite);
			renderer.DrawString(x, y, "    Average render time:  "sv, vt::color::ForegroundWhite);
			renderer.DrawString(x + x2, y++, std::format("{:>5.2f}ms", toMs(phaseResult.averageFrameTimings.renderTime)), vt::color::ForegroundBrightWhite);
			renderer.DrawString(x, y, "    Average present time: "sv, vt::color::ForegroundWhite);
			renderer.DrawString(x + x2, y++, std::format("{:>5.2f}ms", toMs(phaseResult.averageFrameTimings.presentTime)), vt::color::ForegroundBrightYellow);
			renderer.DrawString(x, y, "    Average idle time:    "sv, vt::color::ForegroundWhite);
			renderer.DrawString(x + x2, y++, std::format("{:>5.2f}ms", toMs(phaseResult.averageFrameTimings.idleTime)), vt::color::ForegroundBrightWhite);
		};

		constexpr auto resultsPerColumn = 5;
		VerifyElseCrash(m_phaseResults.size() <= 2 * resultsPerColumn);

		uint16_t yStart = y;
		for (int i = 0; i < m_phaseConfigs.size() && i < resultsPerColumn; ++i)
		{
			++y;
			renderer.DrawString(
				0,
				y++,
				std::format(
					"Test {} - {}% of symbols {}change each frame"sv,
					i + 1,
					m_phaseConfigs[i].changePercent,
					m_phaseConfigs[i].renderColor ? "and colors " : ""));
			drawResults(0, m_phaseResults[i]);
		}

		y = yStart;
		for (int i = resultsPerColumn; i < m_phaseConfigs.size(); ++i)
		{
			constexpr auto labelLen = "Test  5- 90% of symbols change each frame"sv.size();
			auto x = std::max(static_cast<uint16_t>(labelLen) + 4, renderer.GetWidth() / 2);
			++y;
			renderer.DrawString(
				x,
				y++,
				std::format(
					"Test {} - {}% of symbols {}change each frame"sv,
					i + 1,
					m_phaseConfigs[i].changePercent,
					m_phaseConfigs[i].renderColor ? "and colors " : ""));
			drawResults(x, m_phaseResults[i]);
		}
		return;
	}

	auto width = renderer.GetWidth();
	for (auto i = 0u; i < m_noise.size(); ++i)
	{
		const auto& [c, col] = m_noise[i];
		renderer.DrawChar(i % width, i / width, c, m_phaseConfigs[m_phase].renderColor ? colors[col] : vt::color::ForegroundWhite);
	}

	uint16_t y = 0;
	constexpr auto x = ("Present time: "sv).size();
	renderer.DrawString(0, y, "Test phase:   "sv, vt::color::ForegroundWhite);
	renderer.DrawString(x, y++, std::format("{:>7}", m_phase + 1), vt::color::ForegroundBrightYellow);
	renderer.DrawString(0, y, "Entropy:      "sv, vt::color::ForegroundWhite);
	renderer.DrawString(x, y++, std::format("{:>6}%", m_phaseConfigs[m_phase].changePercent), vt::color::ForegroundBrightBlue);
	renderer.DrawString(0, y, "Frame:        "sv, vt::color::ForegroundWhite);
	renderer.DrawString(x, y++, std::format("{:>7}", m_currentFrame), vt::color::ForegroundBrightCyan);
	renderer.DrawString(0, y, "FPS:          "sv, vt::color::ForegroundWhite);
	renderer.DrawString(x, y++, std::format("{:>7}", std::round(1.0f / GetEngine()->GetLastFrameTime().count())), vt::color::ForegroundBrightGreen);
	renderer.DrawString(0, y, "Frame time:   "sv, vt::color::ForegroundWhite);
	renderer.DrawString(x, y++, std::format("{:>5.2f}ms", GetEngine()->GetLastFrameTimeMs().count()), vt::color::ForegroundBrightWhite);
	renderer.DrawString(0, y, "Tick time:    "sv, vt::color::ForegroundWhite);
	renderer.DrawString(x, y++, std::format("{:>5.2f}ms", GetEngine()->GetLastTickTimeMs().count()), vt::color::ForegroundBrightWhite);
	renderer.DrawString(0, y, "Render time:  "sv, vt::color::ForegroundWhite);
	renderer.DrawString(x, y++, std::format("{:>5.2f}ms", GetEngine()->GetLastRenderTimeMs().count()), vt::color::ForegroundBrightWhite);
	renderer.DrawString(0, y, "Present time: "sv, vt::color::ForegroundWhite);
	renderer.DrawString(x, y++, std::format("{:>5.2f}ms", GetEngine()->GetLastPresentTimeMs().count()), vt::color::ForegroundBrightWhite);
	renderer.DrawString(0, y, "Idle time:    "sv, vt::color::ForegroundWhite);
	renderer.DrawString(x, y++, std::format("{:>5.2f}ms", GetEngine()->GetLastIdleTimeMs().count()), vt::color::ForegroundBrightWhite);
}

void Benchmark::OnWindowResize(uint16_t /*width*/, uint16_t /*height*/)
{
	if (m_phase < m_phaseConfigs.size())
	{
		Restart();
	}
}

bool Benchmark::OnKeyDown(nu::console::Key key)
{
	if (key == nu::console::Key::R)
	{
		Restart();
		return true;
	}
	return false;
}
