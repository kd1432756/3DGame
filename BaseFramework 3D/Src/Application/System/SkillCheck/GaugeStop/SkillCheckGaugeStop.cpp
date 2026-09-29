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
	auto& sprite = KdShaderManager::Instance().m_spriteShader;

	// 0〜1 → 画面X座標(中央原点)
	auto toX = [](float t) { return (t - 0.5f) * kBarWidth; };

	const float halfH = kBarHeight * 0.5f;

	// 背景バー
	Math::Color bg = { 0.1f, 0.1f, 0.1f, 0.8f };
	sprite.DrawBox(0, (int)kBarY, (int)(kBarWidth * 0.5f), (int)halfH, &bg, true);

	// Goodゾーン(緑)
	Math::Color green = { 0.2f, 0.8f, 0.3f, 1.0f };
	sprite.DrawBox((int)toX(m_zoneCenter), (int)kBarY,
		(int)(kBarWidth * m_goodWidth * 0.5f), (int)halfH, &green, true);

	// Perfectゾーン(黄)
	Math::Color yellow = { 1.0f, 0.9f, 0.2f, 1.0f };
	sprite.DrawBox((int)toX(m_zoneCenter), (int)kBarY,
		(int)(kBarWidth * m_perfWidth * 0.5f), (int)halfH, &yellow, true);

	// カーソル(準備時間中は半透明にして「まだ押せない」を伝える)
	Math::Color cursor = { 1.0f, 1.0f, 1.0f, IsReady() ? 1.0f : 0.4f };
	sprite.DrawBox((int)toX(m_cursor), (int)kBarY, 3, (int)halfH + 8, &cursor, true);
}