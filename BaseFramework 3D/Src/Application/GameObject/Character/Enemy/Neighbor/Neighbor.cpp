#include "Neighbor.h"

#include "../../../../System/Navigation/NavMesh/NavMeshManager.h"
#include "../../../../System/Navigation/NavMesh/NavMeshPathfinder.h"

#include "../../../../System/Location/LocationPointManager.h"

#include "../../../../Scene/SceneManager.h"

#include "../../../../System/Score/ScoreManager.h"

#include "../../../Object/StaticMeshObject/Item/Item.h"
#include "../../../Object/StaticMeshObject/Item/ItemManager.h"

#include "../../../Object/StaticMeshObject/InteractiveObject/InteractiveObject.h"

#include "../../../../System/DayCycle/DayCycle.h"

#include "../../../../System/Damage/DamageDirector.h"

//============================================================
// Init
//============================================================

void Neighbor::Init()
{
	EnemyBase::Init();

	//============================================================
	// Model
	//============================================================

	m_spModel = std::make_shared<KdModelWork>();

	ReloadModel();

	//============================================================
	// Location Point Manager
	//============================================================

	m_pLocationPointManager = &LocationPointManager::Instance();

	//============================================================
	// State
	//============================================================

	ApplyStateSettings();

	//============================================================
	// Emote Icon
	//============================================================

	InitEmoteIcon();

	//============================================================
	// Inventory初期化
	//============================================================

	m_inventory.Clear();

	m_currentItemData = ItemData();

	m_prankCooldownTimer = KdRandom::GetFloat(5.0f, 12.0f);

	//============================================================
	// Utility
	//============================================================
	
	m_utilityUpdateTimer = KdRandom::GetFloat(0.0f, kUtilityUpdateInterval);
}

//============================================================
// Model
//============================================================

void Neighbor::ReloadModel()
{
	m_spModel->SetModelData(m_modelPath);

	m_spAnimator = std::make_shared<KdAnimator>();
	m_spAnimator->SetAnimation(m_spModel->GetAnimation("Armature|Idle"));

	UpdateAnimation();

	m_pCollider = std::make_unique<KdCollider>();
	m_pCollider->RegisterCollisionShape(
		"Neighbor",
		m_spModel,
		KdCollider::TypeEditor
	);
}

//============================================================
// Inventory
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

	const ItemData* d = ItemData::Get(item->itemPath);

	m_currentItemData = d ? *d : ItemData();
}

void Neighbor::OnItemsConfiscated()
{
	if (!m_inventory.GetCurrentItem())
	{
		SelectInventory(-1);
	}
}

//============================================================
// AI
//============================================================

void Neighbor::UpdateBehavior()
{
	UpdateTimer();

	//============================================================
	// 不要アイテムを削除
	//============================================================

	if (m_state != State::Prank)
	{
		DropUselessItems();
	}

	//============================================================
	// 確保中は行動しない
	//============================================================

	if (m_isArrested)
	{
		return;
	}

	//============================================================
	// 拘束中：移動・思考・インタラクト進行を止める（進捗は保持）
	//============================================================

	if (m_isRestrained)
	{
		SetMoveDirection(Math::Vector3::Zero);
		PlayAnimation("Armature|Idle");
		return;
	}

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
	}
}

void Neighbor::UpdateTimer()
{
	if (m_prankCooldownTimer > 0.0f)
	{
		m_prankCooldownTimer -= 1.0f / 60.0f;
	}
}

//============================================================
// Utility AI
//============================================================

void Neighbor::UpdateUtility()
{
	const float deltaTime = 1.0f / 60.0f;

	m_utilityUpdateTimer += deltaTime;

	if (m_utilityUpdateTimer < kUtilityUpdateInterval)
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

	//============================================================
	// 上限数リセット
	//============================================================

	if (m_prankUtility <= 0.0f && m_state != State::Prank)
	{
		m_prankChainCount = 0;
	}

	//============================================================
	// 一番Utilityの高い行動を選択
	//============================================================

	const Action nextAction = DecideAction();

	m_currentAction = nextAction;

	m_debugInfo.actionName = ActionToString(m_currentAction);

	//============================================================
	// ImGui用に保存
	//============================================================

	m_debugInfo.taskUtility = m_taskUtility;
	m_debugInfo.searchItemUtility = m_searchItemUtility;
	m_debugInfo.prankUtility = m_prankUtility;

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
		if (!m_bestPrankCandidate.target ||
			!SelectPrankCandidate(m_bestPrankCandidate))
		{
			ChangeState(State::Task);
			return;
		}

		ChangeState(State::Prank);
		break;
	}
	}
}

//============================================================
// Decide Action
//============================================================

