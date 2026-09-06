#pragma once

#include "../EnemyBase.h"

#include "../../../../System/Navigation/NavMesh/NavMeshPathfinder.h"
#include "../../../../System/Inventory/Inventory.h"

class NavMeshManager;

class Item;
#include "../../../Object/StaticMeshObject/Item/ItemData.h"

class InteractiveObject;

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
		// State
		//============================================================

	enum class State
	{
		Task,
		Prank,
		Repair
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
	// Task
	//============================================================

	void UpdateTask();

	//============================================================
	// Prank
	//============================================================

	void UpdatePrank();

	bool MoveToPrankTarget();

	void TryStartPrank();

	std::shared_ptr<InteractiveObject> SearchPrankTarget();

	//============================================================
	// Repair
	//============================================================

	void UpdateRepair();

	//============================================================
	// 移動（NavMesh）
	//============================================================

	void FindPathTo(const Math::Vector3& goal);

	bool MoveToNextNavPathPoint();

	//============================================================
	// State
	//============================================================

	void ChangeState(State newState);

	void ApplyStateSettings();

private:

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
	// Prank
	//============================================================

	std::weak_ptr<InteractiveObject> m_wpPrankTarget;

	// いたずら実行を試みるまでの間隔（Task中に加算）
	float m_prankCheckTimer = 0.0f;

	float m_prankCheckInterval = 10.0f;

	// いたずら実行の確率（0.0～1.0）
	float m_prankChance = 0.3f;

	// Prank対象を検索する半径
	float m_prankSearchRadius = 8.0f;

	float m_prankInteractDistance = 1.5f;

	// ターゲットに到着後、Use()を呼ぶまでのインタラクト待機
	bool m_isInteracting = false;

	float m_interactTimer = 0.0f;

	//============================================================
	// Repair（未実装・遷移口のみ）
	//============================================================

	std::weak_ptr<InteractiveObject> m_wpRepairTarget;
};