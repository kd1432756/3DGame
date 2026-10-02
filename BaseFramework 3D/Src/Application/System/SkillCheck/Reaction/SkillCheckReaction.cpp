#include "SkillCheckReaction.h"

void SkillCheckReaction::OnStart()
{
	m_wait = KdRandom::GetFloat(0.6f, 1.8f);	// 合図が出るまでの待ち時間
	m_perfectT = Clamp(0.28f / m_difficulty, 0.15f, 0.35f);
	m_goodT = Clamp(0.55f / m_difficulty, 0.30f, 0.70f);

	m_timeLimit = m_wait + m_goodT + 0.3f;	// 念のための保険(通常は下の判定で先に終わる)

	m_timer = 0.0f;
	m_reaction = 0.0f;
	m_signal = false;
}

void SkillCheckReaction::OnUpdate(float dt)
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

void SkillCheckReaction::OnDraw()
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
