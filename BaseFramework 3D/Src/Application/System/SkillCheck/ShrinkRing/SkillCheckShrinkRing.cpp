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

	// 判定の帯の幅(px) = 縮む速さ(px/秒) × 判定の猶予(秒)
	const float speed = (kStartHalf - kTargetHalf) / m_shrinkTime;
	const float goodBand = speed * m_goodWin;
	const float perfBand = speed * m_perfWin;

	const Math::Color green = { 0.2f, 0.8f, 0.3f, 1.0f };
	const Math::Color yellow = { 1.0f, 0.9f, 0.2f, 1.0f };
	const Math::Color dark = { 0.06f, 0.07f, 0.10f, 1.0f };

	// 下地
	DrawCircle(0, kY, kStartHalf + 12.0f, { 0.06f, 0.07f, 0.10f, 0.92f * a });

	// 判定の帯(外側から内側へ重ねる: 緑 → 黄 → 緑 → 暗い穴)
	DrawCircle(0, kY, kTargetHalf + goodBand, green);
	DrawCircle(0, kY, kTargetHalf + perfBand, yellow);
	DrawCircle(0, kY, kTargetHalf - perfBand, green);
	DrawCircle(0, kY, kTargetHalf - goodBand, dark);

	// 縮む輪(重なる瞬間に kTargetHalf、その後も縮み続ける)
	float t = m_timer / m_shrinkTime;
	float r = kTargetHalf + (kStartHalf - kTargetHalf) * (1.0f - t);
	if (r < 2.0f) r = 2.0f;

	DrawRing(0, kY, r, 4.0f, { 1.0f, 1.0f, 1.0f, a });

	DrawKey(SkillKey::MouseLeft, 0.0f, kY - kStartHalf - 40.0f, SkillKeyAnim::Timing);
}
