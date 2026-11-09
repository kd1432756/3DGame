#pragma once

#include "../TextPopupUI/TextPopupUI.h"

// 上にふわっと浮かびながら消える文字
class FloatingTextUI : public TextPopupUI
{
public:
	FloatingTextUI();

protected:
	void OnUpdate(float dt, float t) override;
};
