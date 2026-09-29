#include "TimeUI.h"

#include "../../../System/DayCycle/DayCycle.h"

void TimeUI::Init()
{
	m_gaugeTex = std::make_shared<KdTexture>();
	m_gaugeTex->Load("Asset/Textures/UI/TimeGauge.png");
	m_indicatorTex = std::make_shared<KdTexture>();
	m_indicatorTex->Load("Asset/Textures/UI/TimeIndicator.png");
	m_sunTex = std::make_shared<KdTexture>();
	m_sunTex->Load("Asset/Textures/UI/Sun.png");
	m_moonTex = std::make_shared<KdTexture>();
	m_moonTex->Load("Asset/Textures/UI/Moon.png");
}

void TimeUI::Update()
{

}

void TimeUI::DrawSprite()
{
	auto dayCycle = DayCycle::Instance();

	DrawGauge();
	DrawTimeIcons();
	DrawIndicator();
}

void TimeUI::DrawGauge()
{
	if (!m_gaugeTex)
	{
		return;
	}

	auto& spriteShader =
		KdShaderManager::Instance().m_spriteShader;

	spriteShader.DrawTex(
		m_gaugeTex,
		StartX,
		StartY,
		GaugeWidth,
		GaugeHeight
	);
}

void TimeUI::DrawTimeIcons()
{
	auto& spriteShader =
		KdShaderManager::Instance().m_spriteShader;

	// ゲージ左端
	const float gaugeStartX =
		StartX - GaugeWidth * 0.5f;

	const float sunX =
		gaugeStartX + GaugeLeft;

	const float moonX =
		gaugeStartX + GaugeRight;

	if (m_sunTex)
	{
		spriteShader.DrawTex(
			m_sunTex,
			static_cast<int>(sunX),
			StartY,
			TimeIconSize,
			TimeIconSize
		);
	}

	if (m_moonTex)
	{
		spriteShader.DrawTex(
			m_moonTex,
			static_cast<int>(moonX),
			StartY,
			TimeIconSize,
			TimeIconSize
		);
	}
}

void TimeUI::DrawIndicator()
{
	if (!m_indicatorTex)
	{
		return;
	}

	auto& spriteShader =
		KdShaderManager::Instance().m_spriteShader;

	const float progress =
		DayCycle::Instance().GetTimeProgress();

	const float gaugeStartX =
		StartX - GaugeWidth * 0.5f;

	const float indicatorX =
		gaugeStartX
		+ GaugeLeft
		+ (GaugeRight - GaugeLeft) * progress;

	const float indicatorY =
		StartY;

	spriteShader.DrawTex(
		m_indicatorTex,
		static_cast<int>(indicatorX),
		static_cast<int>(indicatorY),
		IndicatorSize,
		IndicatorSize,
		nullptr,
		&kWhiteColor,
		Math::Vector2{ 0.5f, 1.5f }
	);
}