Neighbor::Action Neighbor::DecideAction() const
{
	float highest = m_taskUtility;
	Action best = Action::Task;

	if (m_searchItemUtility > highest)
	{
		highest = m_searchItemUtility;
		best = Action::SearchItem;
	}

	if (m_prankUtility > highest)
	{
		highest = m_prankUtility;
		best = Action::Prank;
	}

	return best;
}

//============================================================
// Task Utility
//============================================================

float Neighbor::EvaluateTask() const
{
	// 常に最低限返す
	return kTaskBaseUtility;
}

//============================================================
// Serach Item Utility
//============================================================

float Neighbor::EvaluateSearchItem()
{
	m_bestSearchItemCandidate = SearchItemCandidate();

	if (!FindBestSearchItem(m_bestSearchItemCandidate))
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
	// 休憩と上限
	//============================================================

	if (m_prankCooldownTimer > 0.0f)
	{
		return 0.0f;
	}

	if (m_state != State::Prank && !DamageDirector::CanDamageMore())
	{
		return 0.0f;
	}

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
// Prank用アプローチ地点
// 対象に一番近いNavMesh上の点を、半径を広げながら探す
//============================================================

bool Neighbor::FindPrankApproachPos(
	const Math::Vector3& targetPos,
	Math::Vector3& outApproachPos) const
{
	const auto& navMeshManager = NavMeshManager::Instance();

	// すでにNavMesh上ならそのまま使う
	if (navMeshManager.FindPolygonFromPoint(targetPos))
	{
		outApproachPos = targetPos;
		return true;
	}

	// ちいさいほうから順に試す
	const float radii[] =
	{
		kNavSnapRadius,
		kPrankApproachMaxDistance
	};

	for (const float radius : radii)
	{
		const Math::Vector3 point = navMeshManager.FindNearestPoint(targetPos, radius * m_sceneScale);

		// FindNearestPoint の戻り値は信用せず、ポリゴンが取れたかで判定
		if (navMeshManager.FindPolygonFromPoint(point))
		{
			outApproachPos = point;
			return true;
		}
	}

	return false;
}
//============================================================
// Utility用 path距離計算
//============================================================

bool Neighbor::CalculatePathDistance(
	const Math::Vector3& goal,
	float& outDistance) const
{
	outDistance = 0.0f;

	//============================================================
	// Utility計算専用のPathを取得
	// 始点・終点を補正
	//============================================================

	const Math::Vector3 fixedStart = SnapToNavMesh(m_pos);

	const Math::Vector3 fixedGoal = SnapToNavMesh(goal);

	const auto path =
		m_navMeshPathfinder->FindPathPoints(
			NavMeshManager::Instance(),
			fixedStart,
			fixedGoal
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
		Math::Vector3 diff = path[i] - path[i - 1];

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
	// 到着後の待機中
	//============================================================

	if (m_isTaskIdling)
	{
		m_taskIdleTimer -= 1.0f / 60.0f;

		m_debugInfo.taskMoveStatus = "Waiting";

		SetMoveDirection(Math::Vector3::Zero);

		PlayAnimation("Armature|Idle");

		if (m_taskIdleTimer <= 0.0f)
		{
			m_isTaskIdling = false;
		}

		return;
	}

	auto destPoint = m_wpTaskDestination.lock();

	if (!destPoint)
	{
		destPoint = SelectTaskDestination();

		if (!destPoint)
		{
			m_debugInfo.hasTaskDestination = false;
			m_debugInfo.taskDestinationName = "None";
			m_debugInfo.taskMoveStatus = "Idle";
			return;
		}

		m_wpTaskDestination = destPoint;

		ClearNavPath();

		m_debugInfo.taskDestinationName = LocationTagToString(destPoint->GetTag());
	}

	//============================================================
	// Debug情報更新
	//============================================================

	Math::Vector3 diff = destPoint->GetPos() - m_pos;

	diff.y = 0.0f;

	m_debugInfo.taskDestinationDistance = diff.Length();

	//============================================================
	// 移動処理
	//============================================================

	const MoveResult moveResult = MoveToTaskDestination(destPoint);

	if (moveResult == MoveResult::Moving)
	{
		m_debugInfo.taskMoveStatus = "Moving";

		PlayAnimation("Armature|Sprint");

		return;
	}

	if (moveResult == MoveResult::Failed)
	{
		m_debugInfo.taskMoveStatus = "Failed";
	}
	else
	{
		m_debugInfo.taskMoveStatus = "Arrived";

		m_isTaskIdling = true;
		m_taskIdleTimer = kTaskIdleMin + KdRandom::GetFloat(0.0f, 1.0f) * (kTaskIdleMax - kTaskIdleMin);
	}

	m_wpTaskDestination.reset();

	ClearNavPath();

	SetMoveDirection(Math::Vector3::Zero);

	PlayAnimation("Armature|Idle");
}

MoveResult Neighbor::MoveToTaskDestination(const std::shared_ptr<LocationPoint>& destPoint)
{
	if (!destPoint)
	{
		return MoveResult::Failed;
	}

	Math::Vector3 toDest = destPoint->GetPos() - m_pos;

	toDest.y = 0.0f;

	const float arriveDistance = kTaskArriveDistance * m_sceneScale;

	if (toDest.LengthSquared() <= arriveDistance * arriveDistance)
	{
		ClearNavPath();

		return MoveResult::Arrived;
	}

	if (m_navPath.empty())
	{
		SetNavPathTo(destPoint->GetPos());

		if (m_navPath.empty())
		{
			return MoveResult::Failed;
		}
	}

	return FollowNavPath();
}

//============================================================
// Task目的地の選定
//============================================================

std::shared_ptr<LocationPoint> Neighbor::SelectTaskDestination()
{
	// 1. インベントリMaxかつクールダウン中でない
	if (!m_inventory.HasEmptySlot() && m_prankCooldownTimer <= 0.0f)
	{
		if (const auto p = SelectHousePoint())
		{
			return p;
		}
	}

	// 2. 空きがある
	if (m_inventory.HasEmptySlot())
	{
		if (const auto p = SelectItemAreaPoint())
		{
			return p;
		}
	}

	// 3. ランダム
	return SelectRandomPoint();
}

//============================================================
// 家（プレイヤーの家・隣人の家3軒）から1つ選択
//============================================================

std::shared_ptr<LocationPoint> Neighbor::SelectHousePoint() const
{
	std::vector<std::shared_ptr<LocationPoint>> candidates;

	static const LocationTag houseTags[] =
	{
		LocationTag::PlayerHouse,
		LocationTag::NeighborHouse1,
		LocationTag::NeighborHouse2,
		LocationTag::NeighborHouse3,
	};

	for (const auto tag : houseTags)
	{
		const auto points = m_pLocationPointManager->FindAllByTag(tag);

		candidates.insert(candidates.end(), points.begin(), points.end());
	}

	if (candidates.empty())
	{
		return nullptr;
	}

	const int index = KdRandom::GetInt(0, static_cast<int>(candidates.size()) - 1);

	return candidates[index];
}

//============================================================
// アイテムエリア（公園・工事現場・ゴミ捨て場）から1つ選択
//============================================================

std::shared_ptr<LocationPoint> Neighbor::SelectItemAreaPoint() const
{
	std::vector<std::shared_ptr<LocationPoint>> candidates;

	static const LocationTag itemAreaTags[] =
	{
		LocationTag::Park,
		LocationTag::ConstructionSite,
		LocationTag::TrashDump,
	};

	for (const auto tag : itemAreaTags)
	{
		const auto points = m_pLocationPointManager->FindAllByTag(tag);

		candidates.insert(
			candidates.end(),
			points.begin(),
			points.end()
		);
	}

	if (candidates.empty())
	{
		return nullptr;
	}

	const int index = KdRandom::GetInt(0, static_cast<int>(candidates.size()) - 1);

	return candidates[index];
}

//============================================================
// 全地点からランダムに1つ選択（純粋な徘徊）
//============================================================

std::shared_ptr<LocationPoint> Neighbor::SelectRandomPoint() const
{
	std::vector<std::shared_ptr<LocationPoint>> candidates;

	for (const auto& p : m_pLocationPointManager->GetPoints())
	{
		if (!p || p->GetTag() == LocationTag::DeliveryPoint) continue;
		candidates.push_back(p);
	}

	if (candidates.empty()) return nullptr;

	return candidates[KdRandom::GetInt(0, static_cast<int>(candidates.size()) - 1)];
}

//============================================================
// Search Item
//============================================================

void Neighbor::UpdateSearchItem()
{
	const auto item = m_wpSearchItem.lock();

	//============================================================
	// アイテムが消えた
	//============================================================
	
	if (!item || item->IsExpired())
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

	const MoveResult moveResult = MoveToSearchItem();

	//============================================================
	// 移動中
	//============================================================

	if (moveResult == MoveResult::Moving)
	{
		PlayAnimation("Armature|Sprint");

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

bool Neighbor::FindBestSearchItem(SearchItemCandidate& outCandidate)
{
	outCandidate = SearchItemCandidate();

	if (!m_inventory.HasEmptySlot())
	{
		return false;
	}

	const auto& objList = SceneManager::Instance().GetObjList();

	const float searchRadius = kSearchItemRadius * m_sceneScale;

	const float searchDistanceSq = searchRadius * searchRadius;

	float bestUtility = 0.0f;

	for (const auto& obj : objList)
	{
		if (!obj)
		{
			continue;
		}

		const auto item = std::dynamic_pointer_cast<Item>(obj);

		if (!item || item->IsExpired()) continue;

		if (item->GetMode() == ItemMode::InInventory)
		{
			continue;
		}

		//========================================================
		// 距離
		//========================================================

		Math::Vector3 diff = item->GetPos() - m_pos;

		diff.y = 0.0f;

		const bool isCurrentTarget = (m_state == State::SearchItem && m_wpSearchItem.lock() == item);

		if (!isCurrentTarget && diff.LengthSquared() > searchDistanceSq)
		{
			continue;
		}

		const ItemData& itemData = item->GetItemData();

		if (!itemData.HasPrankUse()) continue;
		if (!CanUseItemForPrank(itemData)) continue;

		//========================================================
		// すでにインベントリにあるアイテムなら不要
		//========================================================

		if (HasInventoryItem(itemData.GetItemPath()))
		{
			continue;
		}

		//========================================================
		// 経路探索
		//========================================================

		float pathDistance = 0.0f;
		if (!CalculatePathDistance(item->GetPos(), pathDistance))
		{
			continue; // 到達不能は除外
		}

		//========================================================
		// このアイテムを拾う価値
		//========================================================

		const float utility = EvaluateSearchItemCandidate(item, pathDistance);

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

float Neighbor::EvaluateSearchItemCandidate(const std::shared_ptr<Item>& item, float pathDistance) const
{
	if (!item)
	{
		return 0.0f;
	}

	const ItemData& itemData = item->GetItemData();

	//============================================================
	// 使えないItem
	//============================================================

	if (itemData.GetUses().empty())
	{
		return 0.0f;
	}

	//============================================================
	// Itemまでの距離
	//============================================================

	Math::Vector3 itemDiff = item->GetPos() - m_pos;

	itemDiff.y = 0.0f;

	const float searchRadius = kSearchItemRadius * m_sceneScale;

	const float pathScore = 1.0f - std::clamp(pathDistance / (searchRadius * 2.0f), 0.0f, 1.0f);

	//============================================================
	// 基本Utility
	//============================================================

	float utility = 20.0f;

	utility += pathScore * 20.0f;

	//============================================================
	// ヒステリシス
	//============================================================

	if (m_state == State::SearchItem && m_wpSearchItem.lock() == item)
	{
		utility += kSearchCommitBonus;
	}

	//============================================================
	// Prankに使える
	//============================================================

	utility += 50.0f;
	
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
	// 同じターゲットなら経路を維持
	//============================================================

	if (m_state == State::SearchItem && m_wpSearchItem.lock() == candidate.item)
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

	ClearNavPath();
}

//============================================================
// Move To Search Item
//============================================================

MoveResult Neighbor::MoveToSearchItem()
{
	const auto item = m_wpSearchItem.lock();

	//============================================================
	// アイテムが存在しない
	//============================================================
	
	if (!item || item->IsExpired())
	{
		return MoveResult::Failed;
	}

	Math::Vector3 toItem = item->GetPos() - m_pos;

	toItem.y = 0.0f;

	const float interactDistance = kSearchItemInteractDistance * m_sceneScale;

	//============================================================
	// 到着判定
	//============================================================

	if (toItem.LengthSquared() <= interactDistance * interactDistance)
	{
		ClearNavPath();

		SetMoveDirection(Math::Vector3::Zero);

		return MoveResult::Arrived;
	}

	//============================================================
	// Pathがない
	//============================================================

	if (m_navPath.empty())
	{
		SetNavPathTo(item->GetPos());

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

	return FollowNavPath();
}

void Neighbor::PickupSearchItem()
{
	const auto item = m_wpSearchItem.lock();

	//============================================================
	// アイテムが存在しない
	//============================================================

	if (!item || item->IsExpired())
	{
		m_wpSearchItem.reset();

		ChangeState(State::Task);

		return;
	}

	//============================================================
	// 念のため、拾う直前にも確認
	//============================================================

	const std::string newItemPath = item->GetItemData().GetItemPath();

	if (HasInventoryItem(newItemPath))
	{
		m_wpSearchItem.reset();
		ChangeState(State::Task);
		return;
	}

	//============================================================
	// 拾う（空きがあれば通常、満杯なら交換）
	//============================================================

	bool pickedUp = false;

	if (m_inventory.HasEmptySlot())
	{
		pickedUp = item->Pickup(m_inventory);
	}
	
	m_wpSearchItem.reset();

	if (!pickedUp)
	{
		ChangeState(State::Task);
		return;
	}

	//============================================================
	// 拾った/交換したアイテムを選択
	//============================================================

	const auto& items = m_inventory.GetItems();

	for (int i = 0; i < Inventory::MaxItem; ++i)
	{
		if (items[i].itemPath == newItemPath)
		{
			SelectInventory(i);
			break;
		}
	}

	ShowEmote(EmoteIcon::IconType::Stars, 2.0f);

	ChangeState(State::Task);
}

bool Neighbor::CanUseItemForPrank(const ItemData& itemData) const
{
	const auto& objList = SceneManager::Instance().GetObjList();

	for (const auto& obj : objList)
	{
		const auto target = std::dynamic_pointer_cast<InteractiveObject>(obj);

		if (!target)
		{
			continue;
		}

		if (target->CanNPCUse(itemData, GetName()))
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
	const auto target = m_wpPrankTarget.lock();

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
	const bool bare = IsBareHanded();

	const auto canUse = [&]() {return bare ? target->CanNPCUseBare(GetName()) : target->CanNPCUse(itemData, GetName());};

	//============================================================
	// インタラクト中
	//============================================================

	if (m_isInteracting)
	{
		m_interactTimer += 1.0f / 60.0f;

		const float useTime = bare ? target->GetNPCUseTimeBare(GetName()) : target->GetNPCUseTime(itemData, GetName());

		if (m_interactTimer >= useTime)
		{
			m_isInteracting = false;

			m_interactTimer = 0.0f;

			//====================================================
			// いたずら実行
			//====================================================

			auto self = std::dynamic_pointer_cast<Neighbor>(shared_from_this());
			const int usedIndex = m_inventory.GetSelectIndex();

			if (bare) target->NPCUseBare(self);
			else target->NPCUse(itemData, self);

			if (!bare && usedIndex >= 0)
			{
				m_inventory.RemoveItem(usedIndex);
				SelectInventory(-1);
			}

			//====================================================
			// 連続して行う
			//====================================================

			++m_prankChainCount;

			const bool chain = m_prankChainCount < kMaxPrankChain && KdRandom::GetFloat(0.0f, 1.0f) < kPrankChainChance;

			if (chain)
			{
				// 再捜索
				m_prankCooldownTimer = 0.0f;
				m_utilityUpdateTimer = kUtilityUpdateInterval;
			}
			else
			{
				// 終了
				m_prankChainCount = 0;
				StartPrankCooldown();
			}

			ShowEmote(EmoteIcon::IconType::Laugh, 2.0f);

			//====================================================
			// いたずら終了
			//====================================================

			ReleasePrankTarget();

			ChangeState(State::Task);
		}

		return;
	}

	//============================================================
	// ターゲットまで移動
	//============================================================

	const MoveResult moveResult = MoveToPrankTarget();

	//============================================================
	// 移動中
	//============================================================

	if (moveResult == MoveResult::Moving)
	{
		PlayAnimation("Armature|Sprint");
		return;
	}

	//============================================================
	// 移動失敗
	//============================================================

	if (moveResult == MoveResult::Failed)
	{
		m_wpPrankTarget.reset();

		m_prankInventoryIndex = -1;

		ChangeState(State::Task);

		return;
	}

	//============================================================
	// 到着したので、もう一度使用可能か確認
	//============================================================

	if (!canUse())
	{
		m_wpPrankTarget.reset();

		m_prankInventoryIndex = -1;

		ChangeState(State::Task);

		return;
	}

	//============================================================
	// 到着
	//============================================================

	SetMoveDirection(Math::Vector3::Zero);

	m_isInteracting = true;

	m_interactTimer = 0.0f;

	PlayAnimation("Armature|Idle");
}

//============================================================
// Move To Prank Target
//============================================================

MoveResult Neighbor::MoveToPrankTarget()
{
	const auto target = m_wpPrankTarget.lock();

	//============================================================
	// ターゲットが存在しない
	//============================================================

	if (!target)
	{
		return MoveResult::Failed;
	}

	Math::Vector3 toTarget = target->GetPos() - m_pos;

	toTarget.y = 0.0f;

	const float interactDistance = kPrankInteractDistance * m_sceneScale;

	//============================================================
	// 到着判定
	//============================================================

	if (toTarget.LengthSquared() <= interactDistance * interactDistance)
	{
		ClearNavPath();

		return MoveResult::Arrived;
	}

	//============================================================
	// Pathがない
	//============================================================

	if (m_navPath.empty())
	{
		SetNavPathTo(m_prankApproachPos);

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

	return FollowNavPath();
}

//============================================================
// Find Best Prank Candidate
//============================================================

bool Neighbor::FindBestPrankCandidate(PrankCandidate& outCandidate)
{
	outCandidate = PrankCandidate();

	m_debugInfo.prankCandidateLog.clear();

	//============================================================
	// Scene Object
	//============================================================

	const auto& objList = SceneManager::Instance().GetObjList();

	//============================================================
	// Search Range
	//============================================================

	const float searchRadius = kPrankSearchRadius * m_sceneScale;

	const float searchDistanceSq = searchRadius * searchRadius;

	//============================================================
	// 現在のInventory
	//============================================================

	const auto& items = m_inventory.GetItems();

	float bestUtility = 0.0f;
	int bestLogIndex = -1;

	//============================================================
	// Inventory全スロット
	//============================================================

	for (int inventoryIndex = -1; inventoryIndex < Inventory::MaxItem; ++inventoryIndex)
	{
		const bool bare = (inventoryIndex < 0);

		static const ItemData kEmpty;
		const ItemData* itemData = &kEmpty;

		std::string itemPath = "(bare hands)";

		if (!bare)
		{
			const InventoryItem& inventoryItem = items[inventoryIndex];

			if (inventoryItem.IsEmpty()) { continue; }

			itemData = ItemData::Get(inventoryItem.itemPath);

			if (!itemData || !itemData->HasPrankUse()) continue;

			itemPath = inventoryItem.itemPath;
		}

		//========================================================
		// InteractiveObject全検索
		//========================================================

		for (const auto& obj : objList)
		{
			if (!obj)
			{
				continue;
			}

			const auto target = std::dynamic_pointer_cast<InteractiveObject>(obj);

			if (!target)
			{
				continue;
			}

			//====================================================
			// Debug Log
			//====================================================

			DebugInfo::PrankLog log;
			log.itemPath = itemPath;
			log.targetName = target->GetName();
			log.utility = 0.0f;
			log.canNPCUse = false;
			log.selected = false;

			//====================================================
			// このItemでいたずら可能か
			//====================================================

			const bool canNPCUse = bare
				? target->CanNPCUseBare(GetName())
				: target->CanNPCUse(*itemData, GetName());

			log.canNPCUse = canNPCUse;

			if (!canNPCUse)
			{
				m_debugInfo.prankCandidateLog.push_back(log);
				continue;
			}

			if (!target->CanBeTargetedBy(this))
			{
				m_debugInfo.prankCandidateLog.push_back(log);
				continue;
			}

			//====================================================
			// 距離
			//====================================================

			Math::Vector3 diff = target->GetPos() - m_pos;

			diff.y = 0.0f;

			// 対象は除外しない
			const bool isCurrentTarget = (m_state == State::Prank && m_wpPrankTarget.lock() == target);

			if (!isCurrentTarget && diff.LengthSquared() > searchDistanceSq)
			{
				m_debugInfo.prankCandidateLog.push_back(log);
				continue;
			}

			//============================================================
			// アプローチ地点を求める（取れなければ候補から除外）
			//============================================================

			Math::Vector3 approachPos = Math::Vector3::Zero;

			if (!FindPrankApproachPos(target->GetPos(), approachPos))
			{
				m_debugInfo.prankCandidateLog.push_back(log);
				continue;
			}

			float pathDistance = 0.0f;

			const bool hasPath = CalculatePathDistance(approachPos, pathDistance);

			//============================================================
			// 到達不能なら候補から除外
			//============================================================

			if (!hasPath)
			{
				m_debugInfo.prankCandidateLog.push_back(log);
				continue;
			}

			//====================================================
			// Candidate Utility
			//====================================================

			float utility = EvaluatePrankCandidate(target, *itemData, pathDistance);

			utility += KdRandom::GetFloat(0.0f, 5.0f);

			log.utility = utility;

			m_debugInfo.prankCandidateLog.push_back(log);

			//====================================================
			// より高いUtilityなら更新
			//====================================================

			if (utility <= bestUtility)
			{
				continue;
			}

			bestUtility = utility;

			bestLogIndex = static_cast<int>(m_debugInfo.prankCandidateLog.size()) - 1;

			outCandidate.target = target;

			outCandidate.inventoryIndex = inventoryIndex;

			outCandidate.hasPath = hasPath;

			outCandidate.pathDistance = pathDistance;

			outCandidate.approachPos = approachPos;

			outCandidate.utility = utility;
		}
	}

	//====================================================
	// 一番Utilityが高かった候補にだけselectedフラグを立て直す
	//====================================================

	if (bestLogIndex >= 0)
	{
		m_debugInfo.prankCandidateLog[bestLogIndex].selected = true;
	}

	m_debugInfo.bestPrankTarget =
		outCandidate.target
		? outCandidate.target->GetName()
		: "None";

	return outCandidate.target != nullptr;
}

//============================================================
// Evaluate Prank Candidate
//============================================================

float Neighbor::EvaluatePrankCandidate(
	const std::shared_ptr<InteractiveObject>& target,
	const ItemData& itemData,
	float pathDistance) const
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

	utility += kPrankDesire * 40.0f;

	//============================================================
	// ヒステリシス
	//============================================================

	if (m_state == State::Prank && m_wpPrankTarget.lock() == target)
	{
		utility += kPrankCommitBonus;
	}

	//============================================================
	// 距離
	//============================================================

	const float searchRadius = kPrankSearchRadius * m_sceneScale;

	const float pathScore = 1.0f - std::clamp(pathDistance / searchRadius, 0.0f, 1.0f);

	utility += pathScore * 40.0f;

	return utility;
}

//============================================================
// Select Prank Candidate
//============================================================

bool Neighbor::SelectPrankCandidate(const PrankCandidate& candidate)
{
	if (!candidate.target) return false;

	if (candidate.inventoryIndex < -1 || candidate.inventoryIndex >= Inventory::MaxItem) return false;

	const auto old = m_wpPrankTarget.lock();

	if (old != candidate.target)
	{
		if (old)
		{
			old->ReleaseReservation(this);
		}

		// 評価から最大1秒ずれるので、予約に失敗することがある
		if (!candidate.target->TryReserve(std::dynamic_pointer_cast<Neighbor>(shared_from_this())))
		{
			return false;
		}

		ClearNavPath();
	}

	SelectInventory(candidate.inventoryIndex);

	m_wpPrankTarget = candidate.target;
	m_prankInventoryIndex = candidate.inventoryIndex;
	m_prankApproachPos = candidate.approachPos;

	return true;
}

void Neighbor::ReleasePrankTarget()
{
	if (const auto target = m_wpPrankTarget.lock())
	{
		target->ReleaseReservation(this);
	}

	m_wpPrankTarget.reset();
	m_prankInventoryIndex = -1;
	m_prankApproachPos = Math::Vector3::Zero;
}

void Neighbor::StartPrankCooldown()
{
	const float progress = DayCycle::Instance().GetTimeProgress();

	const float base = std::lerp(kPrankCooldownMax, kPrankCooldownMin, progress);

	m_prankCooldownTimer = base * KdRandom::GetFloat(0.7f, 1.3f);
}

void Neighbor::DropUselessItems()
{
	const auto objects = SceneManager::Instance().FindObjects<InteractiveObject>();

	const auto& items = m_inventory.GetItems();

	for (int i = 0; i < Inventory::MaxItem; ++i)
	{
		if (items[i].IsEmpty()) continue;

		const ItemData* data = ItemData::Get(items[i].itemPath);
		if (!data) continue;

		bool useful = false;

		for (const auto& obj : objects)
		{
			if (obj->CanEverBeBrokenBy(*data))
			{
				useful = true;
				break;
			}
		}

		if (useful) continue;

		const bool selected = (m_inventory.GetSelectIndex() == i);

		m_inventory.RemoveItem(i);

		if (selected)
		{
			SelectInventory(-1);
		}
	}
}

//============================================================
// Arrested
//============================================================

Math::Vector3 Neighbor::GetDamagePos() const
{
	const auto t = m_wpPrankTarget.lock();
	return t ? t->GetPos() : m_pos;
}

//============================================================
// Arrested
//============================================================

void Neighbor::OnArrested(const std::shared_ptr<Police>& police)
{
	CharacterBase::OnArrested(police);

	//============================================================
	// 進行中の行動を全部中断
	//============================================================

	m_wpSearchItem.reset();

	ReleasePrankTarget();

	m_isInteracting = false;
	m_interactTimer = 0.0f;

	m_prankChainCount = 0;

	m_isRestrained = false;

	ClearNavPath();

	SetMoveDirection(Math::Vector3::Zero);
	SetMoveSpeed(0.0f);

	PlayAnimation("Armature|Idle");
}

void Neighbor::OnReleased()
{
	CharacterBase::OnReleased();

	//============================================================
	// 解放直後は強制的にTaskから再評価させる
	//============================================================

	m_state = State::Task;

	// 次のUpdateUtilityですぐ評価し直すようタイマーをリセット
	m_utilityUpdateTimer = kUtilityUpdateInterval;

	ApplyStateSettings();
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

	if (m_state == State::Prank && newState != State::Prank)
	{
		ReleasePrankTarget();
	}

	m_state = newState;

	//============================================================
	// State変更時に移動・インタラクトをリセット
	//============================================================

	ClearNavPath();

	m_isTaskIdling = false;

	m_taskIdleTimer = 0.0f;

	m_isInteracting = false;

	m_interactTimer = 0.0f;

	m_debugInfo.stateName = StateToString(m_state);

	ApplyStateSettings();

	switch (m_state)
	{
	case State::Prank:
		ShowEmote(EmoteIcon::IconType::Idea, 2.0f);
		break;

	default:
		break;
	}

}

//============================================================
// Apply State Settings
//============================================================

void Neighbor::ApplyStateSettings()
{
	switch (m_state)
	{
	case State::Task:

		SetMoveSpeed(kTaskSpeed);

		PlayAnimation("Armature|Idle");

		break;

	case State::SearchItem:

		SetMoveSpeed(kSearchItemSpeed);

		PlayAnimation("Armature|Sprint");

		break;

	case State::Prank:

		SetMoveSpeed(kPrankSpeed);

		PlayAnimation("Armature|Sprint");

		break;
	}
}

//============================================================
// Animation
//============================================================

void Neighbor::PlayAnimation(const std::string& animName)
{
	if (m_currentAnimName == animName)
	{
		return;
	}

	m_currentAnimName = animName;

	m_spAnimator->SetAnimation(m_spModel->GetAnimation(animName));
}

//============================================================
// Debug
//============================================================

void Neighbor::DrawDebugUI()
{
	if (ImGui::Begin("Neighbor AI Debug"))
	{
		ImGui::PushID(this);

		if (ImGui::CollapsingHeader(GetName().c_str(), ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::Text("State  : %s", m_debugInfo.stateName.c_str());
			ImGui::Text("Action : %s", m_debugInfo.actionName.c_str());

			ImGui::Separator();
			ImGui::Text("Utility");
			ImGui::Text(" Task         : %.1f", m_debugInfo.taskUtility);
			ImGui::Text(" SearchItem   : %.1f", m_debugInfo.searchItemUtility);
			ImGui::Text(" Prank        : %.1f", m_debugInfo.prankUtility);

			ImGui::Separator();
			ImGui::Text(" Task Destination  : %s", m_debugInfo.taskDestinationName.c_str());
			ImGui::Text(" Distance  : %.2f", m_debugInfo.taskDestinationDistance);
			ImGui::Text(" Status    : %s", m_debugInfo.taskMoveStatus.c_str());

			ImGui::Separator();

			if (ImGui::TreeNodeEx("Inventory", ImGuiTreeNodeFlags_DefaultOpen))
			{
				const auto& items = m_inventory.GetItems();

				for (int i = 0; i < Inventory::MaxItem; ++i)
				{
					if (items[i].IsEmpty())
					{
						ImGui::Text("[%d] (empty)", i);
						continue;
					}

					const std::filesystem::path path(items[i].itemPath);
					const std::string itemName = path.stem().string();

					const bool isSelected = (m_inventory.GetCurrentItem() == &items[i]);

					ImGui::Text(
						"[%d] %s%s",
						i,
						itemName.c_str(),
						isSelected ? "  <- Selected" : ""
					);
				}

				ImGui::TreePop();
			}

			ImGui::Separator();

			{
				const auto searchTarget = m_wpSearchItem.lock();

				if (searchTarget)
				{
					const ItemData& targetData = searchTarget->GetItemData();

					const std::filesystem::path path(targetData.GetItemPath());
					const std::string itemName = path.stem().string();

					Math::Vector3 diff = searchTarget->GetPos() - m_pos;
					diff.y = 0.0f;

					const bool alreadyHave = HasInventoryItem(targetData.GetItemPath());

					ImGui::Text("Search Target : %s", itemName.c_str());
					ImGui::Text(" Distance     : %.2f", diff.Length());

					if (alreadyHave)
					{
						ImGui::TextColored(
							ImVec4(1.0f, 0.3f, 0.3f, 1.0f),
							" WARNING: Already in inventory!"
						);
					}
				}
				else
				{
					ImGui::Text("Search Target : None");
				}
			}

			ImGui::Separator();
			ImGui::Text("Best Prank Target : %s", m_debugInfo.bestPrankTarget.c_str());

			ImGui::Separator();

			if (ImGui::TreeNodeEx("Prank Candidates",
				ImGuiTreeNodeFlags_DefaultOpen))
			{
				if (ImGui::BeginTable("PrankTable", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
				{
					ImGui::TableSetupColumn("Item");
					ImGui::TableSetupColumn("Owner");
					ImGui::TableSetupColumn("CanUse");
					ImGui::TableSetupColumn("Utility");
					ImGui::TableHeadersRow();

					for (const auto& log : m_debugInfo.prankCandidateLog)
					{
						ImGui::TableNextRow();

						if (log.selected)
						{
							ImGui::TableSetBgColor(
								ImGuiTableBgTarget_RowBg0,
								ImGui::GetColorU32(ImVec4(0.2f, 0.6f, 0.2f, 0.5f))
							);
						}

						const std::filesystem::path path(log.itemPath);
						const std::string itemName = path.stem().string();

						std::string ownerName = log.targetName;

						if (ownerName.empty())
						{
							ownerName = "None";
						}
						else if (ownerName == GetName())
						{
							ownerName = "Self";
						}

						ImGui::TableNextColumn(); ImGui::TextUnformatted(itemName.c_str());
						ImGui::TableNextColumn(); ImGui::TextUnformatted(ownerName.c_str());
						ImGui::TableNextColumn(); ImGui::TextUnformatted(log.canNPCUse ? "true" : "false");
						ImGui::TableNextColumn(); ImGui::Text("%.1f", log.utility);
					}

					ImGui::EndTable();
				}
				ImGui::TreePop();
			}
		}

		ImGui::PopID();
	}
	ImGui::End();
}