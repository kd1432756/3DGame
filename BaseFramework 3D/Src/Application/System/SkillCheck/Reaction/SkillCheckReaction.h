#pragma once
#include "../SkillCheckBase.h"

//==================================================
// 反応
// 待機中は灰色。ランダムな時間の後に赤く光った瞬間にスペースを押す
//  素早い = Perfect / 遅め = Good / 遅すぎ・光る前に押した(フライング) = Miss
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
	float m_reaction = 0.0f;	// 合図が出てからの経過時間
	float m_perfectT = 0.28f;
	float m_goodT = 0.55f;

	static constexpr float kY = -120.0f;
};