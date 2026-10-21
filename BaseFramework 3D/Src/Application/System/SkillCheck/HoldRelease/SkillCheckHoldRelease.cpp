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

	auto toX = [](float t) { return (t - 0.5f) * kBarWidth; };

	// 下地 + 残り時間
	DrawPanel(0, kBarY - 17.0f, halfW + 30.0f, 51.0f, a);
	DrawTimeBar(0, kBarY + 28.0f, halfW);

	// バー(白い枠 + 不透明の背景)
	DrawRect(0, kBarY, halfW + 2.0f, halfH + 2.0f, { 1.0f, 1.0f, 1.0f, 0.6f * a });
	DrawRect(0, kBarY, halfW, halfH, { 0.12f, 0.12f, 0.15f, a });

	// Goodゾーン(右端から)
	DrawRect(toX(1.0f - m_goodW * 0.5f), kBarY,
		kBarWidth * m_goodW * 0.5f, halfH, { 0.2f, 0.8f, 0.3f, a });

	// Perfectゾーン(右端から)
	DrawRect(toX(1.0f - m_perfW * 0.5f), kBarY,
		kBarWidth * m_perfW * 0.5f, halfH, { 1.0f, 0.9f, 0.2f, a });

	// 溜まった量(半透明。ゾーンに入ると色が変わる)
	Math::Color fillColor;
	if (m_value >= 0.97f)          fillColor = { 1.0f, 0.3f, 0.3f, 0.7f * a };	// もう離せ(溜めすぎ寸前)
	else if (m_value >= 1.0f - m_perfW) fillColor = { 1.0f, 0.9f, 0.2f, 0.7f * a };	// Perfect
	else if (m_value >= 1.0f - m_goodW) fillColor = { 0.3f, 0.9f, 0.4f, 0.7f * a };	// Good
	else                                fillColor = { 0.3f, 0.6f, 1.0f, 0.6f * a };	// 溜め中

	const float v = Clamp(m_value, 0.0f, 1.0f);
	DrawRect(-halfW + halfW * v, kBarY, halfW * v, halfH, fillColor);

	// 先端(線 + 上の三角マーカー)
	const float cx = toX(v);
	DrawRect(cx, kBarY, 2.0f, halfH + 6.0f, { 1.0f, 1.0f, 1.0f, a });
	DrawTriangle(
		cx, kBarY + halfH + 1.0f,
		cx - 6.0f, kBarY + halfH + 11.0f,
		cx + 6.0f, kBarY + halfH + 11.0f,
		{ 1.0f, 1.0f, 1.0f, a });

	DrawKey(SkillKey::MouseLeft, 0.0f, kBarY - 45.0f, SkillKeyAnim::Hold);
}
