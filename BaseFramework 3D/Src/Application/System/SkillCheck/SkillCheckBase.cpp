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

	// 開始時に残っている入力を拾わないよう、ここで一度読み捨てる
	DrainInput();

	OnStart();
}

void SkillCheckBase::Update()
{
	if (IsFinished()) return;

	m_elapsed += kDeltaTime;

	// 準備時間中は入力を受け付けない
	if (!IsReady())
	{
		DrainInput();
		return;
	}

	// 時間切れ → Miss
	if (m_elapsed - m_readyTime >= m_timeLimit)
	{
		Finish(SkillResult::Miss);
		return;
	}

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

bool SkillCheckBase::IsKeyPressed(int vk)
{
	return (GetAsyncKeyState(vk) & 0x0001) != 0;
}

bool SkillCheckBase::IsKeyDown(int vk)
{
	return (GetAsyncKeyState(vk) & 0x8000) != 0;
}

void SkillCheckBase::DrainInput()
{
	GetAsyncKeyState(VK_SPACE);
	GetAsyncKeyState('W');
	GetAsyncKeyState('A');
	GetAsyncKeyState('S');
	GetAsyncKeyState('D');
}

void SkillCheckBase::DrawRect(float x, float y, float halfW, float halfH, const Math::Color& color)
{
	if (halfW <= 0.0f || halfH <= 0.0f) return;

	Math::Color c = color;
	KdShaderManager::Instance().m_spriteShader.DrawBox(
		static_cast<int>(x), static_cast<int>(y),
		static_cast<int>(halfW), static_cast<int>(halfH),
		&c, true);
}