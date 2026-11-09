#pragma once

#include "../UIBase/UIBase.h"

// ResultUI の上に重ねる暗転用UI
class ResultFadeUI : public UIBase
{
public:
	// ポーズ中も動くので常時更新
	ResultFadeUI() : UIBase(true) {}

	void DrawSprite() override;

	void SetAlpha(float a) { m_alpha = a; }

private:
	float m_alpha = 0.0f;
};
