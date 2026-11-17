#pragma once

class Item;

class ItemSpawner : public KdGameObject
{
public:

	void Update() override;

	// 何のアイテムを出すか
	void SetItemPath(const std::string& path)
	{
		m_itemPath = path;

		std::replace(m_itemPath.begin(), m_itemPath.end(), '\\', '/');
	}

	void SetSpawnPos(const Math::Vector3& pos)
	{
		m_spawnPos = pos;
	}

	void SetSpawnScale(const Math::Vector3& scale)
	{
		m_spawnScale = scale;
	}

	const std::string& GetItemPath() const
	{
		return m_itemPath;
	}

	// 今この地点にItemが出ていないか
	bool IsFree() const;

	void SpawnItem();

private:

	std::string m_itemPath;

	Math::Vector3 m_spawnPos = {};

	Math::Vector3 m_spawnScale = { 1, 1, 1 };

	std::weak_ptr<Item> m_current;

	bool m_registered = false;
};