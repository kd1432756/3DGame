#include "Neighbor.h"

#include "../../../../System/Navigation/NavMesh/NavMeshManager.h"

#include "../../../../Scene/SceneManager.h"

#include "../../../../System/Score/ScoreManager.h"

#include "../../../Object/StaticMeshObject/Item/Item.h"

#include "../../../Object/StaticMeshObject/InteractiveObject/InteractiveObject.h"

//============================================================
// Init
//============================================================

void Neighbor::Init()
{
	EnemyBase::Init();

	m_spModel->SetModelData("Asset/Models/Character/Neighbor/Neighbor.gltf");

	m_spAnimator = std::make_shared<KdAnimator>();

	m_spAnimator->SetAnimation(
		m_spModel->GetAnimation(
			"Armature|Idle"
		)
	);

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
		std::dynamic_pointer_cast<CharacterBase>(shared_from_this())
	);

	//============================================================
	// NavMesh Manager
	//============================================================

	m_pNavMeshManager = &NavMeshManager::Instance();

	//============================================================
	// State
	//============================================================

	ApplyStateSettings();

	//============================================================
	// Inventory初期化
	//============================================================

	m_inventory.Clear();

	m_currentItemData = ItemData();

	//============================================================
	// 仮アイテム
	//============================================================

	AddInventoryItem("Asset/Items/Scissors.item");
	
	AddInventoryItem("Asset/Items/Hammer.item");

	SelectInventory(0);
}

//============================================================
// Inventory
//============================================================

void Neighbor::AddInventoryItem(const std::string& itemPath)
{
	if (itemPath.empty())
	{
		return;
	}

	if (!m_inventory.AddItem(itemPath))
	{
		return;
	}
}

//============================================================
// Select Inventory
//============================================================

void Neighbor::SelectInventory(int index)
{
	m_inventory.SelectIndex(index);

	const InventoryItem* item = m_inventory.GetCurrentItem();

	if (!item)
	{
		m_currentItemData = ItemData();

		return;
	}

	m_currentItemData.Load(item->itemPath);
}

//============================================================
// Inventory
//============================================================

bool Neighbor::HasInventoryItem(
	const std::string& itemPath) const
{
	if (itemPath.empty())
	{
		return false;
	}

	const auto& items =
		m_inventory.GetItems();

	for (int i = 0;
		i < Inventory::MaxItem;
		++i)
	{
		if (items[i].IsEmpty())
		{
			continue;
		}

		if (items[i].itemPath == itemPath)
		{
			return true;
		}
	}

	return false;
}

//============================================================
// AI
//============================================================

