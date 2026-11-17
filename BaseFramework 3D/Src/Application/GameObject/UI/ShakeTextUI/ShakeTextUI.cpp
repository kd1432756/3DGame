#include "ShakeTextUI.h"

ShakeTextUI::ShakeTextUI()
{
	m_lifeTime = 1.5f;
	m_fontId = FontID::Future128;
	m_color = { 1.0f, 0.12f, 0.08f, 1.0f };
}

void ShakeTextUI::OnUpdate(float dt, float t)
{
	// 回転揺れ(６割り超えたら緩める)
	if (t < 0.6f)
	{
		const float shakeT = t / 0.6f;
		const float shakePower = 1.0f - shakeT;

		m_angle = std::sin(m_elapsedTime * 35.0f) * 5.0f * shakePower;
	}
	else
	{
		m_angle = 0.0f;
	}

	m_alpha = CalcFadeAlpha(t);
}
