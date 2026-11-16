#include "SkillCheckGaugeStop.h"

void SkillCheckGaugeStop::OnStart()
{
	// 難易度でゾーンを狭く・カーソルを速く
	m_goodWidth = std::clamp(0.25f / m_difficulty, 0.10f, 0.40f);
	m_perfWidth = m_goodWidth * 0.35f;
	m_speed = 1.1f + 0.4f * m_difficulty;

	// ランダムかつ端に寄りすぎないように
	const float margin = m_goodWidth * 0.5f + 0.05f;
	m_zoneCenter = KdRandom::GetFloat(margin, 1.0f - margin);

	// ゾーンの反対からスタート
	m_cursor = (m_zoneCenter > 0.5f) ? 0.0f : 1.0f;
	m_dir = (m_cursor < 0.5f) ? 1.0f : -1.0f;
}

void SkillCheckGaugeStop::OnUpdate(float dt)
{
	// 往復
	m_cursor += m_dir * m_speed * dt;

	if (m_cursor >= 1.0f) { m_cursor = 1.0f; m_dir = -1.0f; }
	if (m_cursor <= 0.0f) { m_cursor = 0.0f; m_dir = 1.0f; }

	if (!IsActionPressed()) return;

	const float diff = std::abs(m_cursor - m_zoneCenter);

	if (diff <= m_perfWidth * 0.5f)	Finish(SkillResult::Perfect);
	else if (diff <= m_goodWidth * 0.5f) Finish(SkillResult::Good);
	else Finish(SkillResult::Miss);
}

void SkillCheckGaugeStop::OnDraw()
{
	const float a = GetAlpha();
	const float halfW = kBarWidth * 0.5f;
	const float halfH = kBarHeight * 0.5f;

	auto toX = [](float t) { return (t - 0.5f) * kBarWidth; };

	// 背景
	DrawPanel(0, kBarY - 17.0f, halfW + 30.0f, 51.0f, a);
	DrawTimeBar(0, kBarY + 28.0f, halfW);

	// バー
	DrawRect(0, kBarY, halfW + 2.0f, halfH + 2.0f, { 1.0f, 1.0f, 1.0f, 0.6f * a });
	DrawRect(0, kBarY, halfW, halfH, { 0.12f, 0.12f, 0.15f, a });

	// Goodゾーン
	DrawRect(toX(m_zoneCenter), kBarY, kBarWidth * m_goodWidth * 0.5f, halfH,
		{ 0.2f, 0.8f, 0.3f, a });

	// Perfectゾーン
	DrawRect(toX(m_zoneCenter), kBarY, kBarWidth * m_perfWidth * 0.5f, halfH,
		{ 1.0f, 0.9f, 0.2f, a });

	// カーソルの残像
	for (int i = 3; i >= 1; --i)
	{
		const float trail = m_cursor - m_dir * 0.012f * i;
		if (trail < 0.0f || trail > 1.0f) continue;

		DrawRect(toX(trail), kBarY, 2.0f, halfH + 4.0f,
			{ 1.0f, 1.0f, 1.0f, 0.12f * (4 - i) * a });
	}

	// カーソル
	const float cx = toX(m_cursor);
	DrawRect(cx, kBarY, 2.0f, halfH + 6.0f, { 1.0f, 1.0f, 1.0f, a });
	DrawTriangle(
		cx, kBarY + halfH + 1.0f,
		cx - 6.0f, kBarY + halfH + 11.0f,
		cx + 6.0f, kBarY + halfH + 11.0f,
		{ 1.0f, 1.0f, 1.0f, a });

	DrawKey(SkillKey::MouseLeft, 0.0f, kBarY - 45.0f, SkillKeyAnim::Timing);
}