#include "LoadingUI.h"

void LoadingUI::DrawSprite()
{
	// 背景
	DrawFullScreenBox(kBlackColor);

	// 「Loading」
	DrawLabel(
		Math::Vector2{ -500, -250 },
		kWhiteColor,
		FontID::Future24,
		"Loading",
		Math::Vector2{ 0.0f, 0.5f });

	// バー
	DrawGauge(0, -300, 500, 6, m_progress,
		Math::Color{ 0.5f, 0.5f, 0.5f, 0.5f },
		Math::Color{ 1.0f, 1.0f, 1.0f, 1.0f });
}
