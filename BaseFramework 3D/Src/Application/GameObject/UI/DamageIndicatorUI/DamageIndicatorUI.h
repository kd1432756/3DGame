#pragma once

#include "../PlayerUIBase/PlayerUIBase.h"

class DamageIndicatorUI : public PlayerUIBase
{
public:
	void Init() override;
	void Update() override;
	void DrawSprite() override;

	void SetPosition(const Math::Vector2& pos) { m_uiPos = pos; }

private:
	std::shared_ptr<KdTexture> m_texHouse = nullptr;
	std::shared_ptr<KdTexture> m_texBadge = nullptr;

	Math::Vector2 m_uiPos = { 540.0f, 220.0f };

	int   m_count = 0;
	int   m_prevCount = 0;

	static constexpr float kBounceTime = 0.3f;
	float m_bounceTimer = 0.0f;
	float m_badgeScale = 1.0f;

	Math::Color m_houseColor = { 1, 1, 1, 1 };

	static constexpr float kHouseSize = 64.0f;
	static constexpr float kBadgeSize = 28.0f;
};
