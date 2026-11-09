#pragma once

#include "../PlayerUIBase/PlayerUIBase.h"

#include "../../../System/Inventory/Inventory.h"

class InventoryUI : public PlayerUIBase
{
public:
	void Init() override;
	void DrawSprite() override;

private:

	void DrawSlot(
		int index,
		const InventoryItem& item,
		bool selected);

	void DrawItemIcon(
		int index,
		const InventoryItem& item,
		bool selected);

	void DrawKeyIcon(int index, bool selected);

private:

	// ItemDataのパス → アイコンテクスチャ
	std::unordered_map<std::string, std::shared_ptr<KdTexture>> m_iconTextures;

	std::vector<std::shared_ptr<KdTexture>> m_keyTex;   // 数字キー画像（スロット数ぶん）

	// 表示位置
	static constexpr int StartX = -148;
	static constexpr int StartY = -250;

	// スロット
	static constexpr int SlotSize = 128;
	static constexpr int SelectedSlotSize = 144;
	static constexpr int SlotSpace = 20;

	// 通常アイテム / 空
	static inline const Math::Color kSlotBgColor{ 0.90f, 0.90f, 0.90f, 0.85f };
	static inline const Math::Color kSlotSelectedBgColor{ 1.00f, 1.00f, 1.00f, 0.95f };
	static inline const Math::Color kSlotBorderColor{ 0.25f, 0.25f, 0.25f, 0.80f };
	static inline const Math::Color kSlotSelectedBorderColor{ 1.00f, 0.65f, 0.10f, 1.00f };

	// 危険アイテム（ほんのり赤）
	static inline const Math::Color kSlotDangerBgColor{ 1.00f, 0.84f, 0.84f, 0.88f };
	static inline const Math::Color kSlotDangerSelectedBgColor{ 1.00f, 0.78f, 0.78f, 0.95f };
	static inline const Math::Color kSlotDangerBorderColor{ 0.80f, 0.20f, 0.20f, 0.90f };

	static constexpr int BadgeRadius = 16;

	// アイコン
	static constexpr int IconSize = 80;

	// 枠線ぶんの内側余白
	static constexpr int BadgeMargin = 4;

	// キーアイコン
	static constexpr int   KeyIconH = 28;       // キーアイコンの高さ
	static constexpr int   KeyGap = 12;         // スロットとキーの間隔
	static constexpr float KeyDimAlpha = 0.55f; // 非選択時の透明度
};
