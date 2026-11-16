#pragma once

class Item;
class ItemSpawner;

class ItemManager
{
public:
	void Update();

	void Reset();

	//--------------------------------------------------------
	// 登録
	//--------------------------------------------------------

	// Spawnerを登録（初回の出現待ちも積む）
	void RegisterSpawner(const std::shared_ptr<ItemSpawner>& spawner);

	// ワールドに出したItemを登録（数えるため）
	void RegisterWorldItem(const std::shared_ptr<Item>& item);

	//--------------------------------------------------------
	// 生成
	//--------------------------------------------------------

	// 捨てる・交換で落とす時
	std::shared_ptr<Item> SpawnDropped(
		const std::string& path,
		const Math::Vector3& pos,
		const Math::Vector3& dir);

	//--------------------------------------------------------
	// 通知
	//--------------------------------------------------------

	// 拾われた時
	void OnPickedUp(const Item& item);

	//--------------------------------------------------------
	// 数の取得
	//--------------------------------------------------------

	int CountInWorld(const std::string& path) const;
	int CountHeld(const std::string& path) const;

	// ワールド + 所持
	int CountTotal(const std::string& path) const
	{
		return CountInWorld(path) + CountHeld(path);
	}

	// 種類ごとの上限
	int GetMaxCount(const std::string& path);

	// デフォルト上限の設定
	void SetDefaultMaxCount(int count)
	{
		m_defaultMaxCount = count;
	}

	// 出現待ちの秒数
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

	// ワールド上のItem（Spawned / Dropped）
	std::vector<std::weak_ptr<Item>> m_worldItems;

	std::unordered_map<std::string, int> m_maxCountCache;

	int m_defaultMaxCount = 3;

	float m_respawnTime = 10.0f;

	// 上限や空き地点の都合で出せなかった時の再判定間隔（秒）
	float m_retryInterval = 0.5f;


public:
	static ItemManager& Instance()
	{
		static ItemManager instance;
		return instance;
	}
};