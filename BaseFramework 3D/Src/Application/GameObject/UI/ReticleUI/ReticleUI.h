#pragma once

#include "../PlayerUIBase/PlayerUIBase.h"

class Reticle : public PlayerUIBase
{
public:
	void Init() override;
	void Update() override;
	void DrawSprite() override;

private:
	std::shared_ptr<KdTexture> m_tex = nullptr;

	bool m_canInteract = false;

	bool m_isVisible = true;
};
