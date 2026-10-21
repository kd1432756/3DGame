#include "SkillCheckGaugeStop.h"

void SkillCheckGaugeStop::OnStart()
{
	// 難易度でゾーンを狭く・カーソルを速く
	m_goodWidth = std::clamp(0.25f / m_difficulty, 0.10f, 0.40f);
	m_perfWidth = m_goodWidth * 0.35f;
	m_speed = 1.1f + 0.4f * m_difficulty;

	// ゾーン位置は毎回ランダム(端に寄りすぎないよう余白を確保)
	const float margin = m_goodWidth * 0.5f + 0.05f;
	m_zoneCenter = KdRandom::GetFloat(margin, 1.0f - margin);

	// カーソルはゾーンと反対側から始めて、開始直後に成功しにくくする
	m_cursor = (m_zoneCenter > 0.5f) ? 0.0f : 1.0f;
	m_dir = (m_cursor < 0.5f) ? 1.0f : -1.0f;
}

void SkillCheckGaugeStop::OnUpdate(float dt)
{
	// カーソルを往復させる
	m_cursor += m_dir * m_speed * dt;

	if (m_cursor >= 1.0f) { m_cursor = 1.0f; m_dir = -1.0f; }
	if (m_cursor <= 0.0f) { m_cursor = 0.0f; m_dir = 1.0f; }

	if (!IsActionPressed()) return;

	// 判定
	const float diff = std::abs(m_cursor - m_zoneCenter);

	if (diff <= m_perfWidth * 0.5f)	Finish(SkillResult::Perfect);
	else if (diff <= m_goodWidth * 0.5f) Finish(SkillResult::Good);
	else                                 Finish(SkillResult::Miss);
}

void SkillCheckGaugeStop::OnDraw()
{
	const float a = GetAlpha();
	const float halfW = kBarWidth * 0.5f;
	const float halfH = kBarHeight * 0.5f;

	// 0〜1 → 画面X座標(中央原点)
	auto toX = [](float t) { return (t - 0.5f) * kBarWidth; };

	// 下地 + 残り時間
	DrawPanel(0, kBarY - 17.0f, halfW + 30.0f, 51.0f, a);
	DrawTimeBar(0, kBarY + 28.0f, halfW);

	// バー(白い枠 + 不透明の背景)
	DrawRect(0, kBarY, halfW + 2.0f, halfH + 2.0f, { 1.0f, 1.0f, 1.0f, 0.6f * a });
	DrawRect(0, kBarY, halfW, halfH, { 0.12f, 0.12f, 0.15f, a });

	// Goodゾーン(緑)
	DrawRect(toX(m_zoneCenter), kBarY, kBarWidth * m_goodWidth * 0.5f, halfH,
		{ 0.2f, 0.8f, 0.3f, a });

	// Perfectゾーン(黄)
	DrawRect(toX(m_zoneCenter), kBarY, kBarWidth * m_perfWidth * 0.5f, halfH,
		{ 1.0f, 0.9f, 0.2f, a });

	// カーソルの軌跡(進行方向の後ろに薄い残像)
	for (int i = 3; i >= 1; --i)
	{
		const float trail = m_cursor - m_dir * 0.012f * i;
		if (trail < 0.0f || trail > 1.0f) continue;

		DrawRect(toX(trail), kBarY, 2.0f, halfH + 4.0f,
			{ 1.0f, 1.0f, 1.0f, 0.12f * (4 - i) * a });
	}

	// カーソル本体(線 + 上の三角マーカー)
	const float cx = toX(m_cursor);
	DrawRect(cx, kBarY, 2.0f, halfH + 6.0f, { 1.0f, 1.0f, 1.0f, a });
	DrawTriangle(
		cx, kBarY + halfH + 1.0f,
		cx - 6.0f, kBarY + halfH + 11.0f,
		cx + 6.0f, kBarY + halfH + 11.0f,
		{ 1.0f, 1.0f, 1.0f, a });

	DrawKey(SkillKey::MouseLeft, 0.0f, kBarY - 45.0f, SkillKeyAnim::Timing);
}