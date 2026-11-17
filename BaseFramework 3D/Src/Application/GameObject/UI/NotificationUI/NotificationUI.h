#pragma once

#include "../TextPopupUI/TextPopupUI.h"

class NotificationUI : public TextPopupUI
{
public:
	NotificationUI();

	void Init() override;

	void SetStartY(float y) { m_pos.y = y; m_targetY = y; }

	// 位置が変わったとき
	void SetTargetY(float y) { m_targetY = y; }

protected:
	void OnUpdate(float dt, float t) override;

private:
	float m_targetY = 0.0f;

	static constexpr float kStartX = -900.0f;
	static constexpr float kCenterX = 0.0f;
	static constexpr float kSlideInTime = 0.4f;
	static constexpr float kStayTime = 3.0f;
	static constexpr float kFadeOutTime = 0.4f;
	static constexpr float kMoveSpeed = 0.2f;
};
