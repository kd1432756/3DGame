#include "SkillCheckReaction.h"

namespace
{
	// 待機中
	constexpr float kWaitRadius = 54.0f;	// 外側の輪
	constexpr float kWaitDotBase = 10.0f;	// 中心の点(この半径で呼吸する)

	// 合図
	constexpr float kSignalRadius = 48.0f;	// 赤い丸
	constexpr float kBurstTime = 0.15f;	// 波紋が広がる時間
	constexpr float kBurstGrow = 40.0f;	// 波紋が広がる量
	constexpr float kLimitStartR = 76.0f;	// 残り時間リングの最初の半径
	constexpr float kLimitEndR = 54.0f;	// 残り時間リングの最後の半径(赤い丸のすぐ外)
}

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
		// ---- 待機中: 青くて静かな丸。ゆっくり呼吸して「待て」を伝える ----
		const float breath = std::sin(m_elapsed * 4.0f);	// -1〜1

		// 背景の暗い丸
		DrawCircle(0, kY, kWaitRadius, { 0.05f, 0.07f, 0.12f, 0.6f * a });

		// 外側の輪
		DrawRing(0, kY, kWaitRadius, 3.0f, { 0.45f, 0.60f, 0.90f, a });

		// 中心の点
		DrawCircle(0, kY, kWaitDotBase + breath * 2.0f, { 0.45f, 0.60f, 0.90f, a });
	}
	else
	{
		// ---- 合図: 赤い丸 + 白い「!」 ----
		DrawCircle(0, kY, kSignalRadius, { 1.0f, 0.15f, 0.15f, 1.0f });
		DrawRing(0, kY, kSignalRadius, 3.0f, { 1.0f, 1.0f, 1.0f, 1.0f });

		// 「!」(縦棒 + 点)
		DrawRect(0, kY + 9.0f, 4.0f, 12.0f, { 1.0f, 1.0f, 1.0f, 1.0f });
		DrawRect(0, kY - 14.0f, 4.0f, 4.0f, { 1.0f, 1.0f, 1.0f, 1.0f });

		// 出た瞬間の波紋(広がりながら消える)
		if (m_reaction < kBurstTime)
		{
			const float p = m_reaction / kBurstTime;	// 0 → 1
			DrawRing(0, kY, kSignalRadius + kBurstGrow * p, 3.0f, { 1.0f, 1.0f, 1.0f, 1.0f - p });
		}

		// 残り時間リング(縮んで赤い丸に付いたら Miss)
		float t = m_reaction / m_goodT;
		if (t > 1.0f) t = 1.0f;

		const float ringR = kLimitStartR + (kLimitEndR - kLimitStartR) * t;
		DrawRing(0, kY, ringR, 3.0f, { 1.0f, 0.90f, 0.30f, 1.0f });
	}

	// キー表示(リングと重ならないよう少し下げる)
	DrawKey(kActionKey, 0.0f, kY - 110.0f,
		m_signal ? SkillKeyAnim::Timing : SkillKeyAnim::Static);
}
