#include "EnemyBase.h"

#include "../../../System/Navigation/NavMesh/NavMeshManager.h"

#include "../../../Scene/SceneManager.h"

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
		const float radius = m_collisionRadius * m_sceneScale;
		const float margin = 0.02f * m_sceneScale;

		std::vector<WallHit> walls;
		CollectWallHits(radius + margin, walls);

		// 壁に向かう移動成分を除去（角に備えて2回）
		for (int pass = 0; pass < 2; ++pass)
		{
			for (auto& w : walls)
			{
				float into = moveVec.Dot(w.normal);
				if (into < 0.0f)
				{
					moveVec -= w.normal * into;
				}
			}
		}

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

	// 動く意思があり、実際に動けたときだけ true（押されただけでは走りアニメにしない）
	m_isMoving = wantMove && (movedDist * movedDist > 0.00000001f);

	UpdateStuck(wantMove, movedDist);
}

//============================================================
// キャラ同士の押し戻し（自分だけ退く）
//============================================================
void EnemyBase::ResolveCharacterCollision()
{
	if (m_isGhost || m_isArrested) return;

	const float maxPush = m_maxCharaPush * m_sceneScale;

	for (auto& obj : SceneManager::Instance().GetObjList())
	{
		auto other = std::dynamic_pointer_cast<CharacterBase>(obj);
		if (!other || other.get() == this) continue;
		if (other->IsArrested() || other->IsGhost()) continue;   // 連行中の相手はここで除外

		Math::Vector3 diff = m_pos - other->GetPos();

		if (std::abs(diff.y) > m_bodyHeight * m_sceneScale) continue;
		diff.y = 0.0f;

		const float minDist = (m_bodyRadius + other->GetBodyRadius()) * m_sceneScale;
		const float dist = diff.Length();

		if (dist >= minDist) continue;

		Math::Vector3 n = (dist > 0.0001f) ? diff / dist : m_mWorld.Right();
		n.y = 0.0f;
		n.Normalize();

		// 瞬間移動を避けるため、1回の押し出し量に上限を付ける
		const float push = std::min(minDist - dist, maxPush);

		m_pos += n * push;
		m_hitCharacter = true;
	}
}

bool EnemyBase::IsOverlappingCharacter() const
{
	for (auto& obj : SceneManager::Instance().GetObjList())
	{
		auto other = std::dynamic_pointer_cast<CharacterBase>(obj);
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

	// 最低継続時間を過ぎ、かつ誰とも重なっていなければ復帰
	if (m_ghostTimer <= 0.0f && !IsOverlappingCharacter())
	{
		m_isGhost = false;
		m_ghostCooldown = m_ghostCooldownTime;
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

	const bool notProgressing =
		wantMove && movedDist < m_moveSpeed * m_stuckMoveRatio;

	// ---- ① 原因を問わない詰まり → 経路再探索 ----
	if (m_repathCooldown > 0.0f) m_repathCooldown -= dt;

	if (notProgressing)
	{
		m_blockedTimer += dt;

		if (m_blockedTimer >= m_stuckToRepathTime && m_repathCooldown <= 0.0f)
		{
			m_needRepath = true;
			m_repathCooldown = m_repathCooldownTime;
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

	if (m_stuckTimer >= m_stuckToGhostTime &&
		!m_isGhost &&
		m_ghostCooldown <= 0.0f &&
		CanBecomeGhost())
	{
		m_isGhost = true;
		m_ghostTimer = m_ghostMinTime;
		ResetStuck();
	}
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
