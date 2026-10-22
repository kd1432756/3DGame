#pragma once

#include "../CharacterBase.h"

#include "../../../System/Navigation/NavMesh/NavMeshPathfinder.h"
class NavMeshManager;

class EnemyBase : public CharacterBase
{

public:
	virtual void Init() override;
	virtual void Update() override;

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

	float m_collisionRadius = 0.3f;

	// =========================================
	// NavMesh
	// =========================================

	NavMeshPathfinder m_navMeshPathfinder;

	NavMeshManager* m_pNavMeshManager = nullptr;

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

public:

	bool IsGhost() const override { return m_isGhost; }

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
	const float m_stuckToGhostTime = 2.0f;    // 詰まりがこの秒数続いたらゴースト化
	const float m_stuckMoveRatio = 0.3f;      // 期待移動量のこの割合未満なら「進めていない」
	const float m_ghostMinTime = 1.5f;        // ゴースト最低継続時間
	const float m_ghostCooldownTime = 3.0f;   // 復帰後、再ゴースト化までの時間
	const float m_maxCharaPush = 0.15f;       // 1フレームのキャラ押し出し上限（×スケール）
};
