#include "Benchmark.h"

#include "NuEngine/Assertions.h"
#include "NuEngine/Engine.h"
#include "NuEngine/Game.h"
#include "NuEngine/VirtualTerminalSequences.h"

using namespace std::literals;

constexpr auto numFramesPerPhase = 2000;
constexpr auto numPhases = 5;

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

void Benchmark::BeginPlay()
{
	// Disable frame rate limit
	GetEngine()->SetTargetFramesPerSecond(0);
	Restart();
}

void Benchmark::Restart()
{
	auto [width, height] = GetEngine()->GetRendererSize();
	m_noise.resize(width * height);

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
	m_changePercent = 10;
	m_phase = 0;

	// Invalidate any frame timings
	m_phaseFrameTimings.clear();
	m_phaseFrameTimings.resize(numPhases);
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
		if (m_phase != 0)
		{
			m_changePercent += 20;
		}
		m_noise = m_noiseOriginal;
		++m_phase;
	};

	if (m_phase == 0)
	{
		m_accruedTime += std::chrono::duration_cast<std::chrono::microseconds>(deltaTime);
		if (m_accruedTime < 3s)
		{
			return;
		}

		incrementPhase();
	}

	if (m_phase >= numPhases + 1)
	{
		if (m_phaseResults.empty())
		{
			m_phaseResults.resize(numPhases);
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
		m_phaseFrameTimings[m_phase - 1].emplace_back(GetEngine()->GetLastFrameTimings());
	}

	++m_currentFrame;
	for (auto i = 0u; i < m_noise.size(); ++i)
	{
		if ((m_currentFrame + i) % 100 >= m_changePercent)
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
	if (m_phase == 0)
	{
		uint16_t y = 0;
		renderer.DrawString(0, y++, "Benchmark will simulate/render frames of random symbols and colors, 5 times:"sv);
		renderer.DrawString(0, y++, "    Test 1 - 10% of symbols change each frame"sv);
		renderer.DrawString(0, y++, "    Test 2 - 30% of symbols change each frame"sv);
		renderer.DrawString(0, y++, "    Test 3 - 50% of symbols change each frame"sv);
		renderer.DrawString(0, y++, "    Test 4 - 70% of symbols change each frame"sv);
		renderer.DrawString(0, y++, "    Test 5 - 90% of symbols change each frame"sv);
		renderer.DrawString(
			0,
			++y,
			std::format("Starting in {} seconds...", std::chrono::duration_cast<std::chrono::seconds>(3s - m_accruedTime).count()),
			vt::color::ForegroundBrightYellow);
		return;
	}

	if (m_phase >= numPhases + 1)
	{
		if (m_phaseResults.empty())
		{
			return;
		}

		uint16_t y = 0;
		renderer.DrawString(0, y++, "Benchmark complete."sv);

		auto drawResults = [&y, &renderer](const PhaseResult& phaseResult)
		{
			auto toMs = [](const auto& duration) { return std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(duration).count(); };
			constexpr auto x = ("    Average present time: "sv).size();
			renderer.DrawString(0, y, "    Total frames: "sv, vt::color::ForegroundWhite);
			renderer.DrawString(x, y++, std::format("{:>7}", phaseResult.frames), vt::color::ForegroundBrightWhite);
			renderer.DrawString(0, y, "    Average frame time:   "sv, vt::color::ForegroundWhite);
			renderer.DrawString(x, y++, std::format("{:>5.2f}ms", toMs(phaseResult.averageFrameTimings.totalFrameTime)), vt::color::ForegroundBrightWhite);
			renderer.DrawString(0, y, "    Average tick time:    "sv, vt::color::ForegroundWhite);
			renderer.DrawString(x, y++, std::format("{:>5.2f}ms", toMs(phaseResult.averageFrameTimings.tickTime)), vt::color::ForegroundBrightWhite);
			renderer.DrawString(0, y, "    Average render time:  "sv, vt::color::ForegroundWhite);
			renderer.DrawString(x, y++, std::format("{:>5.2f}ms", toMs(phaseResult.averageFrameTimings.renderTime)), vt::color::ForegroundBrightWhite);
			renderer.DrawString(0, y, "    Average present time: "sv, vt::color::ForegroundWhite);
			renderer.DrawString(x, y++, std::format("{:>5.2f}ms", toMs(phaseResult.averageFrameTimings.presentTime)), vt::color::ForegroundBrightYellow);
			renderer.DrawString(0, y, "    Average idle time:    "sv, vt::color::ForegroundWhite);
			renderer.DrawString(x, y++, std::format("{:>5.2f}ms", toMs(phaseResult.averageFrameTimings.idleTime)), vt::color::ForegroundBrightWhite);
		};

		VerifyElseCrash(m_phaseResults.size() >= 5);

		++y;
		renderer.DrawString(0, y++, "Test 1 - 10% of symbols change each frame"sv);
		drawResults(m_phaseResults[0]);

		++y;
		renderer.DrawString(0, y++, "Test 2 - 30% of symbols change each frame"sv);
		drawResults(m_phaseResults[1]);

		++y;
		renderer.DrawString(0, y++, "Test 3 - 50% of symbols change each frame"sv);
		drawResults(m_phaseResults[2]);

		++y;
		renderer.DrawString(0, y++, "Test 4 - 70% of symbols change each frame"sv);
		drawResults(m_phaseResults[3]);

		++y;
		renderer.DrawString(0, y++, "Test 5 - 90% of symbols change each frame"sv);
		drawResults(m_phaseResults[4]);
		return;
	}

	auto width = renderer.GetWidth();
	for (auto i = 0u; i < m_noise.size(); ++i)
	{
		const auto& [c, col] = m_noise[i];
		renderer.DrawChar(i % width, i / width, c, colors[col]);
	}

	uint16_t y = 0;
	constexpr auto x = ("Present time: "sv).size();
	renderer.DrawString(0, y, "Test phase:   "sv, vt::color::ForegroundWhite);
	renderer.DrawString(x, y++, std::format("{:>7}", m_phase), vt::color::ForegroundBrightYellow);
	renderer.DrawString(0, y, "Entropy:      "sv, vt::color::ForegroundWhite);
	renderer.DrawString(x, y++, std::format("{:>6}%", m_changePercent), vt::color::ForegroundBrightBlue);
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
	if (m_phase <= numPhases)
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
