#include "SlideTextUI.h"

void SlideTextUI::Init()
{
	m_updateGroup = UpdateGroup::Always;
	m_pos.x = m_startX;	// 画面左端から開始
	m_pos.y = 150.0f;
}

void SlideTextUI::Update()
{
	const float deltaTime = 1.0f / 60.0f;

	switch (m_state)
	{
	case State::SlideIn:
	{
		m_elapsedTime += deltaTime;

		float t = m_elapsedTime / m_slideInTime;
		t = std::clamp(t, 0.0f, 1.0f);

		// Ease Out
		t = 1.0f - std::pow(1.0f - t, 3.0f);

		m_pos.x = m_startX
			+ (m_centerX - m_startX) * t;

		if (t >= 1.0f)
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
			m_state = State::SlideOut;
		}

		break;
	}

	case State::SlideOut:
	{
		m_elapsedTime += deltaTime;

		float t = m_elapsedTime / m_slideOutTime;
		t = std::clamp(t, 0.0f, 1.0f);

		// Ease In
		t = t * t * t;

		m_pos.x = m_centerX
			+ (m_endX - m_centerX) * t;

		if (t >= 1.0f)
		{
			m_isExpired = true;
		}

		break;
	}
	}
}

void SlideTextUI::DrawSprite()
{
	Math::Color color = { 1.0f, 0.45f, 0.10f, 1.0f };

	KdShaderManager::Instance().m_spriteShader.DrawFont(
		m_pos,
		&color,
		Math::Vector2{ 0.5f, 0.5f },
		0.0f,
		FontID::Future64,
		m_text.c_str()
	);
}
