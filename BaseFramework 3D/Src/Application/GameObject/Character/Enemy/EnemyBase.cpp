#include "EnemyBase.h"

void EnemyBase::Init()
{
	CharacterBase::Init();
}

void EnemyBase::Update()
{
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
	if (m_moveDir.LengthSquared() <= 0.000001f)
	{
		return;
	}

	// 向きを移動方向へ変更
	RotateToMoveDirection(m_moveDir);

	// 移動量
	Math::Vector3 moveVec =
		m_moveDir * m_moveSpeed;

	// 移動
	m_pos += moveVec;

	// 壁との衝突解決
	ResolveCollision(moveVec);
}
