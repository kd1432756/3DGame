#pragma once

class Item;

//============================================================
// ItemSpawner
//
// 「アイテムの出現地点」。
// タイマー・上限判定・出現のタイミングは ItemManager が持つ。
//============================================================
class ItemSpawner : public KdGameObject
{
public:

	// Managerへの登録のみ（初回1回）
	void Update() override;

	//--------------------------------------------------------
	// 設定
	//--------------------------------------------------------

	// 何のアイテムを出すか
	void SetItemPath(const std::string& path)
	{
		m_itemPath = path;

		// Inventory / ItemData と同じ表記に揃える
		std::replace(m_itemPath.begin(), m_itemPath.end(), '\\', '/');
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

	const std::string& GetItemPath() const
	{
		return m_itemPath;
	}

	//--------------------------------------------------------
	// Managerから使う
	//--------------------------------------------------------

	// 今この地点にItemが出ていないか
	bool IsFree() const;

	// Itemを出す（Managerが上限・タイミングを判断済み）
	void SpawnItem();

private:

	std::string m_itemPath;

	Math::Vector3 m_spawnPos = {};

	Math::Vector3 m_spawnScale = { 1, 1, 1 };

	// 今この地点に出ているItem
	std::weak_ptr<Item> m_current;

	bool m_registered = false;
};