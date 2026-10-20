#include "SkillCheckBase.h"

namespace
{
	constexpr float kDeltaTime = 1.0f / 60.0f;	// Player::UpdateUse と同じ固定値
}

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

	PollKeys();	// キー状態は1フレームに1回だけ読む

	m_elapsed += kDeltaTime;

	// 準備時間中は入力を受け付けない
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

	OnUpdate(kDeltaTime);
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

// 入力受付中に、いずれかのキーが押されたフレームでSEを鳴らす
void SkillCheckBase::PlayKeyDownSound() const
{
	for (int i = 0; i < kKeyCount; ++i)
	{
		if (m_keyPressed[i])
		{
			// 同じフレームに複数押されても、鳴らすのは1回だけ			;
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

		// 「押された記録(0x0001)」か、「前フレームは離れていて今は押している」のどちらか
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
	// 半径を1pxずつずらした輪郭を重ねて太く見せる
	const float half = thickness * 0.5f;

	for (float d = -half; d <= half; d += 1.0f)
	{
		DrawCircle(x, y, radius + d, color, false);
	}
}

void SkillCheckBase::DrawKey(SkillKey key, float x, float y, SkillKeyAnim anim, float alphaMul) const
{
	const bool isDown = IsReady() && IsKeyDown(SkillKeyUI::ToVirtualKey(key));
	SkillKeyUI::Instance().Draw(key, x, y + kLayoutOffsetY, anim, m_elapsed, isDown, GetAlpha() * alphaMul);
}