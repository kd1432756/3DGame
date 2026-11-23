#include "EnemyBase.h"

#include "../../../System/Navigation/NavMesh/NavMeshManager.h"

#include "../../../Scene/SceneManager.h"

void EnemyBase::Init()
{
	CharacterBase::Init();

	m_navMeshPathfinder = std::make_unique<NavMeshPathfinder>();

	m_collisionRadius = kEnemyCollisionRadius;
}

void EnemyBase::Update()
{
	m_moveDir = Math::Vector3::Zero;

	// =========================================
	// 詰まり検出からの再探索要求
	// =========================================

	if (ConsumeRepathRequest())
	{
		OnRepathRequested();
	}

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

void EnemyBase::SetMoveDirection(const Math::Vector3& dir)
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
	m_hitCharacter = false;

	UpdateGhost();

	const bool wantMove = m_moveDir.LengthSquared() > 0.000001f;
	const Math::Vector3 before = m_pos;

	if (wantMove)
	{
		// 向きを移動方向へ変更
		RotateToMoveDirection(m_moveDir);

		Math::Vector3 moveVec = m_moveDir * m_moveSpeed;

		// 壁の先読み
		const float radius = WallRadius();
		const float margin = 0.02f * m_sceneScale;

		std::vector<WallHit> walls;
		CollectWallHits(radius + margin, walls);

		// 壁に向かう移動成分を除去（角に備えて2回）
		SlideAlongWalls(moveVec, radius);

		m_pos += moveVec;
	}

	// キャラ同士（立っている時も判定する）
	ResolveCharacterCollision();

	// 壁を最優先で最後に解消
	if (wantMove || m_hitCharacter)
	{
		ResolveCollision();
	}

	Math::Vector3 moved = m_pos - before;
	moved.y = 0.0f;
	const float movedDist = moved.Length();

	// 動く意思があり、実際に動けたときだけ true
	m_isMoving = wantMove && (movedDist * movedDist > 0.00000001f);

	UpdateStuck(wantMove, movedDist);
}

//============================================================
// キャラ同士の押し戻し（自分だけ退く）
//============================================================

void EnemyBase::ResolveCharacterCollision()
{
	if (m_isGhost || m_isArrested) return;

	const float maxPush = kMaxCharaPush * m_sceneScale;

	for (const auto& obj : SceneManager::Instance().GetObjList())
	{
		if (obj->GetCategory() != KdGameObject::ObjectCategory::Character) continue;
		const auto other = std::dynamic_pointer_cast<CharacterBase>(obj);
		if (!other || other.get() == this) continue;
		if (other->IsArrested() || other->IsGhost()) continue;   // 連行中は除外

		Math::Vector3 diff = m_pos - other->GetPos();

		if (std::abs(diff.y) > m_bodyHeight * m_sceneScale) continue;
		diff.y = 0.0f;

		const float minDist = (m_bodyRadius + other->GetBodyRadius()) * m_sceneScale;
		const float dist = diff.Length();

		if (dist >= minDist) continue;

		Math::Vector3 n = (dist > 0.0001f) ? diff / dist : m_mWorld.Right();
		n.y = 0.0f;
		n.Normalize();

		// 押し出し量の上限
		const float push = std::min(minDist - dist, maxPush);

		m_pos += n * push;
		m_hitCharacter = true;
	}
}

bool EnemyBase::IsOverlappingCharacter() const
{
	for (const auto& obj : SceneManager::Instance().GetObjList())
	{
		if (obj->GetCategory() != KdGameObject::ObjectCategory::Character) continue;
		const auto other = std::dynamic_pointer_cast<CharacterBase>(obj);
		if (!other || other.get() == this) continue;
		if (other->IsArrested()) continue;

		Math::Vector3 diff = m_pos - other->GetPos();
		if (std::abs(diff.y) > m_bodyHeight * m_sceneScale) continue;
		diff.y = 0.0f;

		const float minDist = (m_bodyRadius + other->GetBodyRadius()) * m_sceneScale;
		if (diff.LengthSquared() < minDist * minDist) return true;
	}
	return false;
}

//============================================================
// ゴースト（キャラ判定を一時的に無効化）
//============================================================

