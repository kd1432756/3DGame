#include "FloatingTextUI.h"

FloatingTextUI::FloatingTextUI()
{
	m_lifeTime = 1.0f;
	m_fontId = FontID::Future48;
	m_color = { 1.0f, 0.78f, 0.10f, 1.0f };
}

void FloatingTextUI::OnUpdate(float dt, float t)
{
	m_pos.y += 0.5f;

	// フェードインからフェードアウト
	m_alpha = CalcFadeAlpha(t);
}
