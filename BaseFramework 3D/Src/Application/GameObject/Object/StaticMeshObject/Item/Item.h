#pragma once

#include "../StaticMeshObject.h"
#include "../../../../System/Interact/Interact.h"
#include "ItemData.h"

enum class ItemMode
{
	Spawned,		// マップに存在
	InInventory,	// インベントリ内
	Dropped		// キャラが落とした
};

class Inventory;

class ItemSpawner;

class Item : public StaticMeshObject
			,public Interactable
{
public:

	void Init() override;
	void Update() override;
	void DrawLit() override;
	void GenerateDepthMapFromLight() override;

	const char* GetTypeName() const override
	{
		return "Item";
	}

	ObjectCategory GetCategory() const override
	{
		return ObjectCategory::Item;
	}

	bool LoadItem(const std::string& path);

	const ItemData& GetItemData() const
	{
		return m_itemData;
	}

	const std::string& GetItemPath() const
	{
		return m_itemData.GetItemPath();
	}

	void SetSpawner(const std::shared_ptr<ItemSpawner>& spawner)
	{
		m_spawner = spawner;
	}

	const std::weak_ptr<ItemSpawner>& GetSpawner()
	{
		return m_spawner;
	}

	void Interact(std::shared_ptr<Player> player) override;

	void CopyFrom(const std::shared_ptr<KdGameObject>& src)
	{
		KdGameObject::CopyFrom(src);

		auto item = std::dynamic_pointer_cast<Item>(src);

		if (!item) return;

		LoadItem(item->GetItemData().GetItemPath());
	}

	void SetMatrix(const Math::Matrix& world)
	{
		m_mWorld = world;
	}

	void SetMode(ItemMode mode)
	{
		m_mode = mode;
	}

	ItemMode GetMode() const
	{
		return m_mode;
	}

	bool Pickup(Inventory& inventory, int replaceIndex = -1);

	void Drop(const Math::Vector3& pos, const Math::Vector3& dir);

private:

	ItemData m_itemData;

	void RegisterCollider() override;

	void UpdateGravity();
	void UpdateFriction();
	void CheckGround();
	void ResolveCollision();
	void UpdateWorldMatrix();

	void UpdateDropTimer();

	ItemMode m_mode = ItemMode::Spawned;

	std::weak_ptr<ItemSpawner> m_spawner;

	Math::Vector3 m_velocity = Math::Vector3::Zero;

	float m_gravity = 0.0f;

	float m_gravityPower = 0.015f;

	bool m_isGround = false;

	float m_dropTimer = 0.0f;
	float m_dropTime = 30.0f;
};