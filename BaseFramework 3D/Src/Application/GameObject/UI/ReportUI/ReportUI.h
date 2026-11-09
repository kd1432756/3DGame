#pragma once

#include "../PlayerUIBase/PlayerUIBase.h"

class ReportUI : public PlayerUIBase
{
public:
	void Init() override;
	void Update() override;
	void DrawSprite() override;

private:
	KdTexture m_tex;
	KdTexture m_texKeyR;

	float m_remaining = 0.0f;
	float m_ratio = 0.0f;       // 1=CT開始直後 → 0=使用可
	bool  m_prevCooling = false;
	float m_flashTimer = 0.0f;  // 使用可能になった瞬間の演出

	// レイアウト
	static constexpr int   kPosX = 540;
	static constexpr int   kPosY = -260;
	static constexpr int   kIconSize = 96;      // 画像の直径（円の大きさ）
	static constexpr int   kRingWidth = 6;      // 外枠の太さ(px)
	static constexpr float kFlashTime = 0.5f;

	static constexpr float kKeySize = 28.0f;
};
