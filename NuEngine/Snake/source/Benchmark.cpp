#include "Benchmark.h"

#include <string_view>

#include "NuEngine/Engine.h"
#include "NuEngine/Game.h"
#include "NuEngine/VirtualTerminalSequences.h"

using namespace std::literals;

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
}

void Benchmark::BeginPlay()
{
	// Disable frame rate limit
	GetEngine()->SetTargetFramesPerSecond(0);

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

	// Store the original noise for subssequent test phases
	m_noiseOriginal = m_noise;
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
		if (m_accruedTime < 5s)
		{
			return;
		}

		incrementPhase();
	}

	if (m_phase >= 6)
	{
		return;
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

	if (m_currentFrame > 20000)
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
		renderer.DrawString(0, y++, "Benchmark will render 20,000 frames of random symbols and colors, 5 times:"sv);
		renderer.DrawString(0, y++, "    Test 1 - 10% of symbols change each frame"sv);
		renderer.DrawString(0, y++, "    Test 2 - 30% of symbols change each frame"sv);
		renderer.DrawString(0, y++, "    Test 3 - 50% of symbols change each frame"sv);
		renderer.DrawString(0, y++, "    Test 4 - 70% of symbols change each frame"sv);
		renderer.DrawString(0, y++, "    Test 5 - 90% of symbols change each frame"sv);
		renderer.DrawString(
			0,
			++y,
			std::format("Starting in {} seconds...", std::chrono::duration_cast<std::chrono::seconds>(5s - m_accruedTime).count()),
			vt::color::ForegroundBrightYellow);
		return;
	}

	if (m_phase == 6)
	{
		// TODO: Show final results
		uint16_t y = 0;
		renderer.DrawString(0, y, "Benchmark complete. TODO RESULTS."sv);
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

void Benchmark::OnWindowResize(uint16_t width, uint16_t height)
{
}