#include "Neighbor.h"

#include "../../../../System/Navigation/NavMesh/NavMeshManager.h"

#include "../../../../Scene/SceneManager.h"

#include "../../../../System/Score/ScoreManager.h"

#include "../../../Object/StaticMeshObject/Item/Item.h"

#include "../../../Object/StaticMeshObject/InteractiveObject/InteractiveObject.h"

void Neighbor::Init()
{
	EnemyBase::Init();

	m_spModel->SetModelData(
		"Asset/Models/Character/Neighbor/Neighbor.gltf"
	);

	m_spAnimator = std::make_shared<KdAnimator>();
	m_spAnimator->SetAnimation(m_spModel->GetAnimation("Armature|Idle"));

	m_pCollider = std::make_unique<KdCollider>();

	m_pCollider->RegisterCollisionShape(
		"Neighbor",
		m_spModel,
		KdCollider::TypeEditor
	);

	//============================================================
	// Score Manager
	//============================================================

	ScoreManager::Instance().RegisterCharacter(
		std::dynamic_pointer_cast<CharacterBase>(
			shared_from_this()));

	//============================================================
	// NavMesh Manager
	//============================================================

	m_pNavMeshManager =
		&NavMeshManager::Instance();

	//============================================================
	// State
	//============================================================

	ApplyStateSettings();

	// 仮アイテム
	AddInventoryItem(
		"Asset/Data/Item/Scissors.json");
	AddInventoryItem(
		"Asset/Data/Item/Hammer.json");
	SelectInventory(0);
}

//============================================================
// Inventory
//============================================================

void Neighbor::AddInventoryItem(
	const std::string& itemPath)
{
	if (itemPath.empty())
	{
		return;
	}

	// Inventoryに追加
	if (!m_inventory.AddItem(itemPath))
	{
		// インベントリが満杯
		return;
	}
}

void Neighbor::SelectInventory(int index)
{
	m_inventory.SelectIndex(index);

	const InventoryItem* item =
		m_inventory.GetCurrentItem();

	if (!item)
	{
		m_currentItemData =
			ItemData();

		return;
	}

	m_currentItemData.Load(
		item->itemPath);
}

//============================================================
// AI
//============================================================

void Neighbor::UpdateBehavior()
{
	switch (m_state)
	{
	case State::Task:
		UpdateTask();
		break;

	case State::Prank:
		UpdatePrank();
		break;

	case State::Repair:
		UpdateRepair();
		break;
	}
}

//============================================================
// Task
//============================================================

void Neighbor::UpdateTask()
{
	//============================================================
	// 一定間隔でいたずら実行をランダムに試行
	//============================================================

	m_prankCheckTimer += 1.0f / 60.0f;

	if (m_prankCheckTimer >= m_prankCheckInterval)
	{
		m_prankCheckTimer = 0.0f;

		TryStartPrank();
	}

	// TODO: 自宅内でのタスク行動（今回は未実装）
}

//============================================================
// Prank Start
//============================================================

void Neighbor::TryStartPrank()
{
	//============================================================
	// 現在選択中のアイテムが存在するか確認
	//============================================================

	const InventoryItem* currentItem =
		m_inventory.GetCurrentItem();

	if (!currentItem)
	{
		return;
	}

	//============================================================
	// 対象検索
	//============================================================

	auto target =
		SearchPrankTarget();

	if (!target)
	{
		return;
	}

	//============================================================
	// 確率判定
	//============================================================

	float random =
		KdRandom::GetFloat(
			0.0f,
			1.0f);

	if (random > m_prankChance)
	{
		return;
	}

	//============================================================
	// ターゲット確定
	//============================================================

	m_wpPrankTarget =
		target;

	ChangeState(
		State::Prank);
}

//============================================================
// Search Prank Target
//============================================================

std::shared_ptr<InteractiveObject>
Neighbor::SearchPrankTarget()
{
	const ItemData& itemData =
		GetCurrentItemData();

	float searchRadius =
		m_prankSearchRadius * m_sceneScale;

	float searchDistanceSq =
		searchRadius * searchRadius;

	std::vector<
		std::shared_ptr<InteractiveObject>>
		candidates;

	auto& objList =
		SceneManager::Instance().GetObjList();

	for (auto& obj : objList)
	{
		if (!obj)
		{
			continue;
		}

		auto target =
			std::dynamic_pointer_cast<
			InteractiveObject>(obj);

		if (!target)
		{
			continue;
		}

		// 自分の所有物は対象外
		if (target->GetOwner() == m_owner)
		{
			continue;
		}

		// 現在装備しているItemで使用可能か
		if (!target->CanNPCUse(itemData))
		{
			continue;
		}

		Math::Vector3 toTarget =
			target->GetPos() - m_pos;

		toTarget.y = 0.0f;

		if (toTarget.LengthSquared() >
			searchDistanceSq)
		{
			continue;
		}

		candidates.push_back(target);
	}

	if (candidates.empty())
	{
		return nullptr;
	}

	int index =
		KdRandom::GetInt(
			0,
			static_cast<int>(
				candidates.size()) - 1);

	return candidates[index];
}


//============================================================
// Prank Update
//============================================================

