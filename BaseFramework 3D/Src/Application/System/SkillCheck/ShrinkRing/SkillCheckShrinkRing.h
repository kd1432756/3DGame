#pragma once
#include "../SkillCheckBase.h"

//==================================================
// 縮む輪
// 外側の枠が縮んで、中心の枠に重なる瞬間にスペースを押す
//  ほぼ一致 = Perfect / 近い = Good / 早すぎ・遅すぎ = Miss
// ※ 円を描く機能に依存しないよう、四角い枠で表現
//==================================================
class SkillCheckShrinkRing : public SkillCheckBase
{
public:
	SkillCheckType GetType() const override { return SkillCheckType::ShrinkRing; }

protected:
	void OnStart() override;
	
	void OnUpdate(float dt) override;
	
	void OnDraw() override;
	
private:

	// 四角い枠(中心 cx,cy / 半分のサイズ half / 線の太さの半分 thick)
	static void DrawFrame(float cx, float cy, float half, float thick, const Math::Color& c)
	{
		DrawRect(cx, cy + half, half + thick, thick, c);	// 上
		DrawRect(cx, cy - half, half + thick, thick, c);	// 下
		DrawRect(cx - half, cy, thick, half, c);	// 左
		DrawRect(cx + half, cy, thick, half, c);	// 右
	}

	static float Clamp(float v, float lo, float hi)
	{
		return (v < lo) ? lo : (v > hi) ? hi : v;
	}

	float m_timer = 0.0f;
	float m_shrinkTime = 1.2f;	// 重なるまでの秒数
	float m_perfWin = 0.07f;
	float m_goodWin = 0.18f;

	static constexpr float kY = -120.0f;
	static constexpr float kStartHalf = 64.0f;
	static constexpr float kTargetHalf = 24.0f;
};