#include "InteractionPrompt.h"

#include "../../Character/Player/Player.h"

#include "../../Object/StaticMeshObject/Item/Item.h"
#include "../../Object/StaticMeshObject/InteractiveObject/InteractiveObject.h"

void InteractionPrompt::Init()
{
	m_texKeyE.Load("Asset/Textures/UI/Key/keyboard_e.png");
}

void InteractionPrompt::Update()
{
	m_visible = false;
	m_showKeyIcon = false;

	auto player = LockPlayer();

	if (!player) return;

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

void InteractionPrompt::DrawSprite()
{
	if (!m_visible) return;

	const Math::Vector2 pos{ 0, 50 };

	if (m_showKeyIcon)
	{
		// 「Eキー + 文字」を中央揃えで描く
		DrawKeyAndText(&m_texKeyE, pos, m_prompt.c_str(), kWhiteColor);
	}
	else
	{
		// アイコン不要なら文字だけ中央に描く
		DrawLabel(pos, kWhiteColor, FontID::Future24, m_prompt.c_str());
	}
}