void Neighbor::UpdatePrank()
{
	auto target =
		m_wpPrankTarget.lock();

	if (!target)
	{
		ChangeState(
			State::Task);

		return;
	}

	//============================================================
	// 現在選択中のアイテムを取得
	//============================================================

	const ItemData& itemData =
		GetCurrentItemData();

	//============================================================
	// インタラクト中
	//============================================================

	if (m_isInteracting)
	{
		m_interactTimer +=
			1.0f / 60.0f;

		//========================================================
		// ItemDataからUseTimeを取得
		//========================================================

		float useTime =
			target->GetNPCUseTime(itemData);

		if (m_interactTimer >= useTime)
		{
			m_isInteracting = false;

			m_interactTimer = 0.0f;

			//====================================================
			// NPC使用
			//====================================================

			target->NPCUse(itemData);

			ChangeState(
				State::Task);
		}

		return;
	}

	//============================================================
	// ターゲットまで移動
	//============================================================

	if (MoveToPrankTarget())
	{
		m_spAnimator->SetAnimation(
			m_spModel->GetAnimation(
				"Armature|Sprint")
		);

		return;
	}

	//============================================================
	// 到着
	//============================================================

	SetMoveDirection(
		Math::Vector3::Zero);

	m_isInteracting = true;

	m_interactTimer = 0.0f;

	m_spAnimator->SetAnimation(
		m_spModel->GetAnimation(
			"Armature|Idle")
	);
}

bool Neighbor::MoveToPrankTarget()
{
	auto target = m_wpPrankTarget.lock();

	if (!target)
	{
		return false;
	}

	//============================================================
	// ターゲットとの距離判定（当たり判定があるため、
	// NavMesh Pathの終端到達ではなくインタラクト可能距離で判定）
	//============================================================

	Math::Vector3 toTarget =
		target->GetPos() - m_pos;

	toTarget.y = 0.0f;

	const float interactDistance =
		m_prankInteractDistance * m_sceneScale;

	if (toTarget.LengthSquared() <=
		interactDistance * interactDistance)
	{
		m_navPath.clear();
		m_navPathIndex = 0;

		return false;
	}

	//============================================================
	// Pathが無ければ探索
	//============================================================

	if (m_navPath.empty())
	{
		FindPathTo(
			target->GetPos());

		if (m_navPath.empty())
		{
			// 到着扱いにせず中止
			ChangeState(
				State::Task);

			return false;
		}
	}

	return MoveToNextNavPathPoint();
}

//============================================================
// Repair
//============================================================

void Neighbor::UpdateRepair()
{
	// TODO: 被害発見トリガー未実装のため、Repair処理も未実装
	// 遷移してきた場合は一旦Taskへ戻す

	ChangeState(State::Task);
}

//============================================================
// NavMesh
//============================================================

void Neighbor::FindPathTo(const Math::Vector3& goal)
{
	if (!m_pNavMeshManager)
	{
		return;
	}

	m_navPath =
		m_navMeshPathfinder.FindPathPoints(
			*m_pNavMeshManager,
			m_pos,
			goal
		);

	m_navPathIndex = 0;
}

bool Neighbor::MoveToNextNavPathPoint()
{
	if (m_navPath.empty())
	{
		return false;
	}

	if (m_navPathIndex >=
		static_cast<int>(m_navPath.size()))
	{
		m_navPath.clear();
		m_navPathIndex = 0;

		return false;
	}

	//============================================================
	// 現在のTarget Point
	//============================================================

	const Math::Vector3 targetPoint =
		m_navPath[m_navPathIndex];

	//============================================================
	// 移動方向
	//============================================================

	Math::Vector3 dir =
		targetPoint - m_pos;

	dir.y = 0.0f;

	//============================================================
	// 到着判定
	//============================================================

	const float arriveDistance = 0.2f * m_sceneScale;

	if (dir.LengthSquared() <=
		arriveDistance * arriveDistance)
	{
		m_navPathIndex++;

		//========================================================
		// まだPathが残っている
		//========================================================

		if (m_navPathIndex <
			static_cast<int>(m_navPath.size()))
		{
			return true;
		}

		//========================================================
		// Path完了
		//========================================================

		m_navPath.clear();
		m_navPathIndex = 0;

		return false;
	}

	//============================================================
	// 移動
	//============================================================

	SetMoveDirection(dir);

	return true;
}

//============================================================
// State
//============================================================

void Neighbor::ChangeState(State newState)
{
	if (m_state == newState)
	{
		return;
	}

	m_state = newState;

	// State切替時はPathと待機状態をリセット
	m_navPath.clear();
	m_navPathIndex = 0;

	m_isInteracting = false;
	m_interactTimer = 0.0f;

	ApplyStateSettings();
}

void Neighbor::ApplyStateSettings()
{
	switch (m_state)
	{
	case State::Task:

		SetMoveSpeed(m_taskSpeed);
		m_spAnimator->SetAnimation(
			m_spModel->GetAnimation("Armature|Idle")
		);

		break;

	case State::Prank:

		SetMoveSpeed(m_prankSpeed);
		m_spAnimator->SetAnimation(
			m_spModel->GetAnimation("Armature|Sprint")
		);

		break;

	case State::Repair:

		SetMoveSpeed(m_repairSpeed);

		break;
	}
}