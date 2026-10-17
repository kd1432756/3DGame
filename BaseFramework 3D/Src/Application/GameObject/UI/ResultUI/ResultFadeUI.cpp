#include "ResultFadeUI.h"

void ResultFadeUI::Init()
{
	m_updateGroup = UpdateGroup::Always;
}

void ResultFadeUI::DrawSprite()
{
	if (m_alpha <= 0.0f) return;
	Math::Color c = { 0, 0, 0, m_alpha };
	KdShaderManager::Instance().m_spriteShader.DrawBox(0, 0, 640, 360, &c);
}
