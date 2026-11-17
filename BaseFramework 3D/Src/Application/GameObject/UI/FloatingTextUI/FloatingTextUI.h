#pragma once

#include "../TextPopupUI/TextPopupUI.h"

class FloatingTextUI : public TextPopupUI
{
public:
	FloatingTextUI();

protected:
	void OnUpdate(float dt, float t) override;
};
