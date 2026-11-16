#include "SkillCheckReaction.h"

void SkillCheckReaction::OnStart()
{
	m_wait = KdRandom::GetFloat(0.6f, 1.8f);	 // 合図が出るまでの待ち時間
	m_perfectT = Clamp(0.28f / m_difficulty, 0.15f, 0.35f);
	m_goodT = Clamp(0.55f / m_difficulty, 0.30f, 0.70f);

	m_timeLimit = m_wait + m_goodT + 0.3f;

	m_timer = 0.0f;
	m_reaction = 0.0f;
	m_signal = false;
}

void SkillCheckReaction::OnUpdate(float dt)
{
	m_timer += dt;

	if (!m_signal)
	{
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

	m_reaction += dt;

	if (IsActionPressed())
	{
		if (m_reaction <= m_perfectT) Finish(SkillResult::Perfect);
		else if (m_reaction <= m_goodT) Finish(SkillResult::Good);
		else Finish(SkillResult::Miss);
		return;
	}

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
		// ---- 待機中 ----
		const float breath = std::sin(m_elapsed * 4.0f);

		// 背景
		DrawCircle(0, kY, kWaitRadius, { 0.05f, 0.07f, 0.12f, 0.6f * a });

		// 外側の輪
		DrawRing(0, kY, kWaitRadius, 3.0f, { 0.45f, 0.60f, 0.90f, a });

		// 点
		DrawCircle(0, kY, kWaitDotBase + breath * 2.0f, { 0.45f, 0.60f, 0.90f, a });
	}
	else
	{
		// ---- 合図 ----
		DrawCircle(0, kY, kSignalRadius, { 1.0f, 0.15f, 0.15f, 1.0f });
		DrawRing(0, kY, kSignalRadius, 3.0f, { 1.0f, 1.0f, 1.0f, 1.0f });

		// 「!」
		DrawRect(0, kY + 9.0f, 4.0f, 12.0f, { 1.0f, 1.0f, 1.0f, 1.0f });
		DrawRect(0, kY - 14.0f, 4.0f, 4.0f, { 1.0f, 1.0f, 1.0f, 1.0f });

		// 波紋
		if (m_reaction < kBurstTime)
		{
			const float p = m_reaction / kBurstTime;
			DrawRing(0, kY, kSignalRadius + kBurstGrow * p, 3.0f, { 1.0f, 1.0f, 1.0f, 1.0f - p });
		}

		// 残り時間の輪
		float t = m_reaction / m_goodT;
		if (t > 1.0f) t = 1.0f;

		const float ringR = kLimitStartR + (kLimitEndR - kLimitStartR) * t;
		DrawRing(0, kY, ringR, 3.0f, { 1.0f, 0.90f, 0.30f, 1.0f });
	}

	DrawKey(kActionKey, 0.0f, kY - 110.0f,
		m_signal ? SkillKeyAnim::Timing : SkillKeyAnim::Static);
}
