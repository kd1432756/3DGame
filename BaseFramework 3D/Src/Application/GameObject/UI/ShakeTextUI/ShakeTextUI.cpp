#include "ShakeTextUI.h"

void ShakeTextUI::Init()
{}

void ShakeTextUI::Update()
{
	const float deltaTime = 1.0f / 60.0f;

	m_elapsedTime += deltaTime;

	float t =
		m_elapsedTime / m_lifeTime;

	t = std::clamp(t, 0.0f, 1.0f);

	// -------------------------
	// 回転揺れ
	// -------------------------

	if (t < 0.6f)
	{
		float shakeT =
			t / 0.6f;

		// 徐々に揺れを弱くする
		float shakePower =
			1.0f - shakeT;

		m_angle =
			std::sin(m_elapsedTime * 35.0f)
			* 5.0f
			* shakePower;
	}
	else
	{
		m_angle = 0.0f;
	}

	// -------------------------
	// フェード
	// -------------------------

	if (t < 0.1f)
	{
		m_alpha = t / 0.1f;
	}
	else if (t > 0.8f)
	{
		m_alpha =
			1.0f -
			((t - 0.8f) / 0.2f);
	}
	else
	{
		m_alpha = 1.0f;
	}

	// -------------------------
	// 終了
	// -------------------------

	if (t >= 1.0f)
	{
		m_isExpired = true;
	}
}


void ShakeTextUI::DrawSprite()
{
	Math::Color color = { 1.0f, 0.12f, 0.08f, m_alpha };

	KdShaderManager::Instance().m_spriteShader.DrawFont(
		Math::Vector2{ m_pos.x, m_pos.y },
		&color,
		Math::Vector2{ 0.5f, 0.5f },
		m_angle,
		FontID::Future128,
		m_text.c_str()
	);
}
