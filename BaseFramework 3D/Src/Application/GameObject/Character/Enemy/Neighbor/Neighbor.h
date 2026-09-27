#pragma once

#include "../EnemyBase.h"

#include "../../../../System/Navigation/NavMesh/NavMeshPathfinder.h"
#include "../../../../System/Inventory/Inventory.h"

class NavMeshManager;
class LocationPointManager;
class LocationPoint;

#include "../../../../System/Location/LocationTag.h"

class Item;

#include "../../../Object/StaticMeshObject/Item/ItemData.h"

class InteractiveObject;

//============================================================
// Move Result
//============================================================

enum class MoveResult
{
	Moving,
	Arrived,
	Failed
};

//============================================================
// Neighbor
//============================================================

class Neighbor : public EnemyBase
{
public:

	void Init() override;

	const char* GetTypeName() const override
	{
		return "Neighbor";
	}

	ObjectCategory GetCategory() const override
	{
		return ObjectCategory::Character;
	}


	//============================================================
	// FSM State
	//============================================================

	enum class State
	{
		Task,
		SearchItem,
		Prank,
		Repair
	};

	inline const char* StateToString(Neighbor::State state)
	{
		switch (state)
		{
		case Neighbor::State::Task:       return "Task";
		case Neighbor::State::SearchItem: return "SearchItem";
		case Neighbor::State::Prank:      return "Prank";
		case Neighbor::State::Repair:     return "Repair";
		default:                          return "Unknown";
		}
	}

	//============================================================
	// Utility Action
	//============================================================

	enum class Action
	{
		Task,
		SearchItem,
		Prank,
		Repair,
		UrgentRepair
	};

	inline const char* ActionToString(Neighbor::Action action)
	{
		switch (action)
		{
		case Neighbor::Action::Task:       return "Task";
		case Neighbor::Action::SearchItem: return "SearchItem";
		case Neighbor::Action::Prank:      return "Prank";
		case Neighbor::Action::Repair:     return "Repair";
		case Neighbor::Action::UrgentRepair: return "UrgentRepair";
		default:                           return "Unknown";
		}
	}
	
	//============================================================
	// Search Item Candidate
	//============================================================

	struct SearchItemCandidate
	{
		std::shared_ptr<Item> item;

		float utility = 0.0f;
	};

	//============================================================
	// Prank Candidate
	//============================================================

	struct PrankCandidate
	{
		std::shared_ptr<InteractiveObject> target;

		int inventoryIndex = -1;

		float utility = 0.0f;

		bool hasPath = false;

		float pathDistance = 0.0f;
	};

	//============================================================
	// Repair Candidate
	//============================================================

	struct RepairCandidate
	{
		std::shared_ptr<InteractiveObject> target;

		int inventoryIndex = -1;

		float utility = 0.0f;
	};

	//============================================================
	// Inventory
	//============================================================

	Inventory& GetInventory()
	{
		return m_inventory;
	}

	void AddInventoryItem(
		const std::string& itemPath);

	void SelectInventory(
		int index);

	const ItemData& GetCurrentItemData() const
	{
		return m_currentItemData;
	}

	bool HasInventoryItem(const std::string& itemPath) const;

	//============================================================
	// House Tag
	//============================================================

	void SetHouseTag(LocationTag tag)
	{
		m_myHouseTag = tag;
	}

	LocationTag GetHouseTag() const
	{
		return m_myHouseTag;
	}

	//============================================================
	// Prank Target
	//============================================================

	void SetPrankTarget(
		const std::shared_ptr<InteractiveObject>& target)
	{
		m_wpPrankTarget = target;
	}

	//============================================================
	// Arrested
	//============================================================

	void OnArrested(const std::shared_ptr<Police>& police) override;

	void OnReleased() override;

	bool HasPrankItem() const override;
		
	void ConfiscatePrankItems() override;

	bool HasRepairItem() const;

	//============================================================
	// On Pranked
	//============================================================

	void OnPranked(
		const std::shared_ptr<InteractiveObject>& target,
		const InteractiveAction& action) override;


protected:

