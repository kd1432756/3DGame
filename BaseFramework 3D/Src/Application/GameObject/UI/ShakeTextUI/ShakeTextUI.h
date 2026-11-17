#pragma once

#include "../TextPopupUI/TextPopupUI.h"

class ShakeTextUI : public TextPopupUI
{
public:
	ShakeTextUI();

protected:
	void OnUpdate(float dt, float t) override;
};
