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

	auto player = m_wpPlayer.lock();
	if (!player) return;

	m_frame++;

	m_isHindering = player->IsHindering();
	m_canHinder = player->CanHinder();
	m_hinderRatio = player->GetHinderRatio();
}

void HinderUI::DrawSprite()
{
	auto& sprite = KdShaderManager::Instance().m_spriteShader;

	//============================================================
	// 妨害可能プロンプト（InteractionPromptの下に出す）
	//============================================================
	if (m_canHinder)
	{
		DrawKeyText(m_texKeyF, Math::Vector2{ 0, 90 }, "Hinder", kWhiteColor);
	}

	//============================================================
	// 妨害中：残り時間バー + 連打指示
	//============================================================
	if (m_isHindering)
	{
		constexpr int width = 120;
		constexpr int height = 8;
		constexpr int x = 0;
		constexpr int y = -140;

		// 背景
		Math::Color bg = { 0, 0, 0, 0.5f };
		sprite.DrawBox(x, y, width, height, &bg);

		// 前景（左詰め）
		int currentWidth = static_cast<int>(width * m_hinderRatio);
		if (currentWidth > 0)
		{
			Math::Color fg = { 1.0f, 0.8f, 0.3f, 1.0f };
			sprite.DrawBox(
				x - width + currentWidth,
				y,
				currentWidth,
				height,
				&fg);
		}

		// 連打指示（約0.3秒周期で点滅）
		const bool bright = (m_frame / 10) % 2 == 0;
		Math::Color blink = bright
			? Math::Color{ 1.0f, 0.9f, 0.3f, 1.0f }
		: Math::Color{ 1.0f, 1.0f, 1.0f, 1.0f };

		// アイコンも文字も一緒に点滅
		DrawKeyText(m_texKeySpace, Math::Vector2{ 0, -105 }, "MASH!", blink);
	}
}

void HinderUI::DrawKeyText(const KdTexture& tex, const Math::Vector2& pos,
	const char* text, const Math::Color& color)
{
	auto& sprite = KdShaderManager::Instance().m_spriteShader;

	constexpr float iconH = 28.0f;
	constexpr float gap = 12.0f;

	// アイコンサイズ（縦横比維持。Spaceキーは横長になる）
	const float iconW = iconH * (float)tex.GetInfo().Width / (float)tex.GetInfo().Height;

	// 文字サイズ
	auto fontSprite = KdFontManager::Instance().CreateFontTexture(
		static_cast<int>(FontID::Future24), text, false);
	const Math::Vector2 textSize = sprite.CalcFontSize(fontSprite);

	// 全体を中央揃え
	const float totalW = iconW + gap + textSize.x;
	const float left = pos.x - totalW * 0.5f;

	sprite.DrawTex(&tex, (int)left, (int)pos.y, (int)iconW, (int)iconH,
		nullptr, &color, Math::Vector2{ 0.0f, 0.5f });

	sprite.DrawFont(
		Math::Vector2{ left + iconW + gap, pos.y },
		&color,
		Math::Vector2{ 0.0f, 0.5f },
		0.0f,
		FontID::Future24,
		text);
}
