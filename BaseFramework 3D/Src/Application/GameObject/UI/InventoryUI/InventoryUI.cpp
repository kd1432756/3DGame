#include "InventoryUI.h"

#include "../../../Scene/SceneManager.h"

#include "../../Character/Player/Player.h"

#include "../../Object/StaticMeshObject/Item/Item.h"

#include "../../Object/StaticMeshObject/Item/ItemData.h"

void InventoryUI::Init()
{
/*	m_slotTex = std::make_shared<KdTexture>();
	m_slotTex->Load("Asset/Textures/UI/InventorySlot.png");
	m_slotSelectedTex = std::make_shared<KdTexture>();
	m_slotSelectedTex->Load("Asset/Textures/UI/InventorySlotSelected.png");*/

	m_keyTex.clear();
	m_keyTex.resize(Inventory::MaxItem);
	for (int i = 0; i < Inventory::MaxItem; ++i)
	{
		const std::string path =
			"Asset/Textures/UI/Key/keyboard_" + std::to_string(i + 1) + ".png";

		m_keyTex[i] = std::make_shared<KdTexture>();
		m_keyTex[i]->Load(path);
	}
}

void InventoryUI::Update()
{
	auto player = m_wpPlayer.lock();

	if (!player)
	{
		return;
	}
}

void InventoryUI::DrawSprite()
{
	auto player = m_wpPlayer.lock();

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
/*	const int x = StartX + index * (SlotSize + SlotSpace);
	const int y = StartY;

	// --- スロット背景 ---
	std::shared_ptr<KdTexture> texture = selected ? m_slotSelectedTex : m_slotTex;
	const int drawSize = selected ? SelectedSlotSize : SlotSize;

	if (texture)
	{
		KdShaderManager::Instance().m_spriteShader.DrawTex(
			texture, x, y, drawSize, drawSize);
	}*/

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
		DrawItemIcon(index, item);
	}

	// --- キーアイコン（空スロットでも常に表示）---
	DrawKeyIcon(index, selected);
}

void InventoryUI::DrawItemIcon(
	int index,
	const InventoryItem& item)
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

	const int x =
		StartX +
		index * (SlotSize + SlotSpace);

	const int y = StartY;

	KdShaderManager::Instance().m_spriteShader.DrawTex(
		it->second,
		x,
		y,
		IconSize,
		IconSize);

	{
		const ItemData* data = ItemData::Get(item.itemPath);

		bool dangerous = data->IsDangerous();

		if (dangerous)
		{
			// スロット右上（Y上向き）
			const int half = SlotSize / 2;
			const int bx = x + half - BadgeRadius;
			const int by = y + half - BadgeRadius;

			const Math::Color badgeColor = Math::Color{ 0.8f, 0.2f, 0.2f, 1.0f };

			KdShaderManager::Instance().m_spriteShader.DrawCircle(bx, by, BadgeRadius, &badgeColor, true);
			KdShaderManager::Instance().m_spriteShader.DrawCircle(bx, by, BadgeRadius, &kWhiteColor, false);   // 白縁で背景から浮かせる

			int offsetX = 5;

			KdShaderManager::Instance().m_spriteShader.DrawFont(
				Math::Vector2{ static_cast<float>(bx) + offsetX, static_cast<float>(by) },
				&kWhiteColor,
				Math::Vector2{ 0.5f, 0.5f },
				0.0f,
				FontID::Future32,
				"!");
		}
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