#pragma once

#include "../CharacterBase.h"

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
};
