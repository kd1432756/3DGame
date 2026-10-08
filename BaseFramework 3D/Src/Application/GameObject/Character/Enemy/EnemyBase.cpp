#include "EnemyBase.h"

#include "../../../System/Navigation/NavMesh/NavMeshManager.h"

void EnemyBase::Init()
{
	CharacterBase::Init();

	//============================================================
	// NavMesh Manager
	//============================================================

	m_pNavMeshManager = &NavMeshManager::Instance();
}

void EnemyBase::Update()
{
	m_moveDir = Math::Vector3::Zero;

	// =========================================
	// 行動を決める
	// =========================================

	UpdateBehavior();

	// =========================================
	// 決めた方向へ移動
	// =========================================

	ApplyMove();

	// =========================================
	// 重力
	// =========================================

	ApplyGravity();

	// =========================================
	// アニメーション
	// =========================================

	UpdateAnimation();

	// =========================================
	// ワールド行列更新
	// =========================================

	UpdateWorldMatrix();

	// =========================================
	// 各NPC固有の更新
	// =========================================

	UpdateExtra();
}

void EnemyBase::SetMoveDirection(
	const Math::Vector3& dir)
{
	m_moveDir = dir;

	m_moveDir.y = 0.0f;

	if (m_moveDir.LengthSquared() <= 0.000001f)
	{
		m_moveDir = Math::Vector3::Zero;
		return;
	}

	m_moveDir.Normalize();
}

void EnemyBase::ApplyMove()
{
	m_isMoving = false;

	if (m_moveDir.LengthSquared() <= 0.000001f)
	{
		return;
	}

	// 向きを移動方向へ変更
	RotateToMoveDirection(m_moveDir);

	// 移動量
	Math::Vector3 moveVec =
		m_moveDir * m_moveSpeed;

	// 壁の先読み
	const float radius =
		m_collisionRadius * m_sceneScale;

	const float margin =
		0.02f * m_sceneScale;

	std::vector<WallHit> walls;

	CollectWallHits(
		radius + margin,
		walls
	);

	// 壁に向かう移動成分を除去
	// 角に備えて2回処理
	for (int pass = 0; pass < 2; ++pass)
	{
		for (auto& w : walls)
		{
			float into =
				moveVec.Dot(w.normal);

			if (into < 0.0f)
			{
				moveVec -=
					w.normal * into;
			}
		}
	}

	const Math::Vector3 before = m_pos;

	// 移動
	m_pos += moveVec;

	// 念のため残っためり込みを解消
	ResolveCollision();

	// 実際に動いた量で判定(壁に止められたら false)
	m_isMoving = (m_pos - before).LengthSquared() > 0.00000001f;
}

//============================================================
// Snap To NavMesh
//============================================================

Math::Vector3 EnemyBase::SnapToNavMesh(
	const Math::Vector3& point) const
{
	if (!m_pNavMeshManager)
	{
		return point;
	}

	//============================================================
	// すでにNavMesh内ならそのまま
	//============================================================

	auto polygon =
		m_pNavMeshManager->FindPolygonFromPoint(point);

	if (polygon)
	{
		return point;
	}

	//============================================================
	// 範囲外 → 最近傍点へ補正
	//============================================================

	return m_pNavMeshManager->FindNearestPoint(
		point,
		m_navSnapRadius * m_sceneScale
	);
}

std::vector<Math::Vector3> EnemyBase::FindNavPath(
	const Math::Vector3& start, const Math::Vector3& goal) const
{
	if (!m_pNavMeshManager) return {};
	return m_navMeshPathfinder.FindPathPoints(
		*m_pNavMeshManager, SnapToNavMesh(start), SnapToNavMesh(goal));
}
