#include "NotificationUI.h"

void NotificationUI::Init()
{
	m_updateGroup = UpdateGroup::Always;
	m_pos.x = m_startX;
}

void NotificationUI::Update()
{
	const float deltaTime = 1.0f / 60.0f;

	// スタック位置への滑らかな移動（全ステート共通）
	m_pos.y += (m_targetY - m_pos.y) * m_moveSpeed;

	switch (m_state)
	{
	case State::SlideIn:
	{
		m_elapsedTime += deltaTime;

		float t = std::clamp(m_elapsedTime / m_slideInTime, 0.0f, 1.0f);
		t = 1.0f - std::pow(1.0f - t, 3.0f);	// Ease Out

		m_pos.x = m_startX + (m_centerX - m_startX) * t;

		if (m_elapsedTime >= m_slideInTime)
		{
			m_pos.x = m_centerX;
			m_elapsedTime = 0.0f;
			m_state = State::Stay;
		}
		break;
	}

	case State::Stay:
	{
		m_elapsedTime += deltaTime;

		if (m_elapsedTime >= m_stayTime)
		{
			m_elapsedTime = 0.0f;
			m_state = State::FadeOut;
		}
		break;
	}

	case State::FadeOut:
	{
		m_elapsedTime += deltaTime;

		float t = std::clamp(m_elapsedTime / m_fadeOutTime, 0.0f, 1.0f);
		m_alpha = 1.0f - t;

		if (m_elapsedTime >= m_fadeOutTime)
		{
			m_isExpired = true;	// ここで初めて枠が空く
		}
		break;
	}
	}
}

void NotificationUI::DrawSprite()
{
	Math::Color color = { 1.0f, 0.45f, 0.10f, m_alpha };

	KdShaderManager::Instance().m_spriteShader.DrawFont(
		m_pos,
		&color,
		Math::Vector2{ 0.5f, 0.5f },
		0.0f,
		FontID::Future48,
		m_text.c_str()
	);
}