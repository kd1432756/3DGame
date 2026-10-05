#include "SkillCheckShrinkRing.h"

void SkillCheckShrinkRing::OnStart()
{
	// 難易度で縮むのが速く、判定が厳しくなる
	m_shrinkTime = Clamp(1.2f / m_difficulty, 0.7f, 1.6f);
	m_perfWin = Clamp(0.07f / m_difficulty, 0.04f, 0.10f);
	m_goodWin = Clamp(0.18f / m_difficulty, 0.10f, 0.25f);

	m_timeLimit = m_shrinkTime + m_goodWin + 0.2f;
	m_timer = 0.0f;
}

void SkillCheckShrinkRing::OnUpdate(float dt)
{
	m_timer += dt;

	if (IsActionPressed())
	{
		// 「重なる瞬間」とのズレ(秒)で判定
		float diff = m_shrinkTime - m_timer;
		if (diff < 0.0f) diff = -diff;

		if (diff <= m_perfWin) Finish(SkillResult::Perfect);
		else if (diff <= m_goodWin) Finish(SkillResult::Good);
		else                        Finish(SkillResult::Miss);
		return;
	}

	// 重なってから猶予を過ぎても押さない → Miss
	if (m_timer > m_shrinkTime + m_goodWin)
	{
		Finish(SkillResult::Miss);
	}
}

void SkillCheckShrinkRing::OnDraw()
{
	const float a = GetAlpha();

	// 外枠の大きさ(重なる瞬間に kTargetHalf、その後も縮み続ける)
	float t = m_timer / m_shrinkTime;
	float half = kTargetHalf + (kStartHalf - kTargetHalf) * (1.0f - t);
	if (half < 2.0f) half = 2.0f;

	// 目標の枠(緑)と中心点(黄)
	DrawFrame(0, kY, kTargetHalf, 3, { 0.2f, 0.8f, 0.3f, a });
	DrawRect(0, kY, 3, 3, { 1.0f, 0.9f, 0.2f, a });

	// 縮む枠(白)
	DrawFrame(0, kY, half, 3, { 1.0f, 1.0f, 1.0f, a });

	DrawKey(SkillKey::Space, 0.0f, kY - kStartHalf - 30.0f, SkillKeyAnim::Timing);
}
