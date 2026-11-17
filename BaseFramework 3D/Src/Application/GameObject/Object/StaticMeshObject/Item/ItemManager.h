#pragma once

class Item;
class ItemSpawner;

class ItemManager
{
public:
	void Update();

	void Reset();

	// Spawner登録
	void RegisterSpawner(const std::shared_ptr<ItemSpawner>& spawner);

	// ワールドに出したItem
	void RegisterWorldItem(const std::shared_ptr<Item>& item);

	// 捨てた時＆交換で落とした時
	std::shared_ptr<Item> SpawnDropped(
		const std::string& path,
		const Math::Vector3& pos,
		const Math::Vector3& dir);

	// 拾われた時
	void OnPickedUp(const Item& item);

	int CountInWorld(const std::string& path) const;
	int CountHeld(const std::string& path) const;

	// ワールド + 所持
	int CountTotal(const std::string& path) const
	{
		return CountInWorld(path) + CountHeld(path);
	}

	int GetMaxCount(const std::string& path);

	void SetDefaultMaxCount(int count)
	{
		m_defaultMaxCount = count;
	}

	void SetRespawnTime(float time)
	{
		m_respawnTime = time;
	}

private:

	ItemManager() = default;

	struct Pending
	{
		std::string path;
		float timer = 0.0f;
	};

	std::shared_ptr<ItemSpawner> FindFreeSpawner(const std::string& path);

	void PruneExpired();

	static std::string Normalize(std::string path);

	std::vector<Pending> m_pending;

	std::vector<std::weak_ptr<ItemSpawner>> m_spawners;

	std::vector<std::weak_ptr<Item>> m_worldItems;

	std::unordered_map<std::string, int> m_maxCountCache;

	int m_defaultMaxCount = 3;

	float m_respawnTime = 10.0f;

	// 出せなかった時の再判定間隔
	float m_retryInterval = 0.5f;


public:
	static ItemManager& Instance()
	{
		static ItemManager instance;
		return instance;
	}
};