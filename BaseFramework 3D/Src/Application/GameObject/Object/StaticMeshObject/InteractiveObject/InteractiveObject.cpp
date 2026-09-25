#include "InteractiveObject.h"

#include "../../../../Scene/SceneManager.h"

#include "../../../Character/Player/Player.h"
#include "../../../Character/Enemy/Neighbor/Neighbor.h"

#include "../Item/Item.h"

void InteractiveObject::RegisterAction(
	const InteractiveAction& action)
{
	m_actions.push_back(action);
}

const InteractiveAction*
InteractiveObject::FindAction(
	const std::shared_ptr<Player>& player) const
{
	if (!player)
	{
		return nullptr;
	}

	auto item = player->GetHandItem();

	if (!item)
	{
		return nullptr;
	}

	const auto& data = item->GetItemData();

	for (const auto& action : m_actions)
	{
		if (action.fromState != m_state)
		{
			continue;
		}

		if (action.action != data.GetAction())
		{
			continue;
		}

		if (action.target != data.GetTarget())
		{
			continue;
		}

		// いたずらはOwner不可
		if (action.action != ItemAction::Repair &&
			IsOwner(player))
		{
			continue;
		}

		// 修復はOwnerのみ
		if (action.action == ItemAction::Repair &&
			!IsOwner(player))
		{
			continue;
		}

		return &action;
	}

	return nullptr;
}

const InteractiveAction*
InteractiveObject::FindNPCAction(
	const ItemData& item) const
{
	for (const auto& action : m_actions)
	{
		if (action.fromState != m_state)
		{
			continue;
		}

		if (!action.npcCanUse)
		{
			continue;
		}

		if (action.action != item.GetAction())
		{
			continue;
		}

		if (action.target != item.GetTarget())
		{
			continue;
		}

		return &action;
	}

	return nullptr;
}

bool InteractiveObject::CanUse(
	const std::shared_ptr<Player>& player)
{
	return FindAction(player) != nullptr;
}

void InteractiveObject::Use(
	const std::shared_ptr<Player>& player)
{
	const auto* action = FindAction(player);

	if (!action)
	{
		return;
	}

	// 状態変更
	m_state = action->toState;

	// モデル変更
	if (!action->modelPath.empty())
	{
		SetModelPath(action->modelPath);
	}

	// スコア
	if (action->score > 0)
	{
		player->OnPrankSuccess(
			action->score,
			action->message);
	}

	NotifyOwner(*action);

	// 個別特殊処理
	OnAction(*action, player);
}

float InteractiveObject::GetUseTime(
	const std::shared_ptr<Player>& player) const
{
	const auto* action = FindAction(player);

	if (!action)
	{
		return 0.0f;
	}

	return action->useTime;
}

std::string InteractiveObject::GetActionText(
	const std::shared_ptr<Player>& player) const
{
	const auto* action = FindAction(player);

	if (!action)
	{
		return "";
	}

	auto item = player->GetHandItem();

	if (!item)
	{
		return "";
	}

	return item->GetItemData().GetActionText();
}

bool InteractiveObject::CanNPCUse(
	const ItemData& item) const
{
	return FindNPCAction(item) != nullptr;
}

void InteractiveObject::NPCUse(
	const ItemData& item,
	const std::shared_ptr<Neighbor>& npc)
{
	const auto* action =
		FindNPCAction(item);

	if (!action)
	{
		return;
	}

	m_state = action->toState;

	if (!action->modelPath.empty())
	{
		SetModelPath(action->modelPath);
	}

	if (action->score > 0)
	{
		npc->OnPrankSuccess(
			action->score,
			action->message);
	}

	NotifyOwner(*action);

	OnAction(*action, nullptr);
}

float InteractiveObject::GetNPCUseTime(
	const ItemData& itemData) const
{
	const auto* action =
		FindNPCAction(itemData);

	if (!action)
	{
		return 0.0f;
	}

	return action->useTime;
}

bool InteractiveObject::IsOwner(const std::shared_ptr<Player>& player) const
{
	if (!player)
	{
		return false;
	}

	return m_owner == player->GetName();
}

std::shared_ptr<CharacterBase>
InteractiveObject::FindOwner() const
{
	if (m_owner.empty())
	{
		return nullptr;
	}

	const auto& characters =
		SceneManager::Instance().FindObjects<CharacterBase>();

	for (const auto& character : characters)
	{
		if (!character)
		{
			continue;
		}

		if (character->GetName() == m_owner)
		{
			return character;
		}
	}

	return nullptr;
}

void InteractiveObject::NotifyOwner(const InteractiveAction& action)
{
	if (action.ownerMessage.empty())
	{
		return;
	}

	auto owner = FindOwner();

	if (!owner)
	{
		return;
	}

	owner->OnPranked(
		std::dynamic_pointer_cast<InteractiveObject>(shared_from_this()),
		action);
}
