#pragma once

struct InventoryItem
{
	std::string itemPath;

	bool IsEmpty() const
	{
		return itemPath.empty();
	}
};

class Inventory
{
public:

	static constexpr int MaxItem = 3;

	bool AddItem(const std::string& itemPath);

	void RemoveItem(int index);

	void SelectNext();

	void SelectPrev();

	void SelectIndex(int index);

	const InventoryItem* GetCurrentItem() const;

	const std::array<InventoryItem, MaxItem>& GetItems() const
	{
		return m_items;
	}

	int GetSelectIndex() const
	{
		return m_selectIndex;
	}

	int GetItemCount() const;

	bool IsEmpty() const
	{
		return GetItemCount() == 0;
	}

	int CountItems(const std::string& itemPath);

	void Clear();

private:

	std::array<InventoryItem, MaxItem> m_items;

	int m_selectIndex = -1;
};