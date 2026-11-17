#include "BonusUI.h"

#include "../../Character/Player/Player.h"

void BonusUI::Init()
{
	m_keyTex = LoadTexture("Asset/Textures/UI/Key/keyboard_e.png");
	m_animTime = 0.0f;
}

void BonusUI::Update()
{
	m_animTime += DeltaTime();
}

void BonusUI::DrawSprite()
{
	auto player = LockPlayer();

	if (!player) return;

	if (player->GetRepairBonusCount() > 0)
	{
		const float remain = player->GetRepairBonusTime();

		const bool low = remain <= 10.0f;

		const Math::Color labelColor = low
			? Math::Color{ 1.0f, 0.4f, 0.3f, 1.0f }
		: Math::Color{ 1.0f, 0.85f, 0.2f, 1.0f };

		// 倍率
		char text[32];
		sprintf_s(text, "x%.1f  %ds",
			player->GetRepairBonusRate(),
			static_cast<int>(std::ceil(remain)));

		DrawLabel(Math::Vector2{ kBonusX, kBonusY }, labelColor,
			FontID::Future24, text);

		// 残り時間
		DrawGauge(
			static_cast<int>(kBonusX),
			static_cast<int>(kBonusY + kBarOffsetY),
			kBarHalfW, kBarHalfH,
			player->GetRepairBonusTimeRatio(),
			Math::Color{ 0.0f, 0.0f, 0.0f, 0.5f },
			labelColor);
	}

	// Eキーと「Deliver」
	if (player->CanDeliver() && !player->IsUsing() && !player->IsArrested())
	{
		const float a = 0.75f + 0.25f * std::sin(m_animTime * 6.0f);

		DrawKeyAndText(
			m_keyTex.get(),
			Math::Vector2{ kPromptX, kPromptY },
			"Deliver",
			Math::Color{ 1.0f, 1.0f, 1.0f, a },
			FontID::Future24);
	}
}