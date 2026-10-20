#include "InteractionPrompt.h"

#include "../../../Scene/SceneManager.h"
#include "../../Character/Player/Player.h"

#include "../../Object/StaticMeshObject/Item/Item.h"
#include "../../Object/StaticMeshObject/InteractiveObject/InteractiveObject.h"

void InteractionPrompt::Init()
{
	m_texKeyE.Load("Asset/Textures/UI/Key/keyboard_e.png");
}

void InteractionPrompt::DrawSprite()
{
	if (!m_visible) return;

	auto& sprite = KdShaderManager::Instance().m_spriteShader;

	constexpr float iconH = 28.0f;
	constexpr float gap = 12.0f;
	const Math::Vector2 pos{ 0, 50 };

	// アイコン不要なら従来どおり文字だけ中央に描く
	if (!m_showKeyIcon)
	{
		sprite.DrawFont(
			pos,
			&kWhiteColor,
			Math::Vector2{ 0.5f, 0.5f },
			0.0f,
			FontID::Future24,
			m_prompt.c_str());
		return;
	}

	// アイコンサイズ（縦横比維持）
	const float iconW = iconH * (float)m_texKeyE.GetInfo().Width / (float)m_texKeyE.GetInfo().Height;

	// 文字サイズ
	auto fontSprite = KdFontManager::Instance().CreateFontTexture(
		static_cast<int>(FontID::Future24), m_prompt.c_str(), false);
	const Math::Vector2 textSize = sprite.CalcFontSize(fontSprite);

	// 「アイコン + 隙間 + 文字」全体を中央揃え
	const float totalW = iconW + gap + textSize.x;
	const float left = pos.x - totalW * 0.5f;

	sprite.DrawTex(&m_texKeyE, (int)left, (int)pos.y, (int)iconW, (int)iconH,
		nullptr, &kWhiteColor, Math::Vector2{ 0.0f, 0.5f });

	sprite.DrawFont(
		Math::Vector2{ left + iconW + gap, pos.y },
		&kWhiteColor,
		Math::Vector2{ 0.0f, 0.5f },
		0.0f,
		FontID::Future24, 
		m_prompt.c_str());
}

void InteractionPrompt::Update()
{
	m_visible = false;
	m_showKeyIcon = false;

	std::shared_ptr<Player> player = m_wpPlayer.lock();

	if (!player)return;

	if (!player->CanHighlight())
	{
		return;
	}
	
	auto obj = player->GetFocusObject();

	if (auto item = std::dynamic_pointer_cast<Item>(obj))
	{
		m_prompt = item->GetItemData().GetName();
		m_showKeyIcon = true;
		m_visible = true;
		return;
	}

	if (auto interact = std::dynamic_pointer_cast<InteractiveObject>(obj))
	{
		if (!interact->CanUse(player))
		{
			return;
		}

		m_prompt = interact->GetActionText(player) + std::string("    ") + interact->GetTypeName();
		m_showKeyIcon = false;
		m_visible = true;
		return;
	}
}
