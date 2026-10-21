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
	const float keyX = halfW + kSideOffset;	// 左右のキーの位置

	// 下地(左右のキーも含めて囲む) + 残り時間
	DrawPanel(0, kY, keyX + 34.0f, 38.0f, a);
	DrawTimeBar(0, kY + 28.0f, halfW, m_perfectTime);

	// バー(白い枠 + 不透明の背景)
	DrawRect(0, kY, halfW + 2.0f, halfH + 2.0f, { 1.0f, 1.0f, 1.0f, 0.6f * a });
	DrawRect(0, kY, halfW, halfH, { 0.12f, 0.12f, 0.15f, a });

	// ゲージ(左端固定)。同じキーを連続で押した瞬間は赤
	const float gauge = (m_gauge > 1.0f) ? 1.0f : m_gauge;
	const float fillHalf = halfW * gauge;

	Math::Color fillColor = (m_wrongFlash > 0.0f)
		? Math::Color{ 1.0f, 0.2f, 0.2f, a }
	: Math::Color{ 1.0f, 0.6f, 0.2f, a };

	DrawRect(-halfW + fillHalf, kY, fillHalf, halfH, fillColor);

	// 区切り線
	for (int i = 1; i < m_required; ++i)
	{
		const float x = -halfW + kBarWidth * static_cast<float>(i) / static_cast<float>(m_required);
		DrawRect(x, kY, 1.0f, halfH, { 0.0f, 0.0f, 0.0f, 0.45f * a });
	}

	// 次に押す側のキーのリング(脈打たせて目を引く)
	const Math::Color haloColor = (m_wrongFlash > 0.0f)
		? Math::Color{ 1.0f, 0.25f, 0.25f, a }
	: Math::Color{ 1.0f, 0.9f, 0.3f, a };

	const float pulse = std::sin(m_elapsed * 12.0f) * 2.0f;
	DrawRing(m_expectLeft ? -keyX : keyX, kY, 30.0f + pulse, 3.0f, haloColor);

	// 左右のキー表示: 次に押す側だけ動かして明るく
	DrawKey(SkillKey::A, -keyX, kY,
		m_expectLeft ? SkillKeyAnim::Mash : SkillKeyAnim::Static,
		m_expectLeft ? 1.0f : 0.4f);
	DrawKey(SkillKey::D, keyX, kY,
		m_expectLeft ? SkillKeyAnim::Static : SkillKeyAnim::Mash,
		m_expectLeft ? 0.4f : 1.0f);
}