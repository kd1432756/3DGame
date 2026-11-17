#pragma once

#include "../UIBase/UIBase.h"

class ResultFadeUI : public UIBase
{
public:
	// 常時更新
	ResultFadeUI() : UIBase(true) {}

	void DrawSprite() override;

	void SetAlpha(float a) { m_alpha = a; }

private:
	float m_alpha = 0.0f;
};
