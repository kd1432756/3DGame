#include "SkillCheckArrow.h"

void SkillCheckArrow::OnStart()
{
	// 難易度1.0で3個。0.8なら2個、1.5以上なら4個
	int count = 2 + static_cast<int>(m_difficulty);
	if (count < 2) count = 2;
	if (count > 4) count = 4;

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

	// 押されたキーを方向に変換(同フレームに複数なら最初の1つ)
	int pressed = -1;
	if (IsKeyPressed('W')) pressed = Up;
	else if (IsKeyPressed('S')) pressed = Down;
	else if (IsKeyPressed('A')) pressed = Left;
	else if (IsKeyPressed('D')) pressed = Right;

	if (pressed < 0) return;

	// 間違い → Miss
	if (pressed != m_seq[m_index])
	{
		Finish(SkillResult::Miss);
		return;
	}

	// 正解 → 次へ
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

	for (int i = 0; i < n; ++i)
	{
		const float kx = (i - (n - 1) * 0.5f) * kSpacing;
		DrawKey(KeyOf(m_seq[i]), kx, kY - 75.0f,
			(i == m_index) ? SkillKeyAnim::Timing : SkillKeyAnim::Static,
			(i < m_index) ? 0.4f : 1.0f);
	}

	for (int i = 0; i < n; ++i)
	{
		const float x = (i - (n - 1) * 0.5f) * kSpacing;

		// 枠(入力済み=緑 / 今の入力=白 / これから=灰色)
		Math::Color frame;
		if (i < m_index)  frame = { 0.2f, 0.8f, 0.3f, a };
		else if (i == m_index) frame = { 1.0f, 1.0f, 1.0f, a };
		else                   frame = { 0.5f, 0.5f, 0.5f, a };

		DrawRect(x, kY, kSlotHalf + 3, kSlotHalf + 3, frame);
		DrawRect(x, kY, kSlotHalf, kSlotHalf, { 0.1f, 0.1f, 0.1f, 0.9f * a });

		// 方向マーカー(中心から方向へずらした四角)
		float dx = 0.0f, dy = 0.0f;
		switch (m_seq[i])
		{
		case Up:    dy = kMarkerOffset; break;	// 画面は上がプラス
		case Down:  dy = -kMarkerOffset; break;
		case Left:  dx = -kMarkerOffset; break;
		case Right: dx = kMarkerOffset; break;
		}

		// 中心の点(基準) + 方向マーカー(大きめ)
		DrawRect(x, kY, 3, 3, { 0.6f, 0.6f, 0.6f, a });
		DrawRect(x + dx, kY + dy, 8, 8, frame);
	}
}