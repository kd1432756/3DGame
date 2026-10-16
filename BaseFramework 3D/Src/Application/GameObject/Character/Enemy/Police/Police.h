#pragma once

#include "../EnemyBase.h"

#include "../../../../System/Node/AStarNode/AStarPathfinder.h"

class AStarNodeManager;
class LocationPointManager;

class AlertIcon;

class Police : public EnemyBase
{
public:

	void Init() override;

	const char* GetTypeName() const override
	{
		return "Police";
	}

	ObjectCategory GetCategory() const override
	{
		return ObjectCategory::Character;
	}

	enum class State
	{
		Patrol,
		Chase, 
		Arrest,
		Escort,
		LastSeen,
		Search,
		Return, 
		Investigate
	};

protected:

	//============================================================
	// AI
	//============================================================

	void UpdateBehavior() override;
	void UpdateExtra() override;

private:

	//============================================================
	// Timer
	//============================================================

	void UpdateTimers();

	//============================================================
	// Patrol
	//============================================================

	void UpdatePatrol();

	//============================================================
	// Chase
	//============================================================

	void UpdateChase();

	//============================================================
	// Arrest
	//============================================================

	void UpdateArrest();

	//============================================================
	// Escort
	//============================================================

	void UpdateEscort();

	//============================================================
	// Last Seen
	//============================================================

	void UpdateLastSeen();	

	//============================================================
	// Search
	//============================================================

	void UpdateSearch();

	//============================================================
	// Return
	//============================================================

	void UpdateReturn();

	//============================================================
	// Investigate
	//============================================================

	void UpdateInvestigate();

	//============================================================
	// Path
	//============================================================

	void FindPatrolPath();

	int FindNearestPatrolNode(
		const Math::Vector3& pos);

	bool MoveToNextPathNode();

	void FindChasePath();

	bool MoveToNextNavPathPoint();

	//============================================================
	// Search
	//============================================================

	bool SearchTarget();

	bool CanSeeTarget(const std::shared_ptr<CharacterBase>& target);

	bool IsWanted(const std::shared_ptr<CharacterBase>& c) const
	{
		return c->HasDangerousItem() || c->IsPranking();
	}

	//============================================================
	// Arrest
	//============================================================

	bool CanArrestTarget(const std::shared_ptr<CharacterBase>& target) const;

	//============================================================
	// Escort
	//============================================================

	void BeginEscort(const std::shared_ptr<CharacterBase>& target);

	void EndEscort(bool confiscate);

	void FindPolicePostPath();

	//============================================================
	// Investigate
	//============================================================

	bool DetectPrankSound();

	//============================================================
	// State
	//============================================================

	void ChangeState(State newState);

	void ApplyStateSettings();

private:

	//============================================================
	// State
	//============================================================

	State m_state = State::Patrol;

	//============================================================
	// State Settings
	//============================================================

	float m_patrolSpeed = 0.01f;
	float m_chaseSpeed = 0.026f;
	float m_returnSpeed = 0.01f;
	float m_escortSpeed = 0.016f;
	float m_investigateSpeed = 0.018f;

	//============================================================
	// A*
	//============================================================

	AStarPathfinder m_pathfinder;

	AStarNodeManager* m_pNodeManager = nullptr;

	//============================================================
	// Patrol
	//============================================================

	std::vector<int> m_currentPath;

	int m_currentPathIndex = 0;

	int m_currentStartNode = -1;

	int m_previousPatrolNode = -1;

	//============================================================
	// Location Point
	//============================================================

	LocationPointManager* m_pLocationPointManager = nullptr;

	//============================================================
	// Chase
	//============================================================

	std::weak_ptr<CharacterBase> m_wpTarget;

	float m_pathUpdateTimer = 0.0f;

	float m_pathUpdateInterval = 0.2f;

	Math::Vector3 m_lastTargetPos =
		Math::Vector3::Zero;

	bool m_chasingForPrank = false;

	//============================================================
	// Investigate
	//============================================================

	Math::Vector3 m_investigatePos = Math::Vector3::Zero;

	float m_hearRange = 25.0f;

	float m_investigateCooldown = 0.0f;

	//============================================================
	// Arrest
	//============================================================

	std::weak_ptr<CharacterBase> m_wpArrested;

	float m_arrestTimer = 0.0f;

	float m_arrestTime = 0.0f;

	float m_arrestDistance = 1.6f;

	//============================================================
	// lose sight
	//============================================================

	float m_loseSightTimer = 0.0f;

	const float m_loseSightTime = 2.0f;

	//============================================================
	// Search
	//============================================================

	float m_searchTimer = 0.0f;

	// Search全体の時間
	float m_searchTime = 3.0f;

	// 見回す速度
	float m_searchRotateSpeed = 90.0f;

	float m_searchStartRotY = 0.0f;
};
