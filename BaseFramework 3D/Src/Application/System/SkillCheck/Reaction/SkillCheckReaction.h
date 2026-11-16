#pragma once
#include "../SkillCheckBase.h"

//==================================================
// 反応
//==================================================
class SkillCheckReaction : public SkillCheckBase
{
public:
	SkillCheckType GetType() const override { return SkillCheckType::Reaction; }

protected:
	void OnStart() override;

	void OnUpdate(float dt) override;

	void OnDraw() override;

private:
	static float Clamp(float v, float lo, float hi)
	{
		return (v < lo) ? lo : (v > hi) ? hi : v;
	}

	bool  m_signal = false;
	float m_timer = 0.0f;
	float m_wait = 1.0f;
	float m_reaction = 0.0f;		// 経過時間
	float m_perfectT = 0.28f;
	float m_goodT = 0.55f;

	static constexpr float kY = -120.0f;

	// 待機
	static constexpr float kWaitRadius = 54.0f;	// 外側の輪
	static constexpr float kWaitDotBase = 10.0f;	// 中心の点

	// 合図
	static constexpr float kSignalRadius = 48.0f;	// 赤い丸
	static constexpr float kBurstTime = 0.15f;	// 波紋が広がる時間
	static constexpr float kBurstGrow = 40.0f;	// 波紋が広がる量
	static constexpr float kLimitStartR = 76.0f;	// 残り時間リングの最初の半径
	static constexpr float kLimitEndR = 54.0f;	// 残り時間リングの最後の半径
};