#include "SkillCheckAltMash.h"

void SkillCheckAltMash::OnStart()
{
	m_required = 8 + static_cast<int>(2.0f * m_difficulty);	// 難易度1.0で10回
	m_perfectTime = m_required * 0.15f;
	m_timeLimit = m_required * 0.25f;

	m_gauge = 0.0f;
	m_inputTime = 0.0f;
	m_wrongFlash = 0.0f;
	m_expectLeft = (KdRandom::GetInt(0, 1) == 0);	// 最初に押す側はランダム
}

void SkillCheckAltMash::OnUpdate(float dt)
{
	m_inputTime += dt;

	// 放置するとゲージが減る
	m_gauge -= kDecay * dt;
	if (m_gauge < 0.0f) m_gauge = 0.0f;

	if (m_wrongFlash > 0.0f) m_wrongFlash -= dt;

	const bool pressA = IsKeyPressed('A');
	const bool pressD = IsKeyPressed('D');

	if (pressA || pressD)
	{
		const bool correct = m_expectLeft ? pressA : pressD;

		if (correct)
		{
			m_gauge += 1.0f / m_required;
			m_expectLeft = !m_expectLeft;	// 次は反対側
		}
		else
		{
			m_wrongFlash = 0.15f;	// 同じキーの連続 → 無効(演出のみ)
		}
	}

	if (m_gauge >= 1.0f)
	{
		Finish(m_inputTime <= m_perfectTime ? SkillResult::Perfect : SkillResult::Good);
	}
}

void SkillCheckAltMash::OnDraw()
{
	const float a = GetAlpha();
	const float halfW = kBarWidth * 0.5f;
	const float halfH = kBarHeight * 0.5f;

	// ゲージ背景
	DrawRect(0, kY, halfW, halfH, { 0.1f, 0.1f, 0.1f, 0.8f * a });

	// ゲージ(左端固定)。同じキー連打の瞬間は赤
	const float gauge = (m_gauge > 1.0f) ? 1.0f : m_gauge;
	const float fillHalf = halfW * gauge;

	Math::Color fillColor = (m_wrongFlash > 0.0f)
		? Math::Color{ 1.0f, 0.2f, 0.2f, a }
	: Math::Color{ 1.0f, 0.6f, 0.2f, a };

	DrawRect(-halfW + fillHalf, kY, fillHalf, halfH, fillColor);

	// 左右のキー表示: 次に押す側だけ動かして明るく

	DrawKey(SkillKey::A, -halfW - kSideOffset, kY,
		m_expectLeft ? SkillKeyAnim::Mash : SkillKeyAnim::Static,
		m_expectLeft ? 1.0f : 0.4f);
	DrawKey(SkillKey::D, halfW + kSideOffset, kY,
		m_expectLeft ? SkillKeyAnim::Static : SkillKeyAnim::Mash,
		m_expectLeft ? 0.4f : 1.0f);

}