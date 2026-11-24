#pragma once

#include "../EnemyBase.h"

class LocationPointManager;
class LocationPoint;

#include "../../../../System/Location/LocationTag.h"

class Item;

#include "../../../Object/StaticMeshObject/Item/ItemData.h"

#include "../../../../System/Inventory/Inventory.h"

class InteractiveObject;

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
	};

	inline const char* StateToString(Neighbor::State state) const
	{
		switch (state)
		{
		case Neighbor::State::Task: return "Task";
		case Neighbor::State::SearchItem: return "SearchItem";
		case Neighbor::State::Prank: return "Prank";
		default: return "Unknown";
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
	};

	inline const char* ActionToString(Neighbor::Action action) const
	{
		switch (action)
		{
		case Neighbor::Action::Task: return "Task";
		case Neighbor::Action::SearchItem: return "SearchItem";
		case Neighbor::Action::Prank: return "Prank";
		default: return "Unknown";
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

		Math::Vector3 approachPos = Math::Vector3::Zero;
	};

	//============================================================
	// Model
	//============================================================

	void SetModelPath(const std::string& path)
	{
		m_modelPath = path;
		if (m_spModel) ReloadModel();   // 生成済みのときだけ反映
	}

	const std::string& GetModelPath() const { return m_modelPath; }

	//============================================================
	// Inventory
	//============================================================

	void SelectInventory(int index) override;

	void OnItemsConfiscated() override;

	const ItemData& GetCurrentItemData() const
	{
		return m_currentItemData;
	}

	//============================================================
	// Damage
	//============================================================

	bool IsDamaging() const override
	{
		return m_state == State::Prank && m_isInteracting;
	}

	bool IsPranking() const override
	{
		return m_state == State::Prank;
	}

	Math::Vector3 GetDamagePos() const override;

	//============================================================
	// Arrested
	//============================================================

	void OnArrested(const std::shared_ptr<Police>& police) override;

	void OnReleased() override;

protected:

	//============================================================
	// AI
	//============================================================

	void UpdateBehavior() override;

private:

	void UpdateTimer();

	//============================================================
	// Model
	//============================================================

	void ReloadModel();

	//============================================================
	// Utility AI
	//============================================================

	void UpdateUtility();

	Action DecideAction() const;

	float EvaluateTask() const;

	float EvaluateSearchItem();

	float EvaluatePrank();

	bool CalculatePathDistance(
		const Math::Vector3& goal,
		float& outDistance) const;

	//============================================================
	// Task
	//============================================================

	void UpdateTask();

	std::shared_ptr<LocationPoint> SelectTaskDestination();

	std::shared_ptr<LocationPoint> SelectItemAreaPoint() const;
	std::shared_ptr<LocationPoint> SelectHousePoint() const;
	std::shared_ptr<LocationPoint> SelectRandomPoint() const;

	//============================================================
	// Search Item
	//============================================================

	void UpdateSearchItem();

	bool FindBestSearchItem(SearchItemCandidate& outCandidate);

	float EvaluateSearchItemCandidate(const std::shared_ptr<Item>& item, float pathDistance) const;

	void SelectSearchItemCandidate(const SearchItemCandidate& candidate);

	MoveResult MoveToSearchItem();

	void PickupSearchItem();

	bool CanUseItemForPrank(const ItemData& itemData) const;

	bool IsBareHanded() const { return m_inventory.GetCurrentItem() == nullptr; }

	//============================================================
	// Prank
	//============================================================

	void UpdatePrank();

	bool FindBestPrankCandidate(PrankCandidate& outCandidate);

	float EvaluatePrankCandidate(
		const std::shared_ptr<InteractiveObject>& target,
		const ItemData& itemData,
		float pathDistance) const;

	bool SelectPrankCandidate(const PrankCandidate& candidate);

	bool FindPrankApproachPos(
		const Math::Vector3& targetPos,
		Math::Vector3& outApproachPos) const;

	void ReleasePrankTarget();

	void StartPrankCooldown();
	void DropUselessItems();

	//============================================================
	// Movement
	//============================================================

	MoveResult MoveToTaskDestination(const std::shared_ptr<LocationPoint>& destPoint);

	MoveResult MoveToPrankTarget();

	//============================================================
	// Animation
	//============================================================
	
	void PlayAnimation(const std::string& animName);

	//============================================================
	// State
	//============================================================

	void ChangeState(State newState);

	void ApplyStateSettings();

private:

	//============================================================
	// Model
	//============================================================

	std::string m_modelPath = "Asset/Models/Character/Neighbor/Neighbor.gltf";	// デフォルトのパス

	//============================================================
	// Utility
	//============================================================

	float m_utilityUpdateTimer = 0.0f;

	static constexpr float kUtilityUpdateInterval = 1.0f;

	//============================================================
	// Utility Debug
	//============================================================

	float m_taskUtility = 0.0f;

	float m_searchItemUtility = 0.0f;

	float m_prankUtility = 0.0f;

	Action m_currentAction = Action::Task;

	//============================================================
	// Inventory
	//============================================================

	ItemData m_currentItemData;

	//============================================================
	// State
	//============================================================

	State m_state = State::Task;

	//============================================================
	// State Settings
	//============================================================

	static constexpr float kTaskSpeed = 0.016f;

	static constexpr float kSearchItemSpeed = 0.02f;

	static constexpr float kPrankSpeed = 0.02f;

	//============================================================
	// Animation
	//============================================================

	std::string m_currentAnimName = "";

	//============================================================
	// Location Point
	//============================================================

	LocationPointManager* m_pLocationPointManager = nullptr;

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

	static constexpr float kSearchItemRadius = 15.0f;

	static constexpr float kSearchItemInteractDistance = 1.2f;

	static constexpr float kSearchCommitBonus = 15.0f;

	//============================================================
	// Task
	//============================================================

	std::weak_ptr<LocationPoint> m_wpTaskDestination;

	static constexpr float kTaskArriveDistance = 2.0f;

	static constexpr float kTaskBaseUtility = 10.0f;

	float m_taskIdleTimer = 0.0f;      // 残り待機時間

	bool  m_isTaskIdling = false;     // 待機中か

	static constexpr float kTaskIdleMin = 0.2f;      // 最小待機秒

	static constexpr float kTaskIdleMax = 1.0f;      // 最大待機秒

	//============================================================
	// Prank
	//============================================================

	std::weak_ptr<InteractiveObject> m_wpPrankTarget;

	// Utility評価で選ばれたPrank候補
	PrankCandidate m_bestPrankCandidate;

	// 実際にPrankで使用するInventoryスロット
	int m_prankInventoryIndex = -1;

	static constexpr float kPrankDesire = 0.3f;

	static constexpr float kPrankSearchRadius = 25.0f;

	static constexpr float kPrankApproachMaxDistance = 5.0f;
	Math::Vector3 m_prankApproachPos = Math::Vector3::Zero;

	static constexpr float kPrankInteractDistance = 1.0f;

	static constexpr float kPrankCommitBonus = 15.0f;

	static constexpr float kPrankChainChance = 0.5f;	// 再捜索する確率
	static constexpr int   kMaxPrankChain = 3;		// 1回で連続できる上限

	int m_prankChainCount = 0;

	float m_prankCooldownTimer = 0.0f;

	static constexpr float kPrankCooldownMax = 25.0f;	// 朝のクールダウン(秒)
	static constexpr float kPrankCooldownMin = 8.0f;	// 夜のクールダウン(秒)

private:

	// デバッグ用スナップショット
	struct DebugInfo
	{
		// Utility
		float taskUtility = 0.0f;
		float searchItemUtility = 0.0f;
		float prankUtility = 0.0f;

		// 現在のState
		std::string stateName = "None";
		std::string actionName = "None";

		std::string taskDestinationName = "None";
		bool hasTaskDestination = false;
		float taskDestinationDistance = 0.0f;
		std::string taskMoveStatus = "Idle";

		// Prank候補の探索ログ
		struct PrankLog
		{
			std::string itemPath;
			std::string targetName;
			bool canNPCUse = false;
			float distanceSq = 0.0f;
			float utility = 0.0f;
			bool selected = false;
		};
		std::vector<PrankLog> prankCandidateLog;

		// 選ばれた結果
		std::string bestPrankTarget = "None";
	};

	DebugInfo m_debugInfo;


public:

	void DrawDebugUI() override;
	const DebugInfo& GetDebugInfo() const { return m_debugInfo; }

};