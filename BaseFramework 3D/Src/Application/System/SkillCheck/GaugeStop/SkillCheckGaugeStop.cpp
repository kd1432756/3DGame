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

	// 0〜1 → 画面X座標(中央原点)
	auto toX = [](float t) { return (t - 0.5f) * kBarWidth; };

	const float halfH = kBarHeight * 0.5f;

	// 背景バー
	DrawRect(0, kBarY, kBarWidth * 0.5f, halfH, { 0.1f, 0.1f, 0.1f, 0.8f * a });

	// Goodゾーン(緑)
	DrawRect(toX(m_zoneCenter), kBarY, kBarWidth * m_goodWidth * 0.5f, halfH,
		{ 0.2f, 0.8f, 0.3f, a });

	// Perfectゾーン(黄)
	DrawRect(toX(m_zoneCenter), kBarY, kBarWidth * m_perfWidth * 0.5f, halfH,
		{ 1.0f, 0.9f, 0.2f, a });

	// カーソル
	DrawRect(toX(m_cursor), kBarY, 3, halfH + 8, { 1.0f, 1.0f, 1.0f, a });

	DrawKey(SkillKey::MouseLeft, 0.0f, kBarY - 45.0f, SkillKeyAnim::Timing);
}