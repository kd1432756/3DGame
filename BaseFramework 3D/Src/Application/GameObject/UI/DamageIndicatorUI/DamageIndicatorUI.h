#pragma once

#include "../PlayerUIBase/PlayerUIBase.h"

class DamageIndicatorUI : public PlayerUIBase
{
public:
	void Init() override;
	void Update() override;
	void DrawSprite() override;

	// 表示位置(画面中心が原点)
	void SetPosition(const Math::Vector2& pos) { m_uiPos = pos; }

private:
	std::shared_ptr<KdTexture> m_texHouse = nullptr;   // 家アイコン
	std::shared_ptr<KdTexture> m_texBadge = nullptr;   // 赤い丸

	Math::Vector2 m_uiPos = { 540.0f, 220.0f };

	int   m_count = 0;        // 現在の被害数
	int   m_prevCount = 0;    // 前フレームの被害数(増加検知用)

	// バッジのバウンド演出
	float m_bounceTimer = 0.0f;
	float m_bounceTime = 0.3f;   // 演出時間(秒)
	float m_badgeScale = 1.0f;

	// 家アイコンの色(被害ありで赤みがかる)
	Math::Color m_houseColor = { 1, 1, 1, 1 };

	static constexpr float kHouseSize = 64.0f;
	static constexpr float kBadgeSize = 28.0f;
};
