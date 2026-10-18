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

	bool IsPranking() const
	{
		if (!m_isUsing)
		{
			return false;
		}

		auto usable = m_useObject.lock();
		if (!usable)
		{
			return false;
		}

		return !m_isRepairUse;
	}

	Math::Vector3 GetPrankPos() const
	{
		return m_prankPos;
	}

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

	Math::Vector3 m_prankPos = Math::Vector3::Zero;

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
};