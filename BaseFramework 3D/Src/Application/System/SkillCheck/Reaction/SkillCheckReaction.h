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
	void OnStart() override
	{
		m_wait = KdRandom::GetFloat(0.6f, 1.8f);	// 合図が出るまでの待ち時間
		m_perfectT = Clamp(0.28f / m_difficulty, 0.15f, 0.35f);
		m_goodT = Clamp(0.55f / m_difficulty, 0.30f, 0.70f);

		m_timeLimit = m_wait + m_goodT + 0.3f;	// 念のための保険(通常は下の判定で先に終わる)

		m_timer = 0.0f;
		m_reaction = 0.0f;
		m_signal = false;
	}

	void OnUpdate(float dt) override
	{
		m_timer += dt;

		// 合図前
		if (!m_signal)
		{
			// フライング → Miss
			if (IsActionPressed())
			{
				Finish(SkillResult::Miss);
				return;
			}

			if (m_timer >= m_wait)
			{
				m_signal = true;
				m_reaction = 0.0f;
			}
			return;
		}

		// 合図後
		m_reaction += dt;

		if (IsActionPressed())
		{
			if (m_reaction <= m_perfectT) Finish(SkillResult::Perfect);
			else if (m_reaction <= m_goodT)    Finish(SkillResult::Good);
			else                               Finish(SkillResult::Miss);
			return;
		}

		// 押さないまま猶予を過ぎた → Miss
		if (m_reaction > m_goodT)
		{
			Finish(SkillResult::Miss);
		}
	}

	void OnDraw() override
	{
		const float a = GetAlpha();

		if (!m_signal)
		{
			// 待機中: 小さな灰色の四角
			DrawRect(0, kY, 24, 24, { 0.5f, 0.5f, 0.5f, a });
		}
		else
		{
			// 合図: 大きな赤い四角 + 白い中心
			DrawRect(0, kY, 44, 44, { 1.0f, 0.15f, 0.15f, 1.0f });
			DrawRect(0, kY, 16, 16, { 1.0f, 1.0f, 1.0f, 1.0f });
		}
	}

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