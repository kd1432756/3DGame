#pragma once

#include "../PlayerUIBase/PlayerUIBase.h"

class InteractionPrompt : public PlayerUIBase
{
public:
	void Init() override;
	void Update() override;
	void DrawSprite() override;

private:
	std::string m_prompt = "";

	bool m_visible = false;

	KdTexture m_texKeyE;
	bool m_showKeyIcon = false;
};
