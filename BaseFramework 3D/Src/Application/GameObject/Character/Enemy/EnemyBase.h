#pragma once

#include "../CharacterBase.h"

class NavMeshManager;
class NavMeshPathfinder;
#include "../../../System/Navigation/NavMesh/NavMeshPathfinder.h"

enum class MoveResult
{
	Moving,
	Arrived,
	Failed
};

class EnemyBase : public CharacterBase
{

public:
	virtual void Init() override;
	virtual void Update() override;

public:

	bool IsGhost() const override { return m_isGhost; }

protected:

	// =========================================
	// 子クラスが行動を決める
	// =========================================

	virtual void UpdateBehavior() = 0;
	virtual void UpdateExtra() {}

	// =========================================
	// 移動方向を設定
	// =========================================

	void SetMoveDirection(
		const Math::Vector3& dir);

	// =========================================
	// 実際に移動する
	// =========================================

	void ApplyMove();

	// =========================================
	// 移動速度
	// =========================================

	void SetMoveSpeed(float speed)
	{
		m_moveSpeed = speed;
	}

	float GetMoveSpeed() const
	{
		return m_moveSpeed;
	}

	float m_moveSpeed = 0.05f;

	// =========================================
	// 現在の移動方向
	// =========================================

	Math::Vector3 m_moveDir =
		Math::Vector3::Zero;

	static constexpr float m_enemyCollisionRadius = 0.3f;

	// =========================================
	// NavMesh
	// =========================================

	std::unique_ptr<NavMeshPathfinder> m_navMeshPathfinder;

	std::vector<Math::Vector3> m_navPath;

	int m_navPathIndex = 0;

	Math::Vector3 SnapToNavMesh(const Math::Vector3& point) const;

	std::vector<Math::Vector3> FindNavPath(
		const Math::Vector3& start, const Math::Vector3& goal) const;

	static constexpr float m_navSnapRadius = 2.0f;

	// =========================================
	// 当たり判定
	// =========================================

	virtual bool CanBecomeGhost() const { return true; }   // Policeはfalse

	// =========================================
	// 経路探索
	// =========================================

	bool SetNavPathTo(const Math::Vector3& goal);
	void ClearNavPath();
	MoveResult FollowNavPath();

	static constexpr float m_waypointArriveDist = 0.2f;

	// =========================================
	// 再探索要求
	// =========================================

	bool ConsumeRepathRequest()
	{
		const bool r = m_needRepath;
		m_needRepath = false;
		return r;
	}

	virtual void OnRepathRequested()
	{
		m_navPath.clear();
		m_navPathIndex = 0;
	}

private:

	void ResolveCharacterCollision();
	bool IsOverlappingCharacter() const;
	void UpdateGhost();
	void UpdateStuck(bool wantMove, float movedDist);
	void ResetStuck() { m_stuckTimer = 0.0f; }

	bool  m_hitCharacter = false;   // このフレームにキャラに押し戻されたか
	float m_stuckTimer = 0.0f;

	bool  m_isGhost = false;
	float m_ghostTimer = 0.0f;
	float m_ghostCooldown = 0.0f;

	// パラメータ
	static constexpr float m_stuckToGhostTime = 2.0f;    // 詰まりがこの秒数続いたらゴースト化
	static constexpr float m_stuckMoveRatio = 0.3f;      // 期待移動量のこの割合未満なら「進めていない」
	static constexpr float m_ghostMinTime = 1.5f;        // ゴースト最低継続時間
	static constexpr float m_ghostCooldownTime = 3.0f;   // 復帰後、再ゴースト化までの時間
	static constexpr float m_maxCharaPush = 0.15f;       // 1フレームのキャラ押し出し上限（×スケール）

	float m_blockedTimer = 0.0f;
	float m_repathCooldown = 0.0f;
	bool  m_needRepath = false;

	static constexpr float m_stuckToRepathTime = 0.5f;   // この時間進めなければ再探索
	static constexpr float m_repathCooldownTime = 1.0f;  // 再探索の連発防止
};
