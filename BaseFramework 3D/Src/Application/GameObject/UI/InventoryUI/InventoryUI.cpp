#include "InventoryUI.h"

#include "../../Character/Player/Player.h"

#include "../../Object/StaticMeshObject/Item/Item.h"

#include "../../Object/StaticMeshObject/Item/ItemData.h"

void InventoryUI::Init()
{
	m_keyTex.clear();
	m_keyTex.resize(Inventory::MaxItem);
	for (int i = 0; i < Inventory::MaxItem; ++i)
	{
		m_keyTex[i] = LoadTexture(
			"Asset/Textures/UI/Key/keyboard_" + std::to_string(i + 1) + ".png");
	}
}

void InventoryUI::DrawSprite()
{
	auto player = LockPlayer();

	if (!player)
	{
		return;
	}

	const Inventory& inventory = player->GetInventory();

	const auto& items = inventory.GetItems();

	const int selectIndex = inventory.GetSelectIndex();

	for (int i = 0; i < Inventory::MaxItem; ++i)
	{
		DrawSlot(
			i,
			items[i],
			i == selectIndex);
	}
}

void InventoryUI::DrawSlot(
	int index,
	const InventoryItem& item,
	bool selected)
{
	const int x = StartX + index * (SlotSize + SlotSpace);
	const int y = StartY;

	// --- 危険アイテム判定 ---
	bool dangerous = false;
	if (!item.IsEmpty() && !item.itemPath.empty())
	{
		const ItemData* data = ItemData::Get(item.itemPath);
		dangerous = (data && data->IsDangerous());
	}

	// --- 色の選択 ---
	const Math::Color* bgColor;
	const Math::Color* borderColor;

	if (dangerous)
	{
		bgColor = selected ? &kSlotDangerSelectedBgColor : &kSlotDangerBgColor;
		borderColor = &kSlotDangerBorderColor;
	}
	else
	{
		bgColor = selected ? &kSlotSelectedBgColor : &kSlotBgColor;
		borderColor = selected ? &kSlotSelectedBorderColor : &kSlotBorderColor;
	}

	// --- スロット背景（DrawBox）---
	const int drawSize = selected ? SelectedSlotSize : SlotSize;
	const int half = drawSize / 2;

	auto& sprite = KdShaderManager::Instance().m_spriteShader;

	sprite.DrawBox(x, y, half, half, bgColor, true);      // 塗り
	sprite.DrawBox(x, y, half, half, borderColor, false); // 枠

	// 選択時は枠を重ねて太く
	if (selected)
	{
		sprite.DrawBox(x, y, half - 1, half - 1, borderColor, false);
		sprite.DrawBox(x, y, half - 2, half - 2, borderColor, false);
	}

	// --- アイテムアイコン（空でなければ）---
	if (!item.IsEmpty())
	{
		DrawItemIcon(index, item, selected);
	}

	// --- キーアイコン（空スロットでも常に表示）---
	DrawKeyIcon(index, selected);
}

void InventoryUI::DrawItemIcon(
	int index,
	const InventoryItem& item,
	bool selected)
{
	if (item.itemPath.empty())
	{
		return;
	}

	auto it = m_iconTextures.find(item.itemPath);

	if (it == m_iconTextures.end())
	{
		const ItemData* data = ItemData::Get(item.itemPath);

		if (!data)
		{
			return;
		}

		const std::string& iconPath = data->GetIconPath();

		if (iconPath.empty())
		{
			return;
		}

		auto texture = std::make_shared<KdTexture>();

		if (!texture->Load(iconPath))
		{
			return;
		}

		it = m_iconTextures.emplace(
			item.itemPath,
			texture).first;
	}

	const int x = StartX + index * (SlotSize + SlotSpace);
	const int y = StartY;

	auto& sprite = KdShaderManager::Instance().m_spriteShader;

	sprite.DrawTex(
		it->second,
		x,
		y,
		IconSize,
		IconSize);

	// --- 危険アイテムのバッジ ---
	const ItemData* data = ItemData::Get(item.itemPath);
	if (data && data->IsDangerous())
	{
		// 実際に描画されているスロットサイズを基準にする
		const int drawSize = selected ? SelectedSlotSize : SlotSize;
		const int half = drawSize / 2;

		// 角から「バッジ半径 + 余白」だけ内側に置く
		const int bx = x + half - BadgeRadius - BadgeMargin;
		const int by = y + half - BadgeRadius - BadgeMargin;

		const Math::Color badgeColor{ 0.8f, 0.2f, 0.2f, 1.0f };

		sprite.DrawCircle(bx, by, BadgeRadius, &badgeColor, true);
		sprite.DrawCircle(bx, by, BadgeRadius, &kWhiteColor, false);

		DrawLabel(
			Math::Vector2{ static_cast<float>(bx) + 5.0f, static_cast<float>(by) },
			kWhiteColor,
			FontID::Future32,
			"!");
	}
}

void InventoryUI::DrawKeyIcon(int index, bool selected)
{
	if (index >= static_cast<int>(m_keyTex.size())) return;

	const auto& texPtr = m_keyTex[index];
	if (!texPtr || !texPtr->GetInfo().Width) return;

	auto& sprite = KdShaderManager::Instance().m_spriteShader;

	// 縦横比維持
	const float iconW = KeyIconH *
		static_cast<float>(texPtr->GetInfo().Width) /
		static_cast<float>(texPtr->GetInfo().Height);

	// スロット中央X / スロット上端のさらに上（拡大前のSlotSizeを基準にして固定）
	const int x = StartX + index * (SlotSize + SlotSpace);
	const int y = StartY + SlotSize / 2 + KeyGap + KeyIconH / 2;

	const Math::Color color = selected
		? Math::Color{ 1.0f, 1.0f, 1.0f, 1.0f }
	: Math::Color{ 1.0f, 1.0f, 1.0f, KeyDimAlpha };

	sprite.DrawTex(texPtr, x, y,
		static_cast<int>(iconW), KeyIconH,
		nullptr, &color, Math::Vector2{ 0.5f, 0.5f });
}
