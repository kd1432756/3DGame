#include "InventoryUI.h"

#include "../../../Scene/SceneManager.h"

#include "../../Character/Player/Player.h"

#include "../../Object/StaticMeshObject/Item/Item.h"

#include "../../Object/StaticMeshObject/Item/ItemData.h"

void InventoryUI::Init()
{
	m_slotTex = std::make_shared<KdTexture>();
	m_slotTex->Load("Asset/Textures/UI/InventorySlot.png");
	m_slotSelectedTex = std::make_shared<KdTexture>();
	m_slotSelectedTex->Load("Asset/Textures/UI/InventorySlotSelected.png");
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
	const int x =
		StartX +
		index * (SlotSize + SlotSpace);

	const int y = StartY;

	// --------------------------------
	// スロット背景
	// --------------------------------

	std::shared_ptr<KdTexture> texture;

	int drawSize = SlotSize;

	if (selected)
	{
		texture = m_slotSelectedTex;
		drawSize = SelectedSlotSize;
	}
	else
	{
		texture = m_slotTex;
	}

	if (texture)
	{
		KdShaderManager::Instance().m_spriteShader.DrawTex(
			texture,
			x,
			y,
			drawSize,
			drawSize);
	}

	// --------------------------------
	// 空スロット
	// --------------------------------

	if (item.IsEmpty())
	{
		return;
	}

	// --------------------------------
	// アイテムアイコン
	// --------------------------------

	DrawItemIcon(index, item);

	// --------------------------------
	// スロット番号
	// --------------------------------

	const std::string number =
		std::to_string(index + 1);

	const float halfSize =
		drawSize / 2.0f;

	KdShaderManager::Instance().m_spriteShader.DrawFont(
		Math::Vector2{
			x - halfSize + 5.0f,
			y + halfSize - 5.0f
		},
		&kWhiteColor,
		Math::Vector2{ 0.35f, 0.35f },
		0.0f,
		FontID::Future32,
		number.c_str());
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
}