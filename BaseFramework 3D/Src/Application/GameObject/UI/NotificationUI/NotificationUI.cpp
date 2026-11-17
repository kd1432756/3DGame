#include "NotificationUI.h"

NotificationUI::NotificationUI()
{
	m_lifeTime = kSlideInTime + kStayTime + kFadeOutTime;
	m_fontId = FontID::Future48;
	m_color = { 1.0f, 0.45f, 0.10f, 1.0f };
}

void NotificationUI::Init()
{
	m_pos.x = kStartX;
}

void NotificationUI::OnUpdate(float dt, float t)
{
	m_pos.y += (m_targetY - m_pos.y) * kMoveSpeed;

	const float e = m_elapsedTime;

	if (e < kSlideInTime)
	{
		// 横から
		m_pos.x = kStartX + (kCenterX - kStartX) * EaseOutCubic(e / kSlideInTime);
		m_alpha = 1.0f;
	}
	else if (e < kSlideInTime + kStayTime)
	{
		m_pos.x = kCenterX;
		m_alpha = 1.0f;
	}
	else
	{
		// フェードアウト
		const float s = std::clamp((e - kSlideInTime - kStayTime) / kFadeOutTime, 0.0f, 1.0f);
		m_pos.x = kCenterX;
		m_alpha = 1.0f - s;
	}
}
