#include "SlideTextUI.h"

SlideTextUI::SlideTextUI()
{
	m_lifeTime = kSlideInTime + kStayTime + kSlideOutTime;
	m_fontId = FontID::Future64;
	m_color = { 1.0f, 0.45f, 0.10f, 1.0f };
}

void SlideTextUI::Init()
{
	m_pos.x = kStartX;
	m_pos.y = kPosY;
}

void SlideTextUI::OnUpdate(float dt, float t)
{
	const float e = m_elapsedTime;

	if (e < kSlideInTime)
	{
		// Ease Outで入る
		const float s = EaseOutCubic(e / kSlideInTime);
		m_pos.x = kStartX + (kCenterX - kStartX) * s;
	}
	else if (e < kSlideInTime + kStayTime)
	{
		m_pos.x = kCenterX;
	}
	else
	{
		// Ease Inで出る
		const float s = std::clamp(
			(e - kSlideInTime - kStayTime) / kSlideOutTime, 0.0f, 1.0f);
		m_pos.x = kCenterX + (kEndX - kCenterX) * EaseInCubic(s);
	}
}
