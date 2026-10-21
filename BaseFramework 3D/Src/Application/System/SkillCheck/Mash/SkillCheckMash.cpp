#include "SkillCheckMash.h"

void SkillCheckMash::OnStart()
{
	m_required = 8 + static_cast<int>(3.0f * m_difficulty);		// 必要な打鍵数(難易度1.0で11回)
	m_perfectTime = m_required * 0.15f;							// この時間内ならPerfect
	m_timeLimit = m_required * 0.25f;							// これを超えたらMiss
	m_gauge = 0.0f;
	m_inputTime = 0.0f;
}

void SkillCheckMash::OnUpdate(float dt)
{
	m_inputTime += dt;

	// 放置するとゲージが減る
	m_gauge -= kDecay * dt;
	if (m_gauge < 0.0f) m_gauge = 0.0f;

	if (IsActionPressed())
	{
		m_gauge += 1.0f / m_required;
		m_pushFlash = 0.08f;	// 押した瞬間の演出用
	}

	if (m_pushFlash > 0.0f) m_pushFlash -= dt;

	if (m_gauge >= 1.0f)
	{
		Finish(m_inputTime <= m_perfectTime ? SkillResult::Perfect : SkillResult::Good);
	}
}

void SkillCheckMash::OnDraw()
{
	const float a = GetAlpha();
	const float halfW = kBarWidth * 0.5f;
	const float halfH = kBarHeight * 0.5f;

	// 下地 + 残り時間(Perfectが狙える間は黄色)
	DrawPanel(0, kBarY - 17.0f, halfW + 30.0f, 51.0f, a);
	DrawTimeBar(0, kBarY + 28.0f, halfW, m_perfectTime);

	// バー(白い枠 + 不透明の背景)
	DrawRect(0, kBarY, halfW + 2.0f, halfH + 2.0f, { 1.0f, 1.0f, 1.0f, 0.6f * a });
	DrawRect(0, kBarY, halfW, halfH, { 0.12f, 0.12f, 0.15f, a });

	// ゲージ(左端固定で右へ伸びる)
	const float gauge = (m_gauge > 1.0f) ? 1.0f : m_gauge;
	const float fillHalf = halfW * gauge;

	// 押した瞬間は明るくする
	Math::Color fillColor = (m_pushFlash > 0.0f)
		? Math::Color{ 1.0f, 0.95f, 0.5f, a }
	: Math::Color{ 1.0f, 0.6f, 0.2f, a };

	DrawRect(-halfW + fillHalf, kBarY, fillHalf, halfH, fillColor);

	// 区切り線(1回押すごとに1目盛り進む)
	for (int i = 1; i < m_required; ++i)
	{
		const float x = -halfW + kBarWidth * static_cast<float>(i) / static_cast<float>(m_required);
		DrawRect(x, kBarY, 1.0f, halfH, { 0.0f, 0.0f, 0.0f, 0.45f * a });
	}

	DrawKey(SkillKey::MouseLeft, 0.0f, kBarY - 45.0f, SkillKeyAnim::Mash);
}
