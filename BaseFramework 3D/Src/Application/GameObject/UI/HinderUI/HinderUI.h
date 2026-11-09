#pragma once

#include "../PlayerUIBase/PlayerUIBase.h"

class HinderUI : public PlayerUIBase
{
public:
	void Init() override;
	void Update() override;
	void DrawSprite() override;

private:
	bool  m_canHinder = false;
	bool  m_isHindering = false;
	float m_hinderRatio = 0.0f;
	int   m_frame = 0;   // 連打表示の点滅用

	KdTexture m_texKeyF;
	KdTexture m_texKeySpace;
};
