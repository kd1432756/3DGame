#pragma once
#include "../SkillCheckBase.h"

//==================================================
// 縮む輪
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

	// 枠
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
	float m_shrinkTime = 1.2f;		// 重なるまでの秒数
	float m_perfWin = 0.07f;
	float m_goodWin = 0.18f;

	static constexpr float kY = -120.0f;
	static constexpr float kStartHalf = 64.0f;
	static constexpr float kTargetHalf = 24.0f;
};