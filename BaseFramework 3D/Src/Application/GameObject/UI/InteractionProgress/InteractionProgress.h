#pragma once

#include "../PlayerUIBase/PlayerUIBase.h"

class InteractionProgress : public PlayerUIBase
{
public:
	void Update() override;
	void DrawSprite() override;

private:
	bool  m_visible = false;
	float m_progress = 0.0f;
};
