#pragma once

class Item;

class ItemSpawner :	public KdGameObject
{
public:

	void Update() override;

	// 何のアイテムを出すか
	void SetItemPath(const std::string& path)
	{
		m_itemPath = path;
	}

	// どこに出すか
	void SetSpawnPos(const Math::Vector3& pos)
	{
		m_spawnPos = pos;
	}

	// どの大きさで出すか
	void SetSpawnScale(const Math::Vector3& scale)
	{
		m_spawnScale = scale;
	}

	// リスポーン時間
	void SetRespawnTime(float time)
	{
		m_respawnTime = time;
	}

	// リスポーン開始
	void StartRespawn();

private:

	void SpawnItem();

	std::string m_itemPath;

	Math::Vector3 m_spawnPos = {};

	Math::Vector3 m_spawnScale = { 1, 1, 1 };

	float m_respawnTime = 10.0f;
	float m_respawnTimer = 0.0f;

	bool m_respawnTimerFlg = false;
};