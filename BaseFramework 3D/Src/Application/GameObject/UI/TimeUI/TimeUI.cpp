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

void TimeUI::DrawTimeGauge()
{
	if (!m_gaugeTex)
	{
		return;
	}

	KdShaderManager::Instance().m_spriteShader.DrawTex(
		m_gaugeTex,
		kStartX,
		kStartY,
		kGaugeWidth,
		kGaugeHeight
	);
}

void TimeUI::DrawTimeIcons()
{
	const float gaugeStartX = kStartX - kGaugeWidth * 0.5f;

	const float sunX = gaugeStartX + kGaugeLeft;

	const float moonX = gaugeStartX + kGaugeRight;

	if (m_sunTex)
	{
		KdShaderManager::Instance().m_spriteShader.DrawTex(
			m_sunTex,
			static_cast<int>(sunX),
			kStartY,
			kTimeIconSize,
			kTimeIconSize
		);
	}

	if (m_moonTex)
	{
		KdShaderManager::Instance().m_spriteShader.DrawTex(
			m_moonTex,
			static_cast<int>(moonX),
			kStartY,
			kTimeIconSize,
			kTimeIconSize
		);
	}
}

void TimeUI::DrawIndicator()
{
	if (!m_indicatorTex)
	{
		return;
	}

	const float progress = DayCycle::Instance().GetTimeProgress();

	const float gaugeStartX = kStartX - kGaugeWidth * 0.5f;

	const float indicatorX = gaugeStartX + kGaugeLeft + (kGaugeRight - kGaugeLeft) * progress;

	const float indicatorY = kStartY;

	KdShaderManager::Instance().m_spriteShader.DrawTex(
		m_indicatorTex,
		static_cast<int>(indicatorX),
		static_cast<int>(indicatorY),
		kIndicatorSize,
		kIndicatorSize,
		nullptr,
		&kWhiteColor,
		Math::Vector2{ 0.5f, 1.5f }
	);
}
