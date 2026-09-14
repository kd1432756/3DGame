#pragma once

#include "../EnemyBase.h"

#include "../../../../System/Navigation/NavMesh/NavMeshPathfinder.h"
#include "../../../../System/Inventory/Inventory.h"

class NavMeshManager;

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

	//============================================================
	// Utility Action
	//============================================================

	enum class Action
	{
		Task,
		SearchItem,
		Prank,
		Repair
	};

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
	// Prank Target
	//============================================================

	void SetPrankTarget(
		const std::shared_ptr<InteractiveObject>& target)
	{
		m_wpPrankTarget = target;
	}


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


	//============================================================
	// Task
	//============================================================

	void UpdateTask();

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
		const ItemData& itemData);

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
	// Movement
	//============================================================

	void FindPathTo(
		const Math::Vector3& goal);

	MoveResult MoveToNextNavPathPoint();

	MoveResult MoveToPrankTarget();

	MoveResult MoveToRepairTarget();

	//============================================================
	// State
	//============================================================

	void ChangeState(
		State newState);

	void ApplyStateSettings();


	//============================================================
	// Utility
	//============================================================

	float m_utilityUpdateTimer = 0.0f;

	// Utilityを再評価する間隔
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
};