void Neighbor::UpdateBehavior()
{
	//============================================================
	// Utility AI
	//============================================================

	if (!m_isInteracting)
	{
		UpdateUtility();
	}

	//============================================================
	// FSM
	//============================================================

	switch (m_state)
	{
	case State::Task:

		UpdateTask();

		break;

	case State::SearchItem:

		UpdateSearchItem();

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
// Utility AI
//=================================H===========================

void Neighbor::UpdateUtility()
{
	const float deltaTime = 1.0f / 60.0f;

	m_utilityUpdateTimer += deltaTime;

	if (m_utilityUpdateTimer < m_utilityUpdateInterval)
	{
		return;
	}

	m_utilityUpdateTimer = 0.0f;

	//============================================================
	// Utility計算
	//============================================================

	m_taskUtility = EvaluateTask();

	m_searchItemUtility = EvaluateSearchItem();

	m_prankUtility = EvaluatePrank();

	m_repairUtility = EvaluateRepair();

	//============================================================
	// 一番Utilityの高い行動を選択
	//============================================================

	Action nextAction = DecideAction();

	m_currentAction = nextAction;
	char buffer[256];

	_snprintf_s(
		buffer,
		sizeof(buffer),
		_TRUNCATE,
		"Utility: Task=%.1f Search=%.1f Prank=%.1f Repair=%.1f\n",
		m_taskUtility,
		m_searchItemUtility,
		m_prankUtility,
		m_repairUtility
	);
	if (GetName() == "Neighbor_0")
	{
		OutputDebugStringA(buffer);
	}
	//============================================================
	// Utility → FSM
	//============================================================

	switch (nextAction)
	{
	case Action::Task:

		ChangeState(State::Task);

		break;

	case Action::SearchItem:
	{
		if (!m_bestSearchItemCandidate.item)
		{
			ChangeState(State::Task);

			return;
		}

		SelectSearchItemCandidate(m_bestSearchItemCandidate);

		ChangeState(State::SearchItem);

		break;
	}

	case Action::Prank:
	{
		//========================================================
		// EvaluatePrank()で選ばれた候補を使用
		//========================================================

		if (!m_bestPrankCandidate.target)
		{
			ChangeState(State::Task);

			return;
		}

		//========================================================
		// アイテムとターゲットを確定
		//========================================================

		SelectPrankCandidate(m_bestPrankCandidate);

		ChangeState(State::Prank);

		break;
	}

	case Action::Repair:
	{
		//========================================================
		// Repair候補がない
		//========================================================

		if (!m_bestRepairCandidate.target)
		{
			ChangeState(State::Task);

			return;
		}

		//========================================================
		// アイテムとターゲットを確定
		//========================================================

		SelectRepairCandidate(m_bestRepairCandidate);

		ChangeState(State::Repair);

		break;
	}
	}
}

//============================================================
// Decide Action
//============================================================

Neighbor::Action Neighbor::DecideAction()
{
	float highestUtility = m_taskUtility;

	Action bestAction = Action::Task;
	
	//------------------------------------------------------------
	// Search Item
	//------------------------------------------------------------

	if (m_searchItemUtility > highestUtility)
	{
		highestUtility = m_searchItemUtility;

		bestAction = Action::SearchItem;
	}

	//------------------------------------------------------------
	// Prank
	//------------------------------------------------------------

	if (m_prankUtility > highestUtility)
	{
		highestUtility = m_prankUtility;

		bestAction = Action::Prank;
	}

	//------------------------------------------------------------
	// Repair
	//------------------------------------------------------------

	if (m_repairUtility > highestUtility)
	{
		highestUtility = m_repairUtility;

		bestAction = Action::Repair;
	}

	return bestAction;
}

//============================================================
// Task Utility
//============================================================

float Neighbor::EvaluateTask()
{
	// 常に最低限の行動としてTaskを残す
	return 10.0f;
}

//============================================================
// Serach Item Utility
//============================================================

float Neighbor::EvaluateSearchItem()
{
	m_bestSearchItemCandidate = SearchItemCandidate();

	if (!FindBestSearchItem(
		m_bestSearchItemCandidate))
	{
		return 0.0f;
	}

	return m_bestSearchItemCandidate.utility;
}


//============================================================
// Prank Utility
//============================================================

float Neighbor::EvaluatePrank()
{
	//============================================================
	// 前回の候補をリセット
	//============================================================

	m_bestPrankCandidate = PrankCandidate();

	//============================================================
	// 最適なPrank候補を検索
	//============================================================

	if (!FindBestPrankCandidate(m_bestPrankCandidate))
	{
		return 0.0f;
	}

	//============================================================
	// 見つかった候補のUtilityを返す
	//============================================================

	return m_bestPrankCandidate.utility;
}

//============================================================
// Repair Utility
//============================================================

float Neighbor::EvaluateRepair()
{
	//============================================================
	// 前回の候補をリセット
	//============================================================

	m_bestRepairCandidate = RepairCandidate();

	//============================================================
	// 最適なRepair候補を検索
	//============================================================

	if (!FindBestRepairCandidate(m_bestRepairCandidate))
	{
		return 0.0f;
	}

	//============================================================
	// 見つかった候補のUtilityを返す
	//============================================================

	return m_bestRepairCandidate.utility;
}

//============================================================
// 
//============================================================

bool Neighbor::CalculatePathDistance(
	const Math::Vector3& goal,
	float& outDistance) const
{
	outDistance = 0.0f;

	if (!m_pNavMeshManager)
	{
		return false;
	}

	//============================================================
	// Utility計算専用のPathを取得
	// m_navPathは変更しない
	//============================================================

	const auto path =
		m_navMeshPathfinder.FindPathPoints(
			*m_pNavMeshManager,
			m_pos,
			goal
		);

	//============================================================
	// Pathが存在しない
	//============================================================

	if (path.empty())
	{
		return false;
	}

	//============================================================
	// Path距離を計算
	//============================================================

	float distance = 0.0f;

	for (size_t i = 1; i < path.size(); ++i)
	{
		Math::Vector3 diff =
			path[i] - path[i - 1];

		diff.y = 0.0f;

		distance += diff.Length();
	}

	outDistance = distance;

	return true;
}

//============================================================
// Task
//============================================================

void Neighbor::UpdateTask()
{
	//============================================================
	// 現在は通常行動のみ
	//============================================================

	// 今後、
	//
	// ・家へ帰る
	// ・公園へ行く
	// ・アイテムを探す
	// ・アイテムを拾う
	//
	// などをここに追加する
}

//============================================================
// Search Item
//============================================================

void Neighbor::UpdateSearchItem()
{
	auto item = m_wpSearchItem.lock();

	//============================================================
	// アイテムが消えた
	//============================================================

	if (!item)
	{
		m_wpSearchItem.reset();

		ChangeState(State::Task);

		return;
	}

	//============================================================
	// すでにインベントリに入っている
	//============================================================

	const ItemData& itemData = item->GetItemData();

	if (HasInventoryItem(itemData.GetItemPath()))
	{
		m_wpSearchItem.reset();

		ChangeState(State::Task);

		return;
	}

	//============================================================
	// アイテムまで移動
	//============================================================

	MoveResult moveResult = MoveToSearchItem();

	//============================================================
	// 移動中
	//============================================================

	if (moveResult == MoveResult::Moving)
	{
		m_spAnimator->SetAnimation(
			m_spModel->GetAnimation(
				"Armature|Sprint"
			)
		);

		return;
	}

	//============================================================
	// 移動失敗
	//============================================================

	if (moveResult == MoveResult::Failed)
	{
		m_wpSearchItem.reset();

		ChangeState(State::Task);

		return;
	}

	//============================================================
	// 到着
	//============================================================
	
	PickupSearchItem();
}

//============================================================
// Find Best Serach Item Candidate
//============================================================

bool Neighbor::FindBestSearchItem(SearchItemCandidate & outCandidate)
{
	outCandidate = SearchItemCandidate();

	//============================================================
	// インベントリが満杯なら探さない
	//============================================================

	if (!m_inventory.HasEmptySlot())
	{
		return false;
	}

	auto& objList = SceneManager::Instance().GetObjList();

	const float searchRadius = m_searchItemRadius * m_sceneScale;

	const float searchDistanceSq = searchRadius * searchRadius;

	float bestUtility = 0.0f;

	for (auto& obj : objList)
	{
		if (!obj)
		{
			continue;
		}

		auto item = std::dynamic_pointer_cast<Item>(obj);

		if (!item)
		{
			continue;
		}

		//========================================================
		// インベントリ内のItemは拾えない
		//========================================================

		if (item->GetMode() == ItemMode::InInventory)
		{
			continue;
		}

		//========================================================
		// 距離
		//========================================================

		Math::Vector3 diff = item->GetPos() - m_pos;

		diff.y = 0.0f;

		if (diff.LengthSquared() > searchDistanceSq)
		{
			continue;
		}

		//========================================================
		// すでにインベントリにあるアイテムなら不要
		//========================================================

		const ItemData& itemData = item->GetItemData();

		if (HasInventoryItem(itemData.GetItemPath()))
		{
			continue;
		}

		//========================================================
		// このアイテムを拾う価値
		//========================================================

		float utility = EvaluateSearchItemCandidate(item);

		if (utility <= bestUtility)
		{
			continue;
		}

		bestUtility = utility;

		outCandidate.item = item;

		outCandidate.utility = utility;
	}

	return outCandidate.item != nullptr;
}

//============================================================
// Evaluate Search Item Candidate
//============================================================

float Neighbor::EvaluateSearchItemCandidate(const std::shared_ptr<Item>& item)
{
	if (!item)
	{
		return 0.0f;
	}

	const ItemData& itemData = item->GetItemData();

	//============================================================
	// 使えないItem
	//============================================================

	if (itemData.GetAction() == ItemAction::None)
	{
		return 0.0f;
	}

	//============================================================
	// Itemまでの距離
	//============================================================

	Math::Vector3 itemDiff = item->GetPos() - m_pos;

	itemDiff.y = 0.0f;

	float itemDistance = itemDiff.Length();

	float searchRadius = m_searchItemRadius * m_sceneScale;

	float itemDistanceScore =
		1.0f -
		std::clamp(
			itemDistance / searchRadius,
			0.0f,
			1.0f
		);

	//============================================================
	// 基本Utility
	//============================================================

	float utility = 20.0f;

	utility += itemDistanceScore * 20.0f;

	//============================================================
	// Prankに使える
	//============================================================

	if (CanUseItemForPrank(itemData))
	{
		utility += 50.0f;
	}

	//============================================================
	// Repairに使える
	//============================================================

	if (CanUseItemForRepair(itemData))
	{
		utility += 40.0f;
	}

	return utility;
}

//============================================================
// Select Search Item Candidate
//============================================================

void Neighbor::SelectSearchItemCandidate(const SearchItemCandidate& candidate)
{
	if (!candidate.item)
	{
		return;
	}

	//============================================================
	// ターゲット保存
	//============================================================

	m_wpSearchItem = candidate.item;

	//============================================================
	// 古いPathを破棄
	//============================================================

	m_navPath.clear();

	m_navPathIndex = 0;
}

//============================================================
// Move To Search Item
//============================================================

MoveResult Neighbor::MoveToSearchItem()
{
	auto item = m_wpSearchItem.lock();

	//============================================================
	// アイテムが存在しない
	//============================================================

	if (!item)
	{
		return MoveResult::Failed;
	}

	Math::Vector3 toItem = item->GetPos() - m_pos;

	toItem.y = 0.0f;

	const float interactDistance = m_searchItemInteractDistance * m_sceneScale;

	//============================================================
	// 到着判定
	//============================================================

	if (toItem.LengthSquared() <= interactDistance * interactDistance)
	{
		m_navPath.clear();

		m_navPathIndex = 0;

		SetMoveDirection(Math::Vector3::Zero);

		return MoveResult::Arrived;
	}

	//============================================================
	// Pathがない
	//============================================================

	if (m_navPath.empty())
	{
		FindPathTo(item->GetPos());

		//========================================================
		// Path生成失敗
		//========================================================

		if (m_navPath.empty())
		{
			return MoveResult::Failed;
		}
	}

	//============================================================
	// NavMesh上を移動
	//============================================================

	return MoveToNextNavPathPoint();
}

void Neighbor::PickupSearchItem()
{
	auto item = m_wpSearchItem.lock();

	//============================================================
	// アイテムが存在しない
	//============================================================

	if (!item)
	{
		m_wpSearchItem.reset();

		ChangeState(State::Task);

		return;
	}

	//============================================================
	// 念のため、拾う直前にも確認
	//============================================================

	const ItemData& itemData = item->GetItemData();
	
	if (HasInventoryItem(itemData.GetItemPath()))
	{
		m_wpSearchItem.reset();

		ChangeState(State::Task);

		return;
	}

	//============================================================
	// ItemをInventoryに追加
	//============================================================

	if (!item->Pickup(m_inventory))
	{
		m_wpSearchItem.reset();

		ChangeState(State::Task);

		return;
	}

	//============================================================
	// アイテム取得成功
	//============================================================

	m_wpSearchItem.reset();

	//============================================================
	// 取得したアイテムを選択
	//============================================================

	const auto& items = m_inventory.GetItems();

	for (int i = 0;
		i < Inventory::MaxItem;
		++i)
	{
		if (items[i].itemPath == itemData.GetItemPath())
		{
			SelectInventory(i);
			break;
		}
	}

	//============================================================
	// 次の行動を決める
	//============================================================

	ChangeState(State::Task);
}

bool Neighbor::CanUseItemForPrank(const ItemData & itemData) const
{
/*	ただ、ここで一つ問題があります。

	今の

		CanUseItemForPrank()

		は、

		使えるターゲットが世界のどこかに一つでもある

		だけです。

		でも、

		ハサミ
		↓
		花
		↓
		100m先

		だったら、

		ハサミを拾う価値はあるのか？

		という問題があります。*/

/*そこで最終的には、

EvaluateSearchItemCandidate()

の中で、

Item
 ↓
対応するInteractiveObject
 ↓
ターゲットまでの距離
 ↓
Utility

まで見るのがおすすめです。*/

	auto& objList = SceneManager::Instance().GetObjList();

	for (auto& obj : objList)
	{
		auto target =
			std::dynamic_pointer_cast<InteractiveObject>(obj);

		if (!target)
		{
			continue;
		}

		if (target->GetOwner() == GetName())
		{
			continue;
		}

		if (target->CanNPCUse(itemData))
		{
			return true;
		}
	}

	return false;

}

bool Neighbor::CanUseItemForRepair(const ItemData& itemData) const
{
	auto& objList = SceneManager::Instance().GetObjList();

	for (auto& obj : objList)
	{
		auto target = 
			std::dynamic_pointer_cast<InteractiveObject>(obj);

		if (!target)
		{
			continue;
		}

		if (target->GetOwner() != GetName())
		{
			continue;
		}

		if (target->CanNPCUse(itemData))
		{
			return true;
		}
	}

	return false;
}

//============================================================
// Prank
//============================================================

void Neighbor::UpdatePrank()
{
	auto target = m_wpPrankTarget.lock();

	//============================================================
	// ターゲットが消えた
	//============================================================

	if (!target)
	{
		m_prankInventoryIndex = -1;

		ChangeState(State::Task);

		return;
	}

	const ItemData& itemData = GetCurrentItemData();

	//============================================================
	// インタラクト中
	//============================================================

	if (m_isInteracting)
	{
		m_interactTimer += 1.0f / 60.0f;

		float useTime = target->GetNPCUseTime(itemData);

		if (m_interactTimer >= useTime)
		{
			m_isInteracting = false;

			m_interactTimer = 0.0f;

			//====================================================
			// いたずら実行
			//====================================================

			target->NPCUse(itemData, std::dynamic_pointer_cast<Neighbor>(
				shared_from_this()));

			//====================================================
			// いたずら終了
			//====================================================

			m_wpPrankTarget.reset();

			m_prankInventoryIndex = -1;

			ChangeState(State::Task);
		}

		return;
	}

	//============================================================
	// ターゲットまで移動
	//============================================================

	MoveResult moveResult =
		MoveToPrankTarget();

	//============================================================
	// 移動中
	//============================================================

	if (moveResult ==
		MoveResult::Moving)
	{
		m_spAnimator->SetAnimation(
			m_spModel->GetAnimation(
				"Armature|Sprint"
			)
		);

		return;
	}

	//============================================================
	// 移動失敗
	//============================================================

	if (moveResult ==
		MoveResult::Failed)
	{
		m_wpPrankTarget.reset();

		m_prankInventoryIndex = -1;

		ChangeState(
			State::Task
		);

		return;
	}

	//============================================================
	// 到着したので、もう一度使用可能か確認
	//============================================================

	if (!target->CanNPCUse(
		itemData))
	{
		m_wpPrankTarget.reset();

		m_prankInventoryIndex = -1;


		ChangeState(
			State::Task
		);

		return;
	}

	//============================================================
	// 到着
	//============================================================

	SetMoveDirection(Math::Vector3::Zero);

	m_isInteracting = true;

	m_interactTimer = 0.0f;

	m_spAnimator->SetAnimation(
		m_spModel->GetAnimation(
			"Armature|Idle"
		)
	);
}

//============================================================
// Move To Prank Target
//============================================================

MoveResult Neighbor::MoveToPrankTarget()
{
	auto target = m_wpPrankTarget.lock();

	//============================================================
	// ターゲットが存在しない
	//============================================================

	if (!target)
	{
		return MoveResult::Failed;
	}

	Math::Vector3 toTarget = target->GetPos() - m_pos;

	toTarget.y = 0.0f;

	const float interactDistance = m_prankInteractDistance * m_sceneScale;

	//============================================================
	// 到着判定
	//============================================================

	if (toTarget.LengthSquared() <= interactDistance * interactDistance)
	{
		m_navPath.clear();

		m_navPathIndex = 0;

		return MoveResult::Arrived;
	}

	//============================================================
	// Pathがない
	//============================================================

	if (m_navPath.empty())
	{
		FindPathTo(target->GetPos());

		//========================================================
		// Path生成失敗
		//========================================================

		if (m_navPath.empty())
		{
			return MoveResult::Failed;
		}
	}

	//============================================================
	// NavMesh上を移動
	//============================================================

	return MoveToNextNavPathPoint();
}

//============================================================
// Find Best Prank Candidate
//============================================================

bool Neighbor::FindBestPrankCandidate(PrankCandidate& outCandidate)
{
	outCandidate = PrankCandidate();

	OutputDebugStringA(
		"=== FindBestPrankCandidate START ===\n"
	);
	//============================================================
	// Scene Object
	//============================================================

	auto& objList = SceneManager::Instance().GetObjList();

	//============================================================
	// Search Range
	//============================================================

	const float searchRadius = m_prankSearchRadius * m_sceneScale;

	const float searchDistanceSq = searchRadius * searchRadius;

	//============================================================
	// 現在のInventory
	//============================================================

	const auto& items = m_inventory.GetItems();

	float bestUtility = 0.0f;

	//============================================================
	// Inventory全スロット
	//============================================================

	for (int inventoryIndex = 0;
		inventoryIndex < Inventory::MaxItem;
		++inventoryIndex)
	{
		const InventoryItem& inventoryItem = items[inventoryIndex];

		//========================================================
		// 空きスロット
		//========================================================

		if (inventoryItem.IsEmpty())
		{
			OutputDebugStringA(
				"Prank: Inventory slot EMPTY\n"
			);
			continue;
		}

		//========================================================
		// ItemData
		//========================================================
		
		ItemData itemData;

		itemData.Load(inventoryItem.itemPath);
		{
			std::string log =
				"Prank: Inventory[" +
				std::to_string(inventoryIndex) +
				"] Path=[" +
				inventoryItem.itemPath +
				"] Action=" +
				std::to_string(
					static_cast<int>(
						itemData.GetAction()
						)
				) +
				" Target=[" +
				itemData.GetTarget() +
				"]\n";

			OutputDebugStringA(
				log.c_str()
			);
		}
		//========================================================
		// Repairは無視
		//========================================================

		if (itemData.GetAction() == ItemAction::Repair)
		{
			OutputDebugStringA(
				"Prank: SKIP - Repair item\n"
			);
			continue;
		}

		//========================================================
		// InteractiveObject全検索
		//========================================================

		for (auto& obj : objList)
		{
			if (!obj)
			{
				continue;
			}

			auto target = std::dynamic_pointer_cast<InteractiveObject>(obj);

			if (!target)
			{
				continue;
			}

			//====================================================
			// 自分の所有物にはいたずらしない
			//====================================================
			{
				std::string log =
					"Prank: Target Owner=[" +
					target->GetOwner() +
					"] NeighborOwner=[" +
					GetName() +
					"]\n";

				OutputDebugStringA(
					log.c_str()
				);
			}
			if (target->GetOwner() == GetName())
			{
				OutputDebugStringA(
					"Prank: SKIP - Same Owner\n"
				);
				continue;
			}

			//====================================================
			// このItemでいたずら可能か
			//====================================================
			const bool canNPCUse =
				target->CanNPCUse(itemData);

			{
				std::string log =
					"Prank: CanNPCUse=" +
					std::string(
						canNPCUse
						? "TRUE"
						: "FALSE"
					) +
					"\n";

				OutputDebugStringA(
					log.c_str()
				);
			}
			if (!target->CanNPCUse(itemData))
			{
				continue;
			}

			//====================================================
			// 距離
			//====================================================

			Math::Vector3 diff = target->GetPos() - m_pos;

			diff.y = 0.0f;
			const float distanceSq =
				diff.LengthSquared();

			{
				std::string log =
					"Prank: DistanceSq=" +
					std::to_string(distanceSq) +
					" SearchDistanceSq=" +
					std::to_string(searchDistanceSq) +
					"\n";

				OutputDebugStringA(
					log.c_str()
				);
			}
			if (diff.LengthSquared() > searchDistanceSq)
			{
				continue;
			}

			//============================================================
			// ここまで来た候補だけPathfinding
			//============================================================

			float pathDistance = 0.0f;

			const bool hasPath =
				CalculatePathDistance(
					target->GetPos(),
					pathDistance
				);

			//============================================================
			// 到達不能なら候補から除外
			//============================================================

			if (!hasPath)
			{
				continue;
			}

			//====================================================
			// Candidate Utility
			//====================================================

			float utility =
				EvaluatePrankCandidate(
					target,
					itemData,
					pathDistance
				);
			{
				std::string log =
					"Prank: Candidate Utility=" +
					std::to_string(utility) +
					"\n";

				OutputDebugStringA(
					log.c_str()
				);
			}
			//====================================================
			// より高いUtilityなら更新
			//====================================================

			if (utility <= bestUtility)
			{
				OutputDebugStringA(
					"Prank: SKIP - Utility not higher\n"
				);

				continue;
			}

			bestUtility = utility;

			outCandidate.target = target;

			outCandidate.inventoryIndex = inventoryIndex;

			outCandidate.utility = utility;
			OutputDebugStringA(
				"Prank: *** CANDIDATE SELECTED ***\n"
			);

			outCandidate.hasPath =
				hasPath;

			outCandidate.pathDistance =
				pathDistance;
		}
	}

	if (outCandidate.target)
	{
		OutputDebugStringA(
			"=== FindBestPrankCandidate: FOUND ===\n"
		);

		return true;
	}

	OutputDebugStringA(
		"=== FindBestPrankCandidate: NOT FOUND ===\n"
	);

	return false;


	return outCandidate.target != nullptr;
}

//============================================================
// Evaluate Prank Candidate
//============================================================

float Neighbor::EvaluatePrankCandidate(
	const std::shared_ptr<InteractiveObject>& target,
	const ItemData& itemData,
	float pathDistance)
{
	if (!target)
	{
		return 0.0f;
	}

	//============================================================
	// 基本Utility
	//============================================================

	float utility = 40.0f;

	//============================================================
	// いたずら欲求
	//============================================================

	utility += m_prankDesire * 40.0f;

	//============================================================
	// 距離
	//============================================================

	Math::Vector3 diff = target->GetPos() - m_pos;

	diff.y = 0.0f;

	float distance = diff.Length();

	float searchRadius = m_prankSearchRadius * m_sceneScale;

	float distanceScore =
		1.0f -
		std::clamp(
			distance / searchRadius,
			0.0f,
			1.0f
		);

	utility += distanceScore * 10.0f;

	float pathScore =
		1.0f -
		std::clamp(
			pathDistance / searchRadius,
			0.0f,
			1.0f
		);

	utility += pathScore * 30.0f;

	return utility;
}

//============================================================
// Select Prank Candidate
//============================================================

void Neighbor::SelectPrankCandidate(const PrankCandidate& candidate)
{
	if (!candidate.target)
	{
		return;
	}

	if (candidate.inventoryIndex < 0 ||
		candidate.inventoryIndex >=
		Inventory::MaxItem)
	{
		return;
	}

	//============================================================
	// アイテムを選択
	//============================================================

	SelectInventory(candidate.inventoryIndex);

	//============================================================
	// ターゲットを保存
	//============================================================

	m_wpPrankTarget = candidate.target;

	m_prankInventoryIndex = candidate.inventoryIndex;
}

//============================================================
// Repair
//============================================================

void Neighbor::UpdateRepair()
{
	auto target = m_wpRepairTarget.lock();

	//============================================================
	// ターゲットが消えた
	//============================================================

	if (!target)
	{
		m_repairInventoryIndex = -1;

		ChangeState(State::Task);

		return;
	}

	const ItemData& itemData =	GetCurrentItemData();

	//============================================================
	// インタラクト中
	//============================================================

	if (m_isInteracting)
	{
		m_interactTimer += 1.0f / 60.0f;

		float useTime = target->GetNPCUseTime(itemData);


		if (m_interactTimer >= useTime)
		{
			m_isInteracting = false;

			m_interactTimer = 0.0f;

			//====================================================
			// 修復実行
			//
			// RepairもInteractiveObjectから見ればUse
			//====================================================

			target->NPCUse(itemData, std::dynamic_pointer_cast<Neighbor>(
				shared_from_this()));

			//====================================================
			// 修復終了
			//====================================================

			m_wpRepairTarget.reset();

			m_repairInventoryIndex = -1;

			ChangeState(State::Task);
		}

		return;
	}

	//============================================================
	// ターゲットまで移動
	//============================================================

	MoveResult moveResult = MoveToRepairTarget();

	//============================================================
	// 移動中
	//============================================================

	if (moveResult == MoveResult::Moving)
	{
		m_spAnimator->SetAnimation(
			m_spModel->GetAnimation(
				"Armature|Walk"
			)
		);

		return;
	}

	//============================================================
	// 移動失敗
	//============================================================

	if (moveResult == MoveResult::Failed)
	{
		m_wpRepairTarget.reset();

		m_repairInventoryIndex = -1;

		ChangeState(State::Task);

		return;
	}

	//============================================================
	// 到着後、もう一度Use可能か確認
	//============================================================

	if (!target->CanNPCUse(itemData))
	{
		m_wpRepairTarget.reset();

		m_repairInventoryIndex = -1;

		ChangeState(State::Task);

		return;
	}

	//============================================================
	// 到着
	//============================================================

	SetMoveDirection(Math::Vector3::Zero);

	m_isInteracting = true;

	m_interactTimer = 0.0f;

	m_spAnimator->SetAnimation(
		m_spModel->GetAnimation(
			"Armature|Idle"
		)
	);
}

//============================================================
// Move To Repair Target
//============================================================

MoveResult Neighbor::MoveToRepairTarget()
{
	auto target = m_wpRepairTarget.lock();

	//============================================================
	// ターゲットが存在しない
	//============================================================

	if (!target)
	{
		return MoveResult::Failed;
	}

	Math::Vector3 toTarget = target->GetPos() - m_pos;

	toTarget.y = 0.0f;

	const float interactDistance = m_repairInteractDistance * m_sceneScale;

	//============================================================
	// 到着判定
	//============================================================

	if (toTarget.LengthSquared() <= interactDistance * interactDistance)
	{
		m_navPath.clear();

		m_navPathIndex = 0;

		return MoveResult::Arrived;
	}

	//============================================================
	// Path
	//============================================================

	if (m_navPath.empty())
	{
		FindPathTo(target->GetPos());

		//========================================================
		// Path生成失敗
		//========================================================

		if (m_navPath.empty())
		{
			return MoveResult::Failed;
		}
	}

	//============================================================
	// NavMesh上を移動
	//============================================================

	return MoveToNextNavPathPoint();
}

//============================================================
// Find Best Repair Candidate
//============================================================

bool Neighbor::FindBestRepairCandidate(
	RepairCandidate& outCandidate)
{
	outCandidate = RepairCandidate();

	//============================================================
	// Scene Object
	//============================================================

	auto& objList = SceneManager::Instance().GetObjList();

	//============================================================
	// Search Range
	//============================================================

	const float searchRadius = m_repairSearchRadius * m_sceneScale;

	const float searchDistanceSq = searchRadius * searchRadius;

	//============================================================
	// Inventory
	//============================================================

	const auto& items = m_inventory.GetItems();

	float bestUtility = 0.0f;

	//============================================================
	// Inventory全スロット
	//============================================================

	for (int inventoryIndex = 0;
		inventoryIndex < Inventory::MaxItem;
		++inventoryIndex)
	{
		const InventoryItem& inventoryItem = items[inventoryIndex];

		//========================================================
		// 空きスロット
		//========================================================

		if (inventoryItem.IsEmpty())
		{
			continue;
		}

		//========================================================
		// ItemData
		//========================================================

		ItemData itemData;

		itemData.Load(inventoryItem.itemPath);

		//========================================================
		// Repairアイテム以外は無視
		//========================================================

		if (itemData.GetAction() != ItemAction::Repair)
		{
			continue;
		}

		//========================================================
		// InteractiveObject全検索
		//========================================================

		for (auto& obj : objList)
		{
			if (!obj)
			{
				continue;
			}

			auto target = std::dynamic_pointer_cast<InteractiveObject>(obj);

			if (!target)
			{
				continue;
			}

			//====================================================
			// 自分の所有物のみ修復する
			//====================================================

			if (target->GetOwner() != GetName())
			{
				continue;
			}

			OutputDebugStringA(
				("Repair: Target Owner=[" +
					target->GetOwner() +
					"] NeighborOwner=[" +
					GetName() +
					"]\n").c_str()
			);
			//====================================================
			// このアイテムでUse可能か
			//====================================================

			if (!target->CanNPCUse(itemData))
			{
				continue;
			}
			OutputDebugStringA(
				("Repair: CanNPCUse=" +
					std::string(target->CanNPCUse(itemData) ? "TRUE" : "FALSE") +
					"\n").c_str()
			); OutputDebugStringA(
				("Repair: Item Path=[" +
					inventoryItem.itemPath +
					"] Action=" +
					std::to_string(
						static_cast<int>(itemData.GetAction())
					) +
					" Target=[" +
					itemData.GetTarget() +
					"]\n").c_str()
			);

			//====================================================
			// 距離
			//====================================================

			Math::Vector3 diff = target->GetPos() - m_pos;

			diff.y = 0.0f;

			if (diff.LengthSquared() > searchDistanceSq)
			{
				continue;
			}

			//====================================================
			// Candidate Utility
			//====================================================

			float utility =
				EvaluateRepairCandidate(
					target,
					itemData
				);

			//====================================================
			// より高いUtilityなら更新
			//====================================================

			if (utility <= bestUtility)
			{
				continue;
			}

			bestUtility = utility;

			outCandidate.target = target;

			outCandidate.inventoryIndex = inventoryIndex;

			outCandidate.utility = utility;
		}
	}

	return outCandidate.target != nullptr;
}

//============================================================
// Evaluate Repair Candidate
//============================================================

float Neighbor::EvaluateRepairCandidate(
	const std::shared_ptr<InteractiveObject>& target,
	const ItemData& itemData)
{
	if (!target)
	{
		return 0.0f;
	}

	//============================================================
	// 基本Utility
	//============================================================

	float utility = 50.0f;

	//============================================================
	// 距離
	//============================================================

	Math::Vector3 diff = target->GetPos() - m_pos;

	diff.y = 0.0f;

	float distance = diff.Length();

	float searchRadius = m_repairSearchRadius * m_sceneScale;

	float distanceScore =
		1.0f -
		std::clamp(
			distance / searchRadius,
			0.0f,
			1.0f
		);

	utility += distanceScore * 20.0f;

	return utility;
}

//============================================================
// Select Repair Candidate
//============================================================

void Neighbor::SelectRepairCandidate(
	const RepairCandidate& candidate)
{
	if (!candidate.target)
	{
		return;
	}

	if (candidate.inventoryIndex < 0 ||
		candidate.inventoryIndex >= Inventory::MaxItem)
	{
		return;
	}

	//============================================================
	// 修復アイテムを選択
	//============================================================

	SelectInventory(candidate.inventoryIndex);

	//============================================================
	// ターゲットを保存
	//============================================================

	m_wpRepairTarget = candidate.target;

	m_repairInventoryIndex = candidate.inventoryIndex;
}

//============================================================
// NavMesh
//============================================================

void Neighbor::FindPathTo(
	const Math::Vector3& goal)
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

//============================================================
// Move Next Nav Path Point
//============================================================

MoveResult Neighbor::MoveToNextNavPathPoint()
{
	//============================================================
	// Pathがない
	//============================================================

	if (m_navPath.empty())
	{
		return MoveResult::Failed;
	}

	//============================================================
	// Path終了
	//============================================================

	if (m_navPathIndex >= static_cast<int>(m_navPath.size()))
	{
		m_navPath.clear();

		m_navPathIndex = 0;

		return MoveResult::Arrived;
	}

	const Math::Vector3 targetPoint =
		m_navPath[m_navPathIndex];

	Math::Vector3 dir = targetPoint - m_pos;

	dir.y = 0.0f;

	const float arriveDistance = 0.2f * m_sceneScale;

	//============================================================
	// Waypoint到着
	//============================================================

	if (dir.LengthSquared() <= arriveDistance * arriveDistance)
	{
		m_navPathIndex++;

		//========================================================
		// まだ次のWaypointがある
		//========================================================

		if (m_navPathIndex <
			static_cast<int>(
				m_navPath.size()))
		{
			return MoveResult::Moving;
		}

		//========================================================
		// Path終了
		//========================================================

		m_navPath.clear();

		m_navPathIndex = 0;

		return MoveResult::Arrived;
	}

	//============================================================
	// 移動
	//============================================================

	SetMoveDirection(dir);

	return MoveResult::Moving;
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

	//============================================================
	// State変更時に移動・インタラクトをリセット
	//============================================================

	m_navPath.clear();

	m_navPathIndex = 0;

	m_isInteracting = false;

	m_interactTimer = 0.0f;

	ApplyStateSettings();
}

//============================================================
// Apply State Settings
//============================================================

void Neighbor::ApplyStateSettings()
{
	switch (m_state)
	{
	case State::Task:

		SetMoveSpeed(m_taskSpeed);

		m_spAnimator->SetAnimation(
			m_spModel->GetAnimation(
				"Armature|Idle"
			)
		);

		break;

	case State::SearchItem:

		SetMoveSpeed(m_searchItemSpeed);

		m_spAnimator->SetAnimation(
			m_spModel->GetAnimation(
				"Armature|Sprint"
			)
		);

		break;

	case State::Prank:

		SetMoveSpeed(m_prankSpeed);

		m_spAnimator->SetAnimation(
			m_spModel->GetAnimation(
				"Armature|Sprint"
			)
		);

		break;

	case State::Repair:

		SetMoveSpeed(m_repairSpeed);

		m_spAnimator->SetAnimation(
			m_spModel->GetAnimation(
				"Armature|Walk"
			)
		);

		break;
	}
}