#include "SkillCheckHoldRelease.h"

void SkillCheckHoldRelease::OnStart()
{
	// 難易度でゲージが速く、ゾーンが狭くなる
	m_fillTime = Clamp(0.7f / m_difficulty, 0.8f, 2.0f);
	m_perfW = Clamp(0.08f / m_difficulty, 0.04f, 0.12f);
	m_goodW = Clamp(0.25f / m_difficulty, 0.12f, 0.35f);

	m_timeLimit = 4.0f;		// 押し始めるまでの待ち時間も含む
	m_holding = false;
	m_value = 0.0f;
}

void SkillCheckHoldRelease::OnUpdate(float dt)
{
	// まだ押していない → 押し始めを待つ
	if (!m_holding)
	{
		if (IsActionPressed()) m_holding = true;
		return;
	}

	// 押し続けている間ゲージが溜まる
	if (IsKeyDown(VK_LBUTTON))
	{
		m_value += dt / m_fillTime;

		// 満タンまで押し続けた(溜めすぎ) → Miss
		if (m_value >= 1.0f)
		{
			m_value = 1.0f;
			Finish(SkillResult::Miss);
		}
		return;
	}

	// 離した瞬間に判定
	if (m_value >= 1.0f - m_perfW) Finish(SkillResult::Perfect);
	else if (m_value >= 1.0f - m_goodW) Finish(SkillResult::Good);
	else                                Finish(SkillResult::Miss);
}

void SkillCheckHoldRelease::OnDraw()
{
	const float a = GetAlpha();
	const float halfW = kBarWidth * 0.5f;
	const float halfH = kBarHeight * 0.5f;

	auto toX = [&](float t) { return (t - 0.5f) * kBarWidth; };

	// 背景
	DrawRect(0, kBarY, halfW, halfH, { 0.1f, 0.1f, 0.1f, 0.8f * a });

	// Goodゾーン(右端から)
	DrawRect(toX(1.0f - m_goodW * 0.5f), kBarY,
		kBarWidth * m_goodW * 0.5f, halfH, { 0.2f, 0.8f, 0.3f, a });

	// Perfectゾーン(右端から)
	DrawRect(toX(1.0f - m_perfW * 0.5f), kBarY,
		kBarWidth * m_perfW * 0.5f, halfH, { 1.0f, 0.9f, 0.2f, a });

	// 溜まった量(半透明の青を重ねる)
	const float v = Clamp(m_value, 0.0f, 1.0f);
	DrawRect(-halfW + halfW * v, kBarY, halfW * v, halfH, { 0.3f, 0.6f, 1.0f, 0.55f * a });

	// 先端の線
	DrawRect(toX(v), kBarY, 3, halfH + 8, { 1.0f, 1.0f, 1.0f, a });

	DrawKey(SkillKey::MouseLeft, 0.0f, kBarY - 45.0f, SkillKeyAnim::Hold);
}