void EnemyBase::UpdateGhost()
{
	const float dt = 1.0f / 60.0f;

	if (m_ghostCooldown > 0.0f) m_ghostCooldown -= dt;

	if (!m_isGhost) return;

	m_ghostTimer -= dt;

	// 誰とも重なっていなければ復帰
	if (m_ghostTimer <= 0.0f && !IsOverlappingCharacter())
	{
		m_isGhost = false;
		m_ghostCooldown = kGhostCooldownTime;
		m_needRepath = true;
		ResetStuck();
	}
}

//============================================================
// スタック判定 → 一定時間でゴースト化
//============================================================

void EnemyBase::UpdateStuck(bool wantMove, float movedDist)
{
	const float dt = 1.0f / 60.0f;

	const bool notProgressing = wantMove && movedDist < m_moveSpeed * kStuckMoveRatio;

	// ---- ① 原因を問わない詰まり → 経路再探索 ----
	if (m_repathCooldown > 0.0f) m_repathCooldown -= dt;

	if (notProgressing)
	{
		m_blockedTimer += dt;

		if (m_blockedTimer >= kStuckToRepathTime && m_repathCooldown <= 0.0f)
		{
			m_needRepath = true;
			m_repathCooldown = kRepathCooldownTime;
			m_blockedTimer = 0.0f;
		}
	}
	else
	{
		m_blockedTimer = std::max(0.0f, m_blockedTimer - dt * 2.0f);
	}

	// ---- ② キャラ起因の詰まり → ゴースト（既存処理） ----
	const bool stuck = notProgressing && m_hitCharacter;

	if (!stuck)
	{
		// 一瞬の途切れで即リセットしないよう、減衰させる
		m_stuckTimer = std::max(0.0f, m_stuckTimer - dt * 2.0f);
		return;
	}

	m_stuckTimer += dt;

	if (m_stuckTimer >= kStuckToGhostTime && !m_isGhost && m_ghostCooldown <= 0.0f &&
		CanBecomeGhost())
	{
		m_isGhost = true;
		m_ghostTimer = kGhostMinTime;
		ResetStuck();
	}
}

//============================================================
// NavMeshPath
//============================================================

bool EnemyBase::SetNavPathTo(const Math::Vector3& goal)
{
	m_navPath = FindNavPath(m_pos, goal);
	m_navPathIndex = 0;
	return !m_navPath.empty();
}

void EnemyBase::ClearNavPath()
{
	m_navPath.clear();
	m_navPathIndex = 0;
}

MoveResult EnemyBase::FollowNavPath()
{
	if (m_navPath.empty())
	{
		return MoveResult::Failed;
	}

	if (m_navPathIndex >= static_cast<int>(m_navPath.size()))
	{
		ClearNavPath();
		return MoveResult::Arrived;
	}

	Math::Vector3 dir = m_navPath[m_navPathIndex] - m_pos;
	dir.y = 0.0f;

	// 到着半径がちいさ過ぎると到着できない
	const float arrive = std::max(kWaypointArriveDist * m_sceneScale, m_moveSpeed);

	if (dir.LengthSquared() <= arrive * arrive)
	{
		++m_navPathIndex;

		if (m_navPathIndex >= static_cast<int>(m_navPath.size()))
		{
			ClearNavPath();
			SetMoveDirection(Math::Vector3::Zero);
			return MoveResult::Arrived;
		}

		return MoveResult::Moving;
	}

	SetMoveDirection(dir);
	return MoveResult::Moving;
}

//============================================================
// Snap To NavMesh
//============================================================

Math::Vector3 EnemyBase::SnapToNavMesh(const Math::Vector3& point) const
{
	const NavMeshManager navMeshManager = NavMeshManager::Instance();

	// すでにNavMesh内ならそのまま
	const auto polygon = navMeshManager.FindPolygonFromPoint(point);

	if (polygon)
	{
		return point;
	}

	// 範囲外 → 最近傍点へ補正
	return navMeshManager.FindNearestPoint(point, kNavSnapRadius * m_sceneScale);
}

std::vector<Math::Vector3> EnemyBase::FindNavPath(const Math::Vector3& start, const Math::Vector3& goal) const
{
	const NavMeshManager navMeshManager = NavMeshManager::Instance();

	return m_navMeshPathfinder->FindPathPoints(navMeshManager, SnapToNavMesh(start), SnapToNavMesh(goal));
}
