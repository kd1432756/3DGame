#include "InteractiveObject.h"

#include "../../../../Scene/SceneManager.h"
#include "../../../../System/Score/ScoreManager.h"

#include "../../../Character/Player/Player.h"
#include "../../../Character/Enemy/Neighbor/Neighbor.h"

#include "../Item/Item.h"

#include "../../../../System/DayCycle/DayCycle.h"

void InteractiveObject::DrawLit()
{
	auto player = SceneManager::Instance().FindObject<Player>();
	bool canUse = CanUse(player);

	float cycle = canUse ? 1.0f : 2.0f;	// 周期
	m_limLightTimer += (2.0f * DirectX::XM_PI) / (cycle * 60.0f);

	float t = (sinf(m_limLightTimer) + 1.0f) * 0.5f;

	float pow = 1.0f + t * 3.0f;

	Math::Vector3 color = canUse
		? Math::Vector3{ 0.9f, 0.6f, 0.1f }
	: Math::Vector3{ 0.5f, 0.5f, 0.5f };

	KdShaderManager::Instance().m_StandardShader.SetLimLightEnable(true);
	KdShaderManager::Instance().m_StandardShader.SetLimLight(color, pow);

	StaticMeshObject::DrawLit();

	KdShaderManager::Instance().m_StandardShader.SetLimLightEnable(false);
}

void InteractiveObject::DrawBright()
{
	if (!IsFocused())
	{
		return;
	}

	if (!m_spModel) return;

	Math::Color color{ 0.1,0.1,0.1,1 };
	KdShaderManager::Instance().m_StandardShader.DrawModel(
		*m_spModel,
		m_mWorld,
		color);
}

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

	for (const auto& action : m_actions)
	{

		if (action.fromState != m_state)
		{
			continue;
		}

		if (action.needItem)
		{
			// アイテム必須：従来どおり照合
			if (!item)
			{
				continue;
			}

			const auto& data = item->GetItemData();

			if (!data.FindUse(action.action, action.target))
			{
				continue;
			}
		}
		else
		{
			// アイテム不要：手ぶらのときだけ
			if (item)
			{
				continue;
			}
		}

		const bool isRepair = (action.action == ItemAction::Repair);

		// いたずらはOwner不可
		if (!isRepair && IsOwner(player))
		{
			continue;
		}

		// 修復はOwnerのみ
		if (isRepair && !IsOwner(player))
		{
			continue;
		}

		return &action;
	}

	return nullptr;
}

const InteractiveAction*
InteractiveObject::FindNPCAction(
	const ItemData& item,
	const std::string& npcName) const
{
	const bool isOwner = (m_owner == npcName);

	for (const auto& action : m_actions)
	{
		if (action.fromState != m_state) continue;
		if (!action.npcCanUse) continue;
		if (!action.needItem) continue;
		if (!item.FindUse(action.action, action.target)) continue;
		const bool isRepair = (action.action == ItemAction::Repair);
		if (isRepair != isOwner) continue;
		return &action;
	}

	return nullptr;
}

const InteractiveAction* InteractiveObject::FindNPCBareAction(const std::string& npcName) const
{
	const bool isOwner = (m_owner == npcName);

	for (const auto& action : m_actions)
	{
		if (action.fromState != m_state) continue;
		if (!action.npcCanUse) continue;
		if (action.needItem) continue;

		const bool isRepair = (action.action == ItemAction::Repair);

		// 修復はオーナーのみ、いたずらはオーナー不可
		if (isRepair != isOwner) continue;

		return &action;
	}
	return nullptr;
}

void InteractiveObject::ExecuteNPCAction(const InteractiveAction& action, const std::shared_ptr<Neighbor>& npc)
{
	m_state = action.toState;

	if (!action.modelPath.empty())
	{
		SetModelPath(action.modelPath);
	}

	if (action.score > 0)
	{
		int score = ScoreManager::Instance().CalcScore(
			action.score, DayCycle::Instance().GetTimeProgress());
		npc->OnPrankSuccess(score, action.message);
	}

	NotifyOwner(action);
	OnAction(action, nullptr);
}

bool InteractiveObject::CanUse(
	const std::shared_ptr<Player>& player)
{
	return FindAction(player) != nullptr;
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
		int score = ScoreManager::Instance().CalcScore(
			action->score, 
			DayCycle::Instance().GetTimeProgress(),
			player->GetSkillResult()
			);

		player->OnPrankSuccess(score, action->message);
	}

	if (auto se = KdAudioManager::Instance().Play("Asset/Sound/SE/Blip_Up.wav"))
	{
		se->SetVolume(0.1f);
	}

	NotifyOwner(*action);

	// 個別特殊処理
	OnAction(*action, player);
}

std::string InteractiveObject::GetActionText(
	const std::shared_ptr<Player>& player) const
{
	const auto* action = FindAction(player);
	if (!action)
	{
		return "";
	}

	if (!action->needItem)
	{
		return action->actionText;
	}

	auto item = player->GetHandItem();
	if (!item)
	{
		return "";
	}

	const auto* use =
		item->GetItemData().FindUse(action->action, action->target);

	if (!use)
	{
		return "";
	}

	return use->actionText.empty() ? action->actionText : use->actionText;
}

bool InteractiveObject::CanNPCUse(
	const ItemData& item,
	const std::string& npcName) const
{
	return FindNPCAction(item, npcName) != nullptr;
}

float InteractiveObject::GetNPCUseTime(
	const ItemData& itemData,
	const std::string& npcName) const
{
	const auto* action =
		FindNPCAction(itemData, npcName);

	if (!action)
	{
		return 0.0f;
	}

	return action->useTime;
}

void InteractiveObject::NPCUse(
	const ItemData& item,
	const std::shared_ptr<Neighbor>& npc)
{
	const auto* action =
		FindNPCAction(item, npc->GetName());

	if (!action)
	{
		return;
	}

	ExecuteNPCAction(*action, npc);
}

bool InteractiveObject::CanNPCUseBare(const std::string& npcName) const
{
	return FindNPCBareAction(npcName) != nullptr;
}

float InteractiveObject::GetNPCUseTimeBare(const std::string& npcName) const
{
	const auto* action = FindNPCBareAction(npcName);
	return action ? action->useTime : 0.0f;
}

void InteractiveObject::NPCUseBare(const std::shared_ptr<Neighbor>& npc)
{
	const auto* action = FindNPCBareAction(npc->GetName());
	if (!action) return;
	ExecuteNPCAction(*action, npc);
}

bool InteractiveObject::IsRepairAction(
	const std::shared_ptr<Player>& player) const
{
	const auto* action = FindAction(player);
	return action && action->action == ItemAction::Repair;
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
