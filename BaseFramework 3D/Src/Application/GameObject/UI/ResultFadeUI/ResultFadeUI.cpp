#include "ResultFadeUI.h"

void ResultFadeUI::DrawSprite()
{
	if (m_alpha <= 0.0f) return;

	DrawFullScreenBox(Math::Color{ 0.0f, 0.0f, 0.0f, m_alpha });
}