	//============================================================
	// AI
	//============================================================

	void UpdateBehavior() override;


private:

	//============================================================
	// Utility AI
	//============================================================

	void UpdateUtility();

	Action DecideAction();

	float EvaluateTask();

	float EvaluateSearchItem();

	float EvaluatePrank();

	float EvaluateRepair();

	bool CalculatePathDistance(
		const Math::Vector3& goal,
		float& outDistance) const;

	//============================================================
	// Task
	//============================================================

	void UpdateTask();

	std::shared_ptr<LocationPoint> SelectTaskDestination();

	std::shared_ptr<LocationPoint> SelectItemAreaPoint();
	std::shared_ptr<LocationPoint> SelectOtherHousePoint();
	std::shared_ptr<LocationPoint> SelectOwnHousePoint();
	std::shared_ptr<LocationPoint> SelectRandomPoint();

	//============================================================
	// Search Item
	//============================================================

	void UpdateSearchItem();

	bool FindBestSearchItem(
		SearchItemCandidate& outCandidate);

	float EvaluateSearchItemCandidate(
		const std::shared_ptr<Item>& item);

	void SelectSearchItemCandidate(
		const SearchItemCandidate& candidate);

	MoveResult MoveToSearchItem();

	void PickupSearchItem();

	bool CanUseItemForPrank(const ItemData& itemData) const;
	bool CanUseItemForRepair(const ItemData& itemData) const;

	//============================================================
	// Prank
	//============================================================

	void UpdatePrank();

	bool FindBestPrankCandidate(
		PrankCandidate& outCandidate);

	float EvaluatePrankCandidate(
		const std::shared_ptr<InteractiveObject>& target,
		const ItemData& itemData,
		float pathDistance);

	void SelectPrankCandidate(
		const PrankCandidate& candidate);

	//============================================================
	// Repair
	//============================================================

	void UpdateRepair();

	bool FindBestRepairCandidate(
		RepairCandidate& outCandidate);

	float EvaluateRepairCandidate(
		const std::shared_ptr<InteractiveObject>& target,
		const ItemData& itemData);

	void SelectRepairCandidate(
		const RepairCandidate& candidate);

	//============================================================
	// Urgent Repair
	//============================================================

	bool FindRepairItemForTarget(
		const std::shared_ptr<InteractiveObject>& target,
		SearchItemCandidate& outCandidate);

	float EvaluateUrgentRepair();

	void CleanupUrgentRepairTargets();

	std::shared_ptr<InteractiveObject> SelectBestUrgentRepairTarget();

	//============================================================
	// Movement
	//============================================================

	void FindPathTo(
		const Math::Vector3& goal);

	Math::Vector3 SnapToNavMesh(const Math::Vector3& point) const;

	MoveResult MoveToNextNavPathPoint();

	MoveResult MoveToTaskDestination(
		const std::shared_ptr<LocationPoint>& destPoint);

	MoveResult MoveToPrankTarget();

	MoveResult MoveToRepairTarget();

	//============================================================
	// State
	//============================================================

	void ChangeState(
		State newState);

	void ApplyStateSettings();

private:

	//============================================================
	// Utility
	//============================================================

	float m_utilityUpdateTimer = 0.0f;

	float m_utilityUpdateInterval = 1.0f;

	//============================================================
	// Utility Debug
	//============================================================

	float m_taskUtility = 0.0f;

	float m_searchItemUtility = 0.0f;

	float m_prankUtility = 0.0f;

	float m_repairUtility = 0.0f;

	Action m_currentAction = Action::Task;

	//============================================================
	// Owner
	//============================================================

	std::string m_owner;

	//============================================================
	// Inventory
	//============================================================

	Inventory m_inventory;

	ItemData m_currentItemData;

	//============================================================
	// State
	//============================================================

	State m_state = State::Task;

	//============================================================
	// State Settings
	//============================================================

	float m_taskSpeed = 0.006f;

	float m_searchItemSpeed = 0.01f;

