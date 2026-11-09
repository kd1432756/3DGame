#pragma once

#include "../TextPopupUI/TextPopupUI.h"

// 回転しながら揺れて消える文字(揺れは徐々に弱くなる)
class ShakeTextUI : public TextPopupUI
{
public:
	ShakeTextUI();

protected:
	void OnUpdate(float dt, float t) override;
};
