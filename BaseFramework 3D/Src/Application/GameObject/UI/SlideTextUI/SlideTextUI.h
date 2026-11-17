#pragma once

#include "../TextPopupUI/TextPopupUI.h"

class SlideTextUI : public TextPopupUI
{
public:
	SlideTextUI();

	void Init() override;

protected:
	void OnUpdate(float dt, float t) override;

private:
	static constexpr float kSlideInTime = 0.4f;
	static constexpr float kStayTime = 1.0f;
	static constexpr float kSlideOutTime = 0.4f;

	static constexpr float kStartX = -700.0f;
	static constexpr float kCenterX = 0.0f;
	static constexpr float kEndX = 700.0f;
	static constexpr float kPosY = 150.0f;
};
