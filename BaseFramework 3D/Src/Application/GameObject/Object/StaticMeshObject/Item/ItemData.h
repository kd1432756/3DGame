#pragma once

enum class ItemAction
{
	None,
	Repair,
	Cut,
	Break,
	Wrench,
	Pierce
};

struct ItemUse
{
	ItemAction  action = ItemAction::None;
	std::string target;
	std::string actionText;
};

class ItemData
{
public:

	bool Load(const std::string& path);

	// キャッシュ経由で取得
	static const ItemData* Get(const std::string& path);

	static void ClearCache();

	const std::string& GetName() const { return m_name; }
	const std::string& GetModelPath() const { return m_modelPath; }
	const std::string& GetIconPath() const { return m_iconPath; }
	const std::string& GetItemPath() const { return m_itemPath; }

	const Math::Vector3& GetWorldScale() const { return m_worldScale; }
	const Math::Vector3& GetHandOffset() const { return m_handOffset; }
	const Math::Vector3& GetHandRotation() const { return m_handRotation; }
	const Math::Vector3& GetHandScale() const { return m_handScale; }

	int GetMaxCount() const { return m_maxCount; }

	const std::vector<ItemUse>& GetUses() const { return m_uses; }

	// 指定の (action, target) で使えるか
	const ItemUse* FindUse(ItemAction action, const std::string& target) const
	{
		for (const auto& use : m_uses)
		{
			if (use.action == action && use.target == target)
			{
				return &use;
			}
		}
		return nullptr;
	}

	bool HasRepairUse() const
	{
		for (const auto& use : m_uses)
		{
			if (use.action == ItemAction::Repair) return true;
		}
		return false;
	}

	bool HasPrankUse() const
	{
		for (const auto& use : m_uses)
		{
			if (use.action != ItemAction::Repair &&
				use.action != ItemAction::None) return true;
		}
		return false;
	}

	// 危険物フラグ
	bool IsDangerous() const { return m_dangerous; }

private:

	static ItemAction ParseAction(const std::string& str);
	
	std::string m_itemPath;
	std::string m_name;
	std::string m_modelPath;
	std::string m_iconPath;

	std::vector<ItemUse> m_uses;

	bool m_dangerous = false;

	Math::Vector3 m_worldScale = { 1,1,1 };

	Math::Vector3 m_handOffset = Math::Vector3::Zero;
	Math::Vector3 m_handRotation = Math::Vector3::Zero;
	Math::Vector3 m_handScale = { 1,1,1 };

	int m_maxCount = -1;
};