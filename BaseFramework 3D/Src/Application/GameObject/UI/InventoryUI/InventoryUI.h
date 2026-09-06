#pragma once

#include "../../../System/Inventory/Inventory.h"

class Player;

class InventoryUI : public KdGameObject
{
public:

	void Init() override;
	void Update() override;
	void DrawSprite() override;

	ObjectCategory GetCategory() const override
	{
		return ObjectCategory::UI;
	}

	void SetPlayer(const std::weak_ptr<Player>& player)
	{
		m_wpPlayer = player;
	}

private:

	void DrawSlot(
		int index,
		const InventoryItem& item,
		bool selected);

	void DrawItemIcon(
		int index,
		const InventoryItem& item);

private:

	std::weak_ptr<Player> m_wpPlayer;

	// ItemDataのパス → アイコンテクスチャ
	std::unordered_map<std::string, std::shared_ptr<KdTexture>> m_iconTextures;

	std::shared_ptr<KdTexture> m_slotTex = nullptr;
	std::shared_ptr<KdTexture> m_slotSelectedTex = nullptr;

	// 表示位置
	static constexpr int StartX = -148;
	static constexpr int StartY = -250;

	// スロット
	static constexpr int SlotSize = 128;
	static constexpr int SelectedSlotSize = 144;
	static constexpr int SlotSpace = 20;

	// アイコン
	//static constexpr int IconSize = 96;
	static constexpr int IconSize = 80;
};