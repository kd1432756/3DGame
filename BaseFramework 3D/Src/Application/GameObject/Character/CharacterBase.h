#pragma once

class Police;

class InteractiveObject;
class InteractiveAction;

#include "../Object/StaticMeshObject/Ground/Ground.h"

#include "../../System/Inventory/Inventory.h"
#include "../Object/StaticMeshObject/Item/ItemData.h"

#include "../../System/FootStep/FootStepPhaseTable.h"

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

	void Destroy() override
	{
		m_isExpired = true;
		m_spEmoteIcon->Destroy();
	}

	// 表示用の名前（リザルト・通知UIなど）
	const std::string& GetDisplayName() const { return m_displayName; }
	void SetDisplayName(const std::string& name) { m_displayName = name; }

	// 未設定なら内部Nameにフォールバック
	const std::string& GetDisplayNameOrName() const
	{
		return m_displayName.empty() ? GetName() : m_displayName;
	}

	// いたずらされた
	virtual void OnObjectDamaged(
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
	bool HasDangerousItem() const;
	void ConfiscateDangerousItems();

	// 選択スロット変更（Player=手持ちモデル更新 / Neighbor=ItemData更新）
	virtual void SelectInventory(int index) { m_inventory.SelectIndex(index); }

	bool IsArrested() const { return m_isArrested; }

	// NPC: 破壊作業中(妨害・警察の聞き込みの対象)
	virtual bool IsDamaging() const { return false; }
	virtual Math::Vector3 GetDamagePos() const { return m_pos; }

	// 警察に疑われる行動中(Player: 壊れて間もない物の修復中)
	virtual bool IsSuspicious() const { return false; }

	// 拘束（移動不可・操作可）
	bool IsRestrained() const { return m_isRestrained; }
	void SetRestrained(bool isRestrained) { m_isRestrained = isRestrained; }

	// 妨害中（プレイヤー側。警察の追跡対象から外す）
	bool IsHindering() const { return m_isHindering; }
	void SetHindering(bool isHindering) { m_isHindering = isHindering; }

	void ShowEmote(EmoteIcon::IconType type, float duration = 0.0f, bool force = false);
	void HideEmote();

	float GetBodyRadius() const { return m_bodyRadius; }
	virtual bool IsGhost() const { return false; }

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

	void CheckGround();

	void SlideAlongWalls(Math::Vector3& move, float radius);

	void CollectWallHits(float radius, std::vector<WallHit>& out);
	void ResolveCollision();

	void ApplyGravity();

	void RotateToMoveDirection(const Math::Vector3& dir);

	Math::Vector3 GetFacingDir() const
	{
		const Math::Matrix rot = Math::Matrix::CreateRotationY(DirectX::XMConvertToRadians(m_rot.y));

		Math::Vector3 dir = Math::Vector3::TransformNormal(Math::Vector3(0, 0, -1), rot);
		dir.y = 0.0f;
		dir.Normalize();
		return dir;
	}

	virtual void UpdateAnimation();

	std::shared_ptr<KdModelWork>				m_spModel = nullptr;
	std::shared_ptr<KdAnimator>					m_spAnimator = nullptr;

	float m_sceneScale = 1.0f;

	float m_gravity = 0.0f;

	float m_gravityPower = 0.015f;

	bool m_isGround = false;

	Math::Vector3 m_lastGroundPos = Math::Vector3::Zero;
	bool m_hasLastGround = false;
	static constexpr float m_fallLimit = 20.0f;

	std::string m_displayName;

	virtual void OnItemsConfiscated() {}

	Inventory m_inventory;

	bool m_isArrested = false;

	void InitEmoteIcon();

	std::shared_ptr<EmoteIcon> m_spEmoteIcon = nullptr;

	Ground::GroundType m_groundType = Ground::GroundType::None;

	void UpdateFootstepByAnim(const KdAnimator& animator);
	const std::vector<float>* GetStepPhases(const KdAnimationData* data) const;

	bool  m_isMoving = false;     // 今フレーム実際に動いたか
	float m_prevPhase = 0.0f;
	const KdAnimationData* m_prevAnimData = nullptr;
	int   m_stepCount = 0;

	bool m_isRestrained = false;
	bool m_isHindering = false;

	float m_bodyRadius = 0.6f;
	float m_bodyHeight = 1.5f;   // Y差がこの範囲内のときだけ判定

	// ===== ray =====
	
	// 最大で登れる段差
	float m_stepHeight = 1.0f;

	// m_posから足元までの距離
	float m_footOffset = 0.8f;

	// ===== sphere =====

	// 壁判定Sphereの半径
	float m_collisionRadius = 0.5f;

	float WallRadius() const { return m_collisionRadius * m_sceneScale; }
};