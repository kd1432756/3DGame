#include "FloatingTextUI.h"

void FloatingTextUI::Init()
{
	m_updateGroup = UpdateGroup::Always;
}

void FloatingTextUI::Update()
{
	const float deltaTime = 1.0f / 60.0f;

	m_elapsedTime += deltaTime;

	m_pos.y += 0.5f;

	float t = m_elapsedTime / m_lifeTime;

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

	if (m_elapsedTime >= m_lifeTime)
	{
		m_isExpired = true;
	}
}

void FloatingTextUI::DrawSprite()
{
	Math::Color color = m_color;
	color.w = m_alpha;

	KdShaderManager::Instance().m_spriteShader.DrawFont(
		m_pos,
		&color,
		Math::Vector2{ 0.5f, 0.5f },
		0.0f,
		m_fontId,
		m_text.c_str()
	);
}
