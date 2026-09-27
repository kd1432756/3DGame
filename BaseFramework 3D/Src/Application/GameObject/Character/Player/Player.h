#pragma once

#include "../CharacterBase.h"

#include "../../../System/Inventory/Inventory.h"

class CameraBase;
class Item;
class Interactable;
class Usable;

class Player : public CharacterBase
{
public:
	Player() {}
	~Player() {}

	void Init() override;
	void Update() override;
	void PostUpdate() override;
	void DrawLit() override;

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

	Inventory& GetInventory()
	{
		return m_inventory;
	}

	void EquipItem(const std::string& itemPath);

	void SwapItem(
		const std::shared_ptr<Item>& newItem,
		int index);

	void SelectInventory(int index);

	bool CanHighlight();

	std::shared_ptr<Item> GetHandItem() const
	{
		return m_handItem;
	}

	bool IsUsing() const
	{
		return m_isUsing;
	}

	float GetUseProgress() const
	{
		if (!m_isUsing)
		{
			return 0.0f;
		}

		return m_useTimer / m_useTime;
	}

	std::shared_ptr<KdGameObject> GetFocusObject() const
	{
		return m_focusObject.lock();
	}

	bool IsInBush() const
	{
		return m_isInBush;
	}

	void OnArrested(const std::shared_ptr<Police>& police) override;

	bool HasPrankItem() const override;
	void ConfiscatePrankItems() override;

private:

	void Move();
	void Jump();

	void UpdateFocusObject();
	void TryInteract();
	void StartUse();
	void UpdateUse();

	void DropItem();

	void UpdateHandItem();

	void CheckBush();

	std::weak_ptr<CameraBase> m_camera;

	bool m_isDrawEnabled = false;

	std::weak_ptr<KdGameObject> m_focusObject;

	Inventory m_inventory;
	std::shared_ptr<Item> m_handItem;

	bool m_isUsing = false;

	float m_useTimer = 0.0f;
	float m_useTime = 0.0f;

	std::weak_ptr<Usable> m_useObject;

	bool m_isInBush = false;
};