	float m_prankSpeed = 0.01f;

	float m_repairSpeed = 0.01f;

	//============================================================
	// NavMesh
	//============================================================

	NavMeshPathfinder m_navMeshPathfinder;

	NavMeshManager* m_pNavMeshManager = nullptr;

	std::vector<Math::Vector3> m_navPath;

	int m_navPathIndex = 0;

	static constexpr float m_navSnapRadius = 2.0f;

	//============================================================
	// Location Point
	//============================================================

	LocationPointManager* m_pLocationPointManager = nullptr;

	LocationTag m_myHouseTag = LocationTag::NeighborHouse1;

	//============================================================
	// Use
	//============================================================

	bool m_isInteracting = false;

	float m_interactTimer = 0.0f;

	//============================================================
	// Search Item
	//============================================================

	SearchItemCandidate m_bestSearchItemCandidate;

	std::weak_ptr<Item> m_wpSearchItem;

	float m_searchItemRadius = 15.0f;

	float m_searchItemInteractDistance = 1.2f;

	//============================================================
	// Task
	//============================================================

	std::weak_ptr<LocationPoint> m_wpTaskDestination;

	float m_taskArriveDistance = 2.0f;

	float m_taskBaseUtility = 10.0f;

	//============================================================
	// Prank
	//============================================================

	std::weak_ptr<InteractiveObject> m_wpPrankTarget;

	// Utility評価で選ばれたPrank候補
	PrankCandidate m_bestPrankCandidate;

	// 実際にPrankで使用するInventoryスロット
	int m_prankInventoryIndex = -1;

	float m_prankDesire = 0.3f;

	float m_prankSearchRadius = 8.0f;

	float m_prankInteractDistance = 1.5f;

	//============================================================
	// Repair
	//============================================================

	std::weak_ptr<InteractiveObject> m_wpRepairTarget;

	RepairCandidate m_bestRepairCandidate;

	int m_repairInventoryIndex = -1;

	float m_repairInteractDistance = 1.5f;

	float m_repairSearchRadius = 8.0f;

	private:

	//============================================================
	// Urgent Repair
	//============================================================
		
	std::vector<std::weak_ptr<InteractiveObject>> m_urgentRepairTargets;
	
	std::weak_ptr<InteractiveObject> m_wpSelectedUrgentRepairTarget;

	float m_urgentRepairUtility = 70.0f;

	float m_urgentRepairBaseUtility = 70.0f;

	float m_urgentRepairSearchRadius = 30.0f;

private:

	// デバッグ用スナップショット
	struct DebugInfo
	{
		// Utility
		float taskUtility = 0.0f;
		float searchItemUtility = 0.0f;
		float prankUtility = 0.0f;
		float repairUtility = 0.0f;
		float urgentRepairUtility = 0.0f;

		// 現在のState
		std::string stateName = "None";
		std::string actionName = "None";

		std::string taskDestinationName = "None";
		bool hasTaskDestination = false;
		float taskDestinationDistance = 0.0f;
		std::string taskMoveStatus = "Idle";

		// Prank候補の探索ログ（直近フレーム分だけ保持）
		struct PrankLog
		{
			std::string itemPath;
			std::string targetOwner;
			bool canNPCUse = false;
			float distanceSq = 0.0f;
			float utility = 0.0f;
			bool selected = false;
		};
		std::vector<PrankLog> prankCandidateLog;

		struct RepairLog
		{
			std::string itemPath;
			std::string targetOwner;
			bool canNPCUse = false;
			float utility = 0.0f;
			bool selected = false;
		};
		std::vector<RepairLog> repairCandidateLog;

		// 選ばれた結果
		std::string bestPrankTarget = "None";
		std::string bestRepairTarget = "None";

		// Urgent Repair
		std::string bestUrgentRepairTarget = "None";
		int urgentRepairQueueSize = 0;
	};

	DebugInfo m_debugInfo;


public:

	void DrawDebugUI() override;
	const DebugInfo& GetDebugInfo() const { return m_debugInfo; }

};