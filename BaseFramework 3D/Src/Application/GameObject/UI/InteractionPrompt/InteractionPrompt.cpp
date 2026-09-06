#include "InteractionPrompt.h"

#include "../../../Scene/SceneManager.h"
#include "../../Character/Player/Player.h"

#include "../../Object/StaticMeshObject/Item/Item.h"
#include "../../Object/StaticMeshObject/InteractiveObject/InteractiveObject.h"

void InteractionPrompt::Init()
{

}

void InteractionPrompt::DrawSprite()
{
	if (!m_visible)	return;

	KdShaderManager::Instance().m_spriteShader.DrawFont(
		Math::Vector2{ 0,50 },
		&kWhiteColor,
		Math::Vector2{ 0.5f,0.5f },
		FontID::Future24,
		m_prompt.c_str()
	);
}

void InteractionPrompt::Update()
{
	m_visible = false;

	std::shared_ptr<Player> player = m_wpPlayer.lock();

	if (!player)return;

	if (!player->CanHighlight())
	{
		return;
	}
	
	auto obj = player->GetFocusObject();

	if (auto item = std::dynamic_pointer_cast<Item>(obj))
	{
		m_prompt = "[E]    " + item->GetItemData().GetName();
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
		m_visible = true;
		return;
	}
}
