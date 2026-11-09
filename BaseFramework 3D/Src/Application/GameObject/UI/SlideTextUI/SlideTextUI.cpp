#include "SlideTextUI.h"

SlideTextUI::SlideTextUI()
{
	// 寿命 = 滑り込み + 停止 + 滑り出し
	m_lifeTime = kSlideInTime + kStayTime + kSlideOutTime;
	m_fontId = FontID::Future64;
	m_color = { 1.0f, 0.45f, 0.10f, 1.0f };
}

void SlideTextUI::Init()
{
	// 画面左端から開始
	m_pos.x = kStartX;
	m_pos.y = kPosY;
}

void SlideTextUI::OnUpdate(float dt, float t)
{
	const float e = m_elapsedTime;

	if (e < kSlideInTime)
	{
		// 滑り込み (Ease Out)
		const float s = EaseOutCubic(e / kSlideInTime);
		m_pos.x = kStartX + (kCenterX - kStartX) * s;
	}
	else if (e < kSlideInTime + kStayTime)
	{
		// 停止
		m_pos.x = kCenterX;
	}
	else
	{
		// 滑り出し (Ease In)
		const float s = std::clamp(
			(e - kSlideInTime - kStayTime) / kSlideOutTime, 0.0f, 1.0f);
		m_pos.x = kCenterX + (kEndX - kCenterX) * EaseInCubic(s);
	}
}
