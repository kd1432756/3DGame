#pragma once

class Police;

#include "../Object/StaticMeshObject/InteractiveObject/InteractiveObject.h"
#include "../../System/Inventory/Inventory.h"
#include "../Object/StaticMeshObject/Item/ItemData.h"

#include "../Object/StaticMeshObject/Ground/Ground.h"

#include "../UI/EmoteIcon/EmoteIcon.h"

class CharacterBase : public KdGameObject
{
public:
	struct WallHit
	{
		Math::Vector3 normal;
		float         overlap;
	};

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

	//============================================================
	// Inventory
	//============================================================
	Inventory& GetInventory() { return m_inventory; }
	const Inventory& GetInventory() const { return m_inventory; }

	bool AddInventoryItem(const std::string& itemPath);
	bool HasInventoryItem(const std::string& itemPath) const;
	bool HasRepairItem() const;
	bool HasPrankItem() const;
	bool HasDangerousItem() const;
	void ConfiscateDangerousItems();

	// 選択スロット変更（Player=手持ちモデル更新 / Neighbor=ItemData更新）
	virtual void SelectInventory(int index) { m_inventory.SelectIndex(index); }

	bool IsArrested() const { return m_isArrested; }

	virtual bool IsPranking() const { return false; }
	virtual Math::Vector3 GetPrankPos() const { return m_pos; }

	void ShowEmote(EmoteIcon::IconType type, float duration = 0.0f, bool force = false);
	void HideEmote();

protected:

	virtual void UpdateWorldMatrix() override
	{
		m_rot.x = 90.0f;
		m_rot.z = 0.0f;

		Math::Matrix rotMat =
			Math::Matrix::CreateFromYawPitchRoll(
				DirectX::XMConvertToRadians(m_rot.y),
				DirectX::XMConvertToRadians(m_rot.x),
				DirectX::XMConvertToRadians(m_rot.z));

		m_mWorld =
			Math::Matrix::CreateScale(m_scale) *
			rotMat *
			Math::Matrix::CreateTranslation(m_pos);
	}

	void CheckStep(
		const Math::Vector3& moveDir);

	void CheckGround();

	void CollectWallHits(float radius, std::vector<WallHit>& out);
	void ResolveCollision();

	void ApplyGravity();

	void RotateToMoveDirection(const Math::Vector3& dir);

	void UpdateAnimation();

	void UpdateFootstep(const Math::Vector3& movedVec);

	std::shared_ptr<KdModelWork>				m_spModel = nullptr;
	std::shared_ptr<KdAnimator>					m_spAnimator = nullptr;

	float m_sceneScale = 1.0f;

	float m_gravity = 0.0f;

	float m_gravityPower = 0.015f;

	bool m_isGround = false;

	float m_jumpPower = 0.30f;

	float m_angle = 0.0f;

	int m_score = 0;

	virtual void OnItemsConfiscated() {}

	Inventory m_inventory;

	bool m_isArrested = false;

	void InitEmoteIcon();

	std::shared_ptr<EmoteIcon> m_spEmoteIcon = nullptr;

	float m_stepDistance = 0.0f;
	float m_stepInterval = 0.8f;
	Ground::GroundType m_groundType = Ground::GroundType::None;

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

	Math::Vector3 m_wallNormal = Math::Vector3::Zero;
};