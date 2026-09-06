#include "FloatingTextUI.h"

void FloatingTextUI::Init()
{}

void FloatingTextUI::Update()
{
	const float deltaTime = 1.0f / 60.0f;

	m_elapsedTime += deltaTime;

	m_pos.y += 0.5f;

	if (m_elapsedTime >= m_lifeTime)
	{
		m_isExpired = true;
	}
}

void FloatingTextUI::DrawSprite()
{
	KdShaderManager::Instance().m_spriteShader.DrawFont(
		m_pos,
		&kYellowColor,
		Math::Vector2{ 0.5f, 0.5f },
		FontID::Future32,
		m_text.c_str()
	);
}
