#include "SkillCheckArrow.h"

void SkillCheckArrow::OnStart()
{
	// 難易度1.0で4個。0.8なら3個、1.5以上なら5個
	int count = 3 + static_cast<int>(m_difficulty);
	if (count < 3) count = 3;
	if (count > 5) count = 5;

	m_seq.clear();
	for (int i = 0; i < count; ++i)
	{
		m_seq.push_back(static_cast<Dir>(KdRandom::GetInt(0, 3)));
	}

	m_index = 0;
	m_inputTime = 0.0f;
	m_perfectTime = count * 0.55f;
	m_timeLimit = count * 1.0f + 0.5f;
}

void SkillCheckArrow::OnUpdate(float dt)
{
	m_inputTime += dt;

	int pressed = -1;
	if (IsKeyPressed('W')) pressed = Up;
	else if (IsKeyPressed('S')) pressed = Down;
	else if (IsKeyPressed('A')) pressed = Left;
	else if (IsKeyPressed('D')) pressed = Right;

	if (pressed < 0) return;

	if (pressed != m_seq[m_index])
	{
		Finish(SkillResult::Miss);
		return;
	}

	++m_index;

	if (m_index >= static_cast<int>(m_seq.size()))
	{
		Finish(m_inputTime <= m_perfectTime ? SkillResult::Perfect : SkillResult::Good);
	}
}

void SkillCheckArrow::OnDraw()
{
	const float a = GetAlpha();
	const int   n = static_cast<int>(m_seq.size());

	const float panelHalfW = n * kSpacing * 0.5f + 30.0f;

	// 背景
	DrawPanel(0, kY - 27.0f, panelHalfW, 73.0f, a);
	DrawTimeBar(0, kY + 40.0f, panelHalfW - 12.0f, m_perfectTime);

	const float pulse = 0.5f + 0.5f * std::sin(m_elapsed * 10.0f);	// 0〜1

	for (int i = 0; i < n; ++i)
	{
		const float x = (i - (n - 1) * 0.5f) * kSpacing;

		const bool done = (i < m_index);
		const bool current = (i == m_index);

		Math::Color frame, arrowColor;
		if (done) { frame = { 0.2f, 0.8f, 0.3f, a };  arrowColor = { 0.2f, 0.8f, 0.3f, 0.5f * a }; }
		else if (current) { frame = { 1.0f, 1.0f, 1.0f, a };  arrowColor = { 1.0f, 1.0f, 1.0f, a }; }
		else { frame = { 0.5f, 0.5f, 0.5f, a };  arrowColor = { 0.55f, 0.55f, 0.55f, a }; }

		// 枠
		const float outer = kSlotHalf + 3.0f + (current ? pulse * 3.0f : 0.0f);
		DrawRect(x, kY, outer, outer, frame);
		DrawRect(x, kY, kSlotHalf, kSlotHalf, { 0.08f, 0.08f, 0.10f, 0.95f * a });

		// 矢印
		int fx = 0, fy = 0;
		switch (m_seq[i])
		{
		case Up:    fy = 1; break;
		case Down:  fy = -1; break;
		case Left:  fx = -1; break;
		case Right: fx = 1; break;
		}
		DrawArrowShape(x, kY, fx, fy, 18.0f, arrowColor);

		// キー
		DrawKey(KeyOf(m_seq[i]), x, kY - 75.0f,
			current ? SkillKeyAnim::Timing : SkillKeyAnim::Static,
			done ? 0.4f : 1.0f);
	}
}