#include "Inventory.h"

bool Inventory::AddItem(const std::string& itemPath)
{
	std::string normalizedPath = itemPath;

	std::replace(normalizedPath.begin(), normalizedPath.end(), '\\', '/');

	for (int i = 0; i < m_items.size(); ++i)
	{
		if (m_items[i].itemPath.empty())
		{
			m_items[i].itemPath = normalizedPath;

			return true;
		}
	}

	return false;
}

void Inventory::RemoveItem(int index)
{
	if (index < 0 || index >= MaxItem)
		return;

	m_items[index].itemPath.clear();
}

void Inventory::SelectNext()
{
	if (m_items.empty()) return;

	m_selectIndex =
		(m_selectIndex + 1) % (int)m_items.size();
}

void Inventory::SelectPrev()
{
	if (m_items.empty()) return;

	m_selectIndex--;

	if (m_selectIndex < 0)
	{
		m_selectIndex = (int)m_items.size() - 1;
	}
}

void Inventory::SelectIndex(int index)
{
	if (index == -1)
	{
		m_selectIndex = -1;
		return;
	}

	if (index < 0 || index >= (int)m_items.size())
	{
		return;
	}

	m_selectIndex = index;
}

const InventoryItem* Inventory::GetCurrentItem() const
{
	if (m_selectIndex < 0 || m_selectIndex >= MaxItem)
	{
		return nullptr;
	}

	if (m_items[m_selectIndex].itemPath.empty())
	{
		return nullptr;
	}

	return &m_items[m_selectIndex];
}

int Inventory::GetItemCount() const
{
	int count = 0;

	for (const auto& item : m_items)
	{
		if (!item.IsEmpty())
		{
			count++;
		}
	}

	return count;
}

int Inventory::CountItems(const std::string& itemPath)
{
	int count = 0;

	for (const auto& item : m_items)
	{
		if (item.itemPath == itemPath)
		{
			count++;
		}
	}

	return count;
}

void Inventory::Clear()
{
	for (auto& item : m_items)
	{
		item.itemPath.clear();
	}

	m_selectIndex = -1;
}
