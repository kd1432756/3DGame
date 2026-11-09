#include "HinderUI.h"

#include "../../Character/Player/Player.h"

void HinderUI::Init()
{
	m_texKeyF.Load("Asset/Textures/UI/Key/keyboard_f.png");
	m_texKeySpace.Load("Asset/Textures/UI/Key/keyboard_space.png");
}

void HinderUI::Update()
{
	m_canHinder = false;
	m_isHindering = false;

	auto player = LockPlayer();
	if (!player) return;

	m_frame++;

	m_isHindering = player->IsHindering();
	m_canHinder = player->CanHinder();
	m_hinderRatio = player->GetHinderRatio();
}

void HinderUI::DrawSprite()
{
	//============================================================
	// 妨害可能プロンプト（InteractionPromptの下に出す）
	//============================================================
	if (m_canHinder)
	{
		DrawKeyAndText(&m_texKeyF, Math::Vector2{ 0, 90 }, "Hinder", kWhiteColor);
	}

	//============================================================
	// 妨害中：残り時間バー + 連打指示
	//============================================================
	if (m_isHindering)
	{
		// 残り時間バー（左詰め）
		DrawGauge(0, -140, 120, 8, m_hinderRatio,
			Math::Color{ 0.0f, 0.0f, 0.0f, 0.5f },
			Math::Color{ 1.0f, 0.8f, 0.3f, 1.0f });

		// 連打指示（約0.3秒周期で点滅）
		const bool bright = (m_frame / 10) % 2 == 0;
		const Math::Color blink = bright
			? Math::Color{ 1.0f, 0.9f, 0.3f, 1.0f }
		: Math::Color{ 1.0f, 1.0f, 1.0f, 1.0f };

		// アイコンも文字も一緒に点滅
		DrawKeyAndText(&m_texKeySpace, Math::Vector2{ 0, -105 }, "Struggle!", blink);
	}
}
