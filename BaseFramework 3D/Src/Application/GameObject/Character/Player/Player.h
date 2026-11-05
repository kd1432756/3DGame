#pragma once

#include "../CharacterBase.h"

class CameraBase;
class Item;
class Interactable;
class Usable;
class FPSCamera;

#include "../../../System/Skillcheck/SkillCheckBase.h"

class SkillCheckFactory;

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

	void OnPrankSuccess(
		int score,
		const std::string& message) override;

	void OnPranked(
		const std::shared_ptr<InteractiveObject>& object,
		const InteractiveAction& action) override;

	void SetCamera(std::shared_ptr<CameraBase> _camera) { m_camera = _camera; }

	void EquipItem(const std::string& itemPath);

	void SwapItem(
		const std::shared_ptr<Item>& newItem,
		int index);

	void ConsumeItem(int index);

	bool CanHighlight();

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

	float TimeRate(SkillResult result)
	{
		switch (result)
		{
		case SkillResult::Perfect: return 0.5f;	// 半分に短縮
		case SkillResult::Good:    return 1.0f;	// 通常
		case SkillResult::Miss:    return 1.5f;	// 延長(無防備な時間が増える)
		default:                   return 1.0f;
		}
	}
	
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

	int GetDamagedCount() const { return m_neglected.size(); }

	// 妨害UI用
	std::shared_ptr<CharacterBase> FindHinderTarget() const;

	bool  CanHinder() const 
	{
		if (m_isHindering || m_isRestrained || m_isArrested || m_isHinderCamReturning)
			return false;
		if (m_hinderCooldownTimer > 0.0f) return false;
		return FindHinderTarget() != nullptr;
	}

	float GetHinderRatio() const
	{
		if (!m_isHindering) return 0.0f;
		return std::clamp(m_hinderRemaining / m_hinderBaseTime, 0.0f, 1.0f);
	}

	float GetReportRemaining() const { return m_reportTimer; }
	float GetReportCooldownRatio() const
	{
		if (m_reportCooldown <= 0.0f) return 0.0f;
		return std::clamp(m_reportTimer / m_reportCooldown, 0.0f, 1.0f);
	}

	bool IsGhost() const override 
	{ 
		if (m_isHindering)return true;		
		return false; 
	}

protected:

	void UpdateAnimation() override;

private:

	void Move();
	void Jump();

	void UpdateRotationByCamera();

	void UpdateFocusObject();

	void TryInteract();

	void UpdateInventorySelect();

	void StartUse();
	void UpdateUse();

	void UpdateArrested();

	void UpdateNeglect();

	void UpdateReport();
	bool TryReport();
	bool FindReportPos(Math::Vector3& outPos) const;
	void ShowReportText(const std::string& text, const Math::Color& color);
	bool IsVisibleFromCamera(
		const Math::Vector3& targetPos,
		const KdGameObject* ignore) const;

	void UpdateHinder();
	bool TryBeginHinder();
	void EndHinder(bool restoreCamera, bool applyCooldown);
	void BeginHinderCamera();
	void UpdateHinderCamera();

	void OnItemsConfiscated() override;
	void SelectInventory(int index) override;
	void TryDropItem();

	void UpdateHandItem();

	void PlayAnimation(const std::string& animName);

	std::string m_currentAnimName;

	std::weak_ptr<CameraBase> m_camera;

	bool m_isDrawEnabled = false;

	std::weak_ptr<KdGameObject> m_focusObject;

	std::shared_ptr<Item> m_handItem;

	bool m_isUsing = false;
	bool m_isRepairUse = false;

	float m_suspicionSeconds = 30.0f;	// 壊れてからこの秒数以内の物を直すと疑われる

	float m_useTimer = 0.0f;
	float m_useTime = 0.0f;
	float m_useTotal = 0.0f;	// 補正後のカウント全体時間

	std::weak_ptr<Usable> m_useObject;

	UsePhase m_usePhase = UsePhase::None;
	std::unique_ptr<SkillCheckBase> m_skillCheck;
	SkillResult m_skillResult = SkillResult::None;
	int m_lastSkillType = -1;	// 直前の種類(連続同種を避ける)

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

	float m_neglectGrace = 10.0f;  // 猶予(秒)
	float m_neglectInterval = 1.0f;   // 減点間隔(秒)
	int   m_neglectPenalty = 1;      // 1回あたりの減点

	// 通報
	float m_reportCooldown = 30.0f;   // 初期値30秒
	float m_reportTimer = 0.0f;    // 残りクールダウン
	float m_reportWindowSeconds = 60.0f;	// 通報の対象にする「最近壊された」の範囲(秒)

	// 妨害
	std::weak_ptr<CharacterBase> m_wpHinderTarget;
	float m_hinderRemaining = 0.0f;

	float m_hinderBaseTime = 3.0f;    // 基本拘束時間
	float m_hinderMashExtend = 0.25f;   // 1回目の連打で延びる秒数
	float m_hinderMashDecay = 0.96f;   // 1回押すごとの減衰率（小さいほど早く伸びなくなる）
	int   m_hinderMashCount = 0;       // 今回の妨害での連打回数
	float m_hinderRange = 2.0f;    // 妨害可能距離（×sceneScale）

	float m_hinderCooldown = 5.0f;    // 妨害後のクールダウン（調整用）
	float m_hinderCooldownTimer = 0.0f;
};