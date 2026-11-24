#pragma once

#include "../CharacterBase.h"

#include "../../Camera/CameraBase.h"
#include "../../Camera/ArrestCamera/ArrestCamera.h"
#include "../../Camera/FPSCamera/FPSCamera.h"
class Item;
class Interactable;
class Usable;
class FPSCamera;
class ArrestCamera;
class DamageDirectionUI;

#include "../../../System/SkillCheck/SkillCheckBase.h"

class Player : public CharacterBase
{
public:

	enum class UsePhase
	{
		None,
		SkillCheck,
		Counting
	};

	Player() {}
	~Player() {}

	void Init() override;
	void Update() override;
	void PostUpdate() override;
	void DrawLit() override;
	void DrawSprite() override;

	const char* GetTypeName() const override
	{
		return "Player";
	}

	ObjectCategory GetCategory() const override
	{
		return ObjectCategory::Character;
	}

	void OnRepairSuccess(int score, const std::string& message);

	void AddScore(int score)
	{
		m_score += score;
	}

	int GetScore() const
	{
		return m_score;
	}

	void OnObjectDamaged(
		const std::shared_ptr<InteractiveObject>& obj,
		const InteractiveAction& action) override;

	void SetCamera(std::shared_ptr<CameraBase> camera)
	{
		camera->SetTarget(std::dynamic_pointer_cast<Player>(shared_from_this()));
		m_camera = camera;
		auto fpsCamera = std::dynamic_pointer_cast<FPSCamera>(camera);
		if (fpsCamera) m_fpsCamera = fpsCamera;
	}

	void EquipItem(const std::string& itemPath);

	void SwapItem(
		const std::shared_ptr<Item>& newItem,
		int index);

	void ClearHandItem();

	void ConsumeItem(int index);

	bool CanHighlight() ;

	std::shared_ptr<Item> GetHandItem() const
	{
		return m_handItem;
	}

	bool IsUsing() const
	{
		return m_isUsing;
	}

	bool IsSuspicious() const override;

	bool IsCounting() const { return m_usePhase == UsePhase::Counting; }

	float GetUseProgress() const
	{
		if (m_usePhase != UsePhase::Counting || m_useTotal <= 0.0f)
		{
			return 0.0f;
		}

		return std::clamp(m_useTimer / m_useTotal, 0.0f, 1.0f);
	}

	void CancelUse();

	SkillResult GetSkillResult() const
	{
		return m_skillResult;
	}

	std::shared_ptr<KdGameObject> GetFocusObject() const
	{
		return m_focusObject.lock();
	}

	void OnArrested(const std::shared_ptr<Police>& police) override;
	void OnReleased() override;

	int GetDamagedCount() const
	{
		return m_neglected.size();
	}
	void AddDamageTarget(const std::shared_ptr<InteractiveObject>& obj);
	void SetDamageUI(const std::shared_ptr<DamageDirectionUI>& ui)
	{
		m_damageUI = ui;
	}

	// 妨害UI用
	std::shared_ptr<CharacterBase> FindHinderTarget() const;

	bool  CanHinder() const
	{
		if (m_isHindering || m_isRestrained || m_isArrested || m_isHinderCamReturning) return false;
		if (m_hinderCooldownTimer > 0.0f) return false;
		return FindHinderTarget() != nullptr;
	}

	float GetHinderRatio() const
	{
		if (!m_isHindering) return 0.0f;
		return std::clamp(m_hinderRemaining / kHinderBaseTime, 0.0f, 1.0f);
	}

	float GetReportRemaining() const { return m_reportTimer; }
	float GetReportCooldownRatio() const
	{
		if (kReportCooldown <= 0.0f) return 0.0f;
		return std::clamp(m_reportTimer / kReportCooldown, 0.0f, 1.0f);
	}

	bool CanDeliver() const;
	float GetRepairBonusRate() const { return 1.0f + 0.2f * m_bonusCount; }
	int GetRepairBonusCount() const { return m_bonusCount; }
	float GetRepairBonusTime() const { return m_bonusTime; }
	float GetRepairBonusTimeRatio() const { return m_bonusTime / kBonusMaxTime; }
	int DeliverDangerousItems();

	bool IsGhost() const override
	{
		return m_isHindering;
	}

protected:

	void UpdateAnimation() override;

private:

	void UpdateLocked();

	void Move();

	void UpdateRotationByCamera();

	void UpdateFocusObject();

	void ClearFocus();

	void TryInteract();

	void UpdateInventorySelect();

	void StartUse();
	void UpdateUse();

	void UpdateArrested();

	void SwitchToArrestCamera();
	void RestoreFpsCamera(const std::shared_ptr<ArrestCamera>& arrestCam);

	void UpdateNeglect();

	void UpdateReport();
	bool TryReport() const;
	bool FindReportPos(Math::Vector3& outPos) const;
	bool IsVisibleFromCamera(
		const Math::Vector3& targetPos,
		const KdGameObject* ignore) const;

	void UpdateHinder();
	bool TryBeginHinder();
	void EndHinder(bool restoreCamera, bool applyCooldown);
	void BeginHinderCamera();
	void UpdateHinderCamera();

	bool TryDeliver();
	void UpdateRepairBonus();

	void OnItemsConfiscated() override;
	void SelectInventory(int index) override;
	void TryDropItem();

	void UpdateHandItem() const;

	void PlayAnimation(const std::string& animName);

	void SpawnFloatingText(
		const std::string& text,
		const Math::Vector2& pos,
		const Math::Color& color,
		FontID font = FontID::Future48) const;

	static constexpr float TimeRate(SkillResult result)
	{
		switch (result)
		{
		case SkillResult::Perfect: return 0.5f;	// 半分に短縮
		case SkillResult::Good:    return 1.0f;	// 通常
		case SkillResult::Miss:    return 1.5f;	// 延長(無防備な時間が増える)
		default:                   return 1.0f;
		}
	}

	int m_score = 0;

	static constexpr float kMoveSpeed = 0.15f;

	std::string m_currentAnimName;

	std::weak_ptr<CameraBase> m_camera;

	bool m_isDrawEnabled = false;

	std::weak_ptr<KdGameObject> m_focusObject;

	std::shared_ptr<Item> m_handItem;

	bool m_isUsing = false;

	static constexpr float kSuspicionSeconds = 30.0f;	// 壊れてからこの秒数以内の物を直すと疑われる

	float m_useTimer = 0.0f;
	float m_useTime = 0.0f;
	float m_useTotal = 0.0f;		// 補正後のカウント全体時間

	std::weak_ptr<Usable> m_useObject;

	UsePhase m_usePhase = UsePhase::None;
	std::unique_ptr<SkillCheckBase> m_skillCheck;
	SkillResult m_skillResult = SkillResult::None;
	int m_lastSkillType = -1;	// 直前の種類

	bool m_isReleasing = false;

	std::shared_ptr<FPSCamera> m_fpsCamera;

	bool m_isHinderCamReturning = false;   // TPS → FPS に戻している最中

	// ===== 放置ペナルティ =====
	struct NeglectedEntry
	{
		std::weak_ptr<InteractiveObject> obj;
		float elapsed = 0.0f;     // いたずらされてからの経過秒
		float tickTimer = 0.0f;   // 減点間隔用
	};

	std::vector<NeglectedEntry> m_neglected;
	std::weak_ptr<DamageDirectionUI> m_damageUI;

	static constexpr float kNeglectGrace = 10.0f;  // 猶予
	static constexpr float kNeglectInterval = 1.0f;   // 減点間隔
	static constexpr int   kNeglectPenalty = 1;      // 1回あたりの減点

	// 通報
	bool m_isReportKeyPressed = false;
	static constexpr float kReportCooldown = 10.0f;
	float m_reportTimer = 0.0f;    // 残りクールダウン
	static constexpr float kReportWindowSeconds = 60.0f;	// 通報の対象にする最近壊された時間
	static constexpr float kReportPrankRange = 5.0f;	// 通報できるプレイヤーからの距離

	// 妨害
	std::weak_ptr<CharacterBase> m_wpHinderTarget;
	float m_hinderRemaining = 0.0f;

	static constexpr float kHinderBaseTime = 3.0f;    // 基本拘束時間
	static constexpr float kHinderMashExtend = 0.25f;   // 1回目の連打で延びる秒数
	static constexpr float kHinderMashDecay = 0.96f;   // 1回押すごとの減衰率
	int m_hinderMashCount = 0;	// 今回の妨害での連打回数
	static constexpr float kHinderRange = 2.0f;    // 妨害可能距離

	static constexpr float kHinderCooldown = 5.0f;    // 妨害後のクールダウン（調整用）
	float m_hinderCooldownTimer = 0.0f;

	// 届
	static constexpr float kDeliverRadius = 2.0f;
	int m_bonusCount = 0;
	float m_bonusTime = 0.0f;
	static constexpr int   kBonusMaxCount = 5;
	static constexpr float kBonusExtend = 20.0f;
	static constexpr float kBonusMaxTime = 60.0f;
};