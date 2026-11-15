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
		if (action.fromState != m_state) continue;

		// プレイヤーは修復アクションだけ
		if (action.action != ItemAction::Repair) continue;

		if (action.needItem)
		{
			if (!item) continue;
			if (!item->GetItemData().FindUse(action.action, action.target)) continue;
		}
		else
		{
			if (item) continue;
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
	for (const auto& action : m_actions)
	{
		if (action.fromState != m_state) continue;
		if (!action.npcCanUse) continue;
		if (!action.needItem) continue;
		if (!item.FindUse(action.action, action.target)) continue;
		const bool isRepair = (action.action == ItemAction::Repair);
		if (action.action == ItemAction::Repair) continue;
		return &action;
	}

	return nullptr;
}

const InteractiveAction* InteractiveObject::FindNPCBareAction(const std::string& npcName) const
{
	for (const auto& action : m_actions)
	{
		if (action.fromState != m_state) continue;
		if (!action.npcCanUse) continue;
		if (action.needItem) continue;
		const bool isRepair = (action.action == ItemAction::Repair);
		if (action.action == ItemAction::Repair) continue;
		return &action;
	}
	return nullptr;
}

void InteractiveObject::ExecuteNPCAction(const InteractiveAction& action, const std::shared_ptr<Neighbor>& npc)
{
	m_state = action.toState;

	OnStateChanged(DamageSource::NPC);

	if (!action.modelPath.empty())
	{
		SetModelPath(action.modelPath);
	}

	NotifyPlayer(action);

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

	OnStateChanged(DamageSource::None);

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
			player->GetSkillResult(),
			player->GetRepairBonusRate()
			);

		player->OnRepairSuccess(score, action->message);
	}

	if (auto se = KdAudioManager::Instance().Play("Asset/Sound/SE/Blip_Up.wav"))
	{
		se->SetVolume(0.1f);
	}

	NotifyPlayer(*action);

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

void InteractiveObject::OnStateChanged(DamageSource source)
{
	m_reservedBy.reset();

	const float now = DayCycle::Instance().GetCurrentTime();

	if (m_state == InteractiveState::Damaged)
	{
		m_damageSource = source;
		m_damagedTime = now;
	}
	else
	{
		m_damageSource = DamageSource::None;
		m_repairedTime = now;	// 修復された時刻
	}
}

void InteractiveObject::NotifyPlayer(const InteractiveAction& action)
{
	if (action.ownerMessage.empty()) return;

	auto player = SceneManager::Instance().FindObject<Player>();
	if (!player) return;

	player->OnObjectDamaged(
		std::dynamic_pointer_cast<InteractiveObject>(shared_from_this()),
		action);
}

bool InteractiveObject::SetInitialDamaged()
{
	for (const auto& action : m_actions)
	{
		if (action.fromState != InteractiveState::Normal) continue;
		if (action.action == ItemAction::Repair) continue;

		m_state = action.toState;

		if (!action.modelPath.empty())
		{
			SetModelPath(action.modelPath);
		}

		OnStateChanged(DamageSource::Initial);
		OnInitialDamage();
		return true;
	}

	return false;
}

bool InteractiveObject::CanBeTargetedBy(const CharacterBase* npc) const
{
	if (m_state != InteractiveState::Normal) return false;

	auto reserved = m_reservedBy.lock();
	if (reserved && reserved.get() != npc) return false;

	const float elapsed = DayCycle::Instance().GetCurrentTime() - m_repairedTime;
	return elapsed >= m_repairCooldownHours;
}

bool InteractiveObject::TryReserve(const std::shared_ptr<CharacterBase>& npc)
{
	if (!CanBeTargetedBy(npc.get())) return false;

	m_reservedBy = npc;
	return true;
}

void InteractiveObject::ReleaseReservation(const CharacterBase* npc)
{
	auto reserved = m_reservedBy.lock();
	if (reserved.get() == npc)
	{
		m_reservedBy.reset();
	}
}

float InteractiveObject::GetSecondsSinceDamaged() const
{
	if (m_state != InteractiveState::Damaged)
	{
		return FLT_MAX;
	}

	const float hours =
		DayCycle::Instance().GetCurrentTime() - m_damagedTime;

	return DayCycle::Instance().GameHoursToRealSeconds(hours);
}

bool InteractiveObject::CanEverBeBrokenBy(const ItemData& item) const
{
	if (m_state == InteractiveState::Repaired)
	{
		return false;
	}

	for (const auto& action : m_actions)
	{
		if (!action.npcCanUse || !action.needItem) continue;
		if (action.action == ItemAction::Repair) continue;

		if (item.FindUse(action.action, action.target))
		{
			return true;
		}
	}

	return false;
}