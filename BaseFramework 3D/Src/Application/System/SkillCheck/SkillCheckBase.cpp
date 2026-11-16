#include "SkillCheckBase.h"

void SkillCheckBase::Start(float difficulty)
{
	m_result = SkillResult::None;
	m_difficulty = (difficulty < 0.1f) ? 0.1f : difficulty;
	m_elapsed = 0.0f;

	// 開始時に残っている入力を拾わないよう、一度読んで捨てる
	PollKeys();
	ClearPressed();

	OnStart();
}

void SkillCheckBase::Update()
{
	if (IsFinished()) return;

	PollKeys();		// キー状態を読む

	m_elapsed += 1.0f / 60.0f;

	if (!IsReady())
	{
		ClearPressed();
		return;
	}

	// 時間切れ → Miss
	if (m_elapsed - m_readyTime >= m_timeLimit)
	{
		Finish(SkillResult::Miss);
		return;
	}

	PlayKeyDownSound();

	OnUpdate(1.0f / 60.0f);
}

void SkillCheckBase::Draw()
{
	if (IsFinished()) return;

	OnDraw();
}

void SkillCheckBase::Finish(SkillResult result)
{
	m_result = result;
}

int SkillCheckBase::KeyIndex(int vk)
{
	for (int i = 0; i < kKeyCount; ++i)
	{
		if (SkillKeyUI::ToVirtualKey(static_cast<SkillKey>(i)) == vk) return i;
	}
	return -1;
}

void SkillCheckBase::PlayKeyDownSound() const
{
	for (int i = 0; i < kKeyCount; ++i)
	{
		if (m_keyPressed[i])
		{
			if (auto se = KdAudioManager::Instance().Play("Asset/Sound/KeyDown/KeyDown.wav"))
			{
				se->SetVolume(0.2f);
				se->SetPitch(-0.2f);
			}
			return;
		}
	}
}

void SkillCheckBase::PollKeys()
{
	for (int i = 0; i < kKeyCount; ++i)
	{
		const SHORT state = GetAsyncKeyState(SkillKeyUI::ToVirtualKey(static_cast<SkillKey>(i)));
		const bool  down = (state & 0x8000) != 0;

		// 「押された記録」、「前フレームは離れていて今は押している」
		m_keyPressed[i] = ((state & 0x0001) != 0) || (down && !m_keyPrevDown[i]);
		m_keyDown[i] = down;
		m_keyPrevDown[i] = down;
	}
}

void SkillCheckBase::ClearPressed()
{
	for (int i = 0; i < kKeyCount; ++i) m_keyPressed[i] = false;
}

bool SkillCheckBase::IsKeyPressed(int vk) const
{
	const int i = KeyIndex(vk);
	return i >= 0 && m_keyPressed[i];
}

bool SkillCheckBase::IsKeyDown(int vk) const
{
	const int i = KeyIndex(vk);
	return i >= 0 && m_keyDown[i];
}

void SkillCheckBase::DrawRect(float x, float y, float halfW, float halfH, const Math::Color& color)
{
	if (halfW <= 0.0f || halfH <= 0.0f) return;

	Math::Color c = color;
	KdShaderManager::Instance().m_spriteShader.DrawBox(
		static_cast<int>(x), static_cast<int>(y + kLayoutOffsetY),
		static_cast<int>(halfW), static_cast<int>(halfH),
		&c, true);
}

void SkillCheckBase::DrawCircle(float x, float y, float radius, const Math::Color& color, bool fill)
{
	if (radius <= 0.0f) return;

	Math::Color c = color;
	KdShaderManager::Instance().m_spriteShader.DrawCircle(
		static_cast<int>(x), static_cast<int>(y + kLayoutOffsetY),
		static_cast<int>(radius),
		&c, fill);
}

void SkillCheckBase::DrawRing(float x, float y, float radius, float thickness, const Math::Color& color)
{
	// 半径をずらして太く見せる
	const float half = thickness * 0.5f;

	for (float d = -half; d <= half; d += 1.0f)
	{
		DrawCircle(x, y, radius + d, color, false);
	}
}
void SkillCheckBase::DrawTriangle(float x1, float y1, float x2, float y2, float x3, float y3,
	const Math::Color& color)
{
	Math::Color c = color;
	KdShaderManager::Instance().m_spriteShader.DrawTriangle(
		static_cast<int>(x1), static_cast<int>(y1 + kLayoutOffsetY),
		static_cast<int>(x2), static_cast<int>(y2 + kLayoutOffsetY),
		static_cast<int>(x3), static_cast<int>(y3 + kLayoutOffsetY),
		&c, true);
}

void SkillCheckBase::DrawPanel(float x, float y, float halfW, float halfH, float alpha)
{
	DrawRect(x, y, halfW + 2.0f, halfH + 2.0f, { 1.0f, 1.0f, 1.0f, 0.25f * alpha });	// 枠
	DrawRect(x, y, halfW, halfH, { 0.04f, 0.04f, 0.06f, 0.65f * alpha });		// 中
}

void SkillCheckBase::DrawArrowShape(float x, float y, int fx, int fy, float s, const Math::Color& color)
{
	const float f_x = static_cast<float>(fx);
	const float f_y = static_cast<float>(fy);
	const float p_x = static_cast<float>(-fy);	//  進行方向を90度回転した方向ベクトル。このベクトルを使って三角形の幅を決める
	const float p_y = static_cast<float>(fx);

	// 三角
	DrawTriangle(
		x + f_x * s, y + f_y * s,
		x + p_x * s * 0.9f, y + p_y * s * 0.9f,
		x - p_x * s * 0.9f, y - p_y * s * 0.9f,
		color);

	// 長方形
	const float len = s * 0.55f;
	const float wid = s * 0.32f;
	const bool  horizontal = (fx != 0);

	DrawRect(x - f_x * len, y - f_y * len,
		horizontal ? len : wid,
		horizontal ? wid : len,
		color);
}

void SkillCheckBase::DrawTimeBar(float x, float y, float halfW, float perfectTime) const
{
	const float a = GetAlpha();
	const float halfH = 3.0f;

	float elapsed = m_elapsed - m_readyTime;	 // 開始からの時間
	if (elapsed < 0.0f) elapsed = 0.0f;

	float remain = 1.0f - elapsed / m_timeLimit;
	if (remain < 0.0f) remain = 0.0f;
	if (remain > 1.0f) remain = 1.0f;

	// 背景
	DrawRect(x, y, halfW, halfH, { 0.0f, 0.0f, 0.0f, 0.6f * a });

	Math::Color color = { 1.0f, 1.0f, 1.0f, a };
	if (perfectTime > 0.0f && elapsed <= perfectTime) color = { 1.0f, 0.9f, 0.2f, a };
	else if (remain < 0.25f) color = { 1.0f, 0.3f, 0.3f, a };

	// 左端固定
	DrawRect(x - halfW + halfW * remain, y, halfW * remain, halfH, color);
}

void SkillCheckBase::DrawKey(SkillKey key, float x, float y, SkillKeyAnim anim, float alphaMul) const
{
	const bool isDown = IsReady() && IsKeyDown(SkillKeyUI::ToVirtualKey(key));
	SkillKeyUI::Instance().Draw(key, x, y + kLayoutOffsetY, anim, m_elapsed, isDown, GetAlpha() * alphaMul);
}