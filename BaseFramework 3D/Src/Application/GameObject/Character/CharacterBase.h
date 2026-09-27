#pragma once

class Police;

#include "../Object/StaticMeshObject/InteractiveObject/InteractiveObject.h"

class CharacterBase : public KdGameObject
{
public:
	virtual void Init() override;
	virtual void Update() override;
	virtual void DrawLit() override;
	virtual void GenerateDepthMapFromLight() override;

	virtual void OnPrankSuccess(
		int score,
		const std::string& message)
	{
		AddScore(score);
	}

	void AddScore(int score)
	{
		m_score += score;
	}

	int GetScore() const
	{
		return m_score;
	}

	// いたずらされた
	virtual void OnPranked(
		const std::shared_ptr<InteractiveObject>& object, 
		const InteractiveAction& action)
	{}
	
	// 警察に確保された
	virtual void OnArrested(const std::shared_ptr<Police>& police);

	// 解放された
	virtual void OnReleased();

	// いたずらに使うアイテムを持っているか(警察の追跡条件)
	virtual bool HasPrankItem() const { return false; }

	// いたずらアイテムを没収する
	virtual void ConfiscatePrankItems() {}

	bool IsArrested() const { return m_isArrested; }

protected:

	Math::Vector3 GetForward() const;

	void CheckStep(
		const Math::Vector3& moveDir);

	void CheckGround();

	void ResolveCollision(
		const Math::Vector3& moveDir);

	void ApplyGravity();

	void RotateToMoveDirection(const Math::Vector3& dir);

	void UpdateAnimation();
	std::shared_ptr<KdModelWork>				m_spModel = nullptr;
	std::shared_ptr<KdAnimator>					m_spAnimator = nullptr;

	float m_sceneScale = 1.0f;

	float m_gravity = 0.0f;

	float m_gravityPower = 0.015f;

	bool m_isGround = false;

	float m_jumpPower = 0.30f;

	float m_angle = 0.0f;

	int m_score = 0;

	bool m_isArrested = false;

	// ===== ray =====
	
	// 最大で登れる段差
	float m_stepHeight = 1.0f;

	// m_posから足元までの距離
	float m_footOffset = 0.8f;

	// 足元からRayを少し上に出す
	float m_rayStart = 0.05f;

	// 足元から下方向に探す距離
	float m_rayRange = 1.0f;

	// ===== sphere =====

	// 壁判定Sphereの半径
	float m_collisionRadius = 0.75f;
};