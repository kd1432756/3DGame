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

	std::unordered_map<std::string, std::shared_ptr<KdTexture>> m_iconTextures;

	std::vector<std::shared_ptr<KdTexture>> m_keyTex;

	static constexpr int kStartX = -148;
	static constexpr int kStartY = -250;

	static constexpr int kSlotSize = 128;
	static constexpr int kSelectedSlotSize = 144;
	static constexpr int kSlotSpace = 20;

	static inline const Math::Color kSlotBgColor{ 0.90f, 0.90f, 0.90f, 0.85f };
	static inline const Math::Color kSlotSelectedBgColor{ 1.00f, 1.00f, 1.00f, 0.95f };
	static inline const Math::Color kSlotBorderColor{ 0.25f, 0.25f, 0.25f, 0.80f };
	static inline const Math::Color kSlotSelectedBorderColor{ 1.00f, 0.65f, 0.10f, 1.00f };

	static inline const Math::Color kSlotDangerBgColor{ 1.00f, 0.84f, 0.84f, 0.88f };
	static inline const Math::Color kSlotDangerSelectedBgColor{ 1.00f, 0.78f, 0.78f, 0.95f };
	static inline const Math::Color kSlotDangerBorderColor{ 0.80f, 0.20f, 0.20f, 0.90f };

	static constexpr int kBadgeRadius = 16;

	static constexpr int kIconSize = 80;

	static constexpr int kBadgeMargin = 4;

	static constexpr int kKeyIconH = 28;
	static constexpr int kKeyGap = 12;
	static constexpr float kKeyDimAlpha = 0.55f;
};
