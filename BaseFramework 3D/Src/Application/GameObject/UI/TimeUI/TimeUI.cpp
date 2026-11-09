#include "TimeUI.h"

#include "../../../System/DayCycle/DayCycle.h"

void TimeUI::Init()
{
	m_gaugeTex = LoadTexture("Asset/Textures/UI/TimeGauge.png");
	m_indicatorTex = LoadTexture("Asset/Textures/UI/TimeIndicator.png");
	m_sunTex = LoadTexture("Asset/Textures/UI/Sun.png");
	m_moonTex = LoadTexture("Asset/Textures/UI/Moon.png");
}

void TimeUI::DrawSprite()
{
	DrawTimeGauge();
	DrawTimeIcons();
	DrawIndicator();
}

// ※ UIBase::DrawGauge(進捗バー用)と区別するため DrawTimeGauge という名前にしている
void TimeUI::DrawTimeGauge()
{
	if (!m_gaugeTex)
	{
		return;
	}

	KdShaderManager::Instance().m_spriteShader.DrawTex(
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

	KdShaderManager::Instance().m_spriteShader.DrawTex(
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
