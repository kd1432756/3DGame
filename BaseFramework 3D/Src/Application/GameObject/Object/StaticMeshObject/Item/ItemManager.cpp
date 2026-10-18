#include "ItemManager.h"

#include "../../../../Scene/SceneManager.h"
#include "../../../Character/CharacterBase.h"   // ※パスは環境に合わせて調整
#include "Item.h"
#include "ItemSpawner.h"

//============================================================
// Util
//============================================================

std::string ItemManager::Normalize(std::string path)
{
	// ItemData::Load と同じ正規化（Inventory側のパスと揃える）
	std::replace(path.begin(), path.end(), '\\', '/');
	return path;
}

//============================================================
// Update
//============================================================

void ItemManager::Update()
{
	const float deltaTime = 1.0f / 60.0f;

	PruneExpired();

	for (auto it = m_pending.begin(); it != m_pending.end();)
	{
		it->timer -= deltaTime;

		if (it->timer > 0.0f)
		{
			++it;
			continue;
		}

		//====================================================
		// 上限チェック（ワールド + 全キャラの所持品）
		//====================================================

		if (CountTotal(it->path) >= GetMaxCount(it->path))
		{
			it->timer = m_retryInterval;
			++it;
			continue;
		}

		//====================================================
		// 空いている同種Spawnerを探す
		//====================================================

		auto spawner = FindFreeSpawner(it->path);

		if (!spawner)
		{
			it->timer = m_retryInterval;
			++it;
			continue;
		}

		//====================================================
		// 出現
		// SpawnItem内で RegisterWorldItem されるので、
		// 同フレームの次の判定にも数が反映される（二重出現しない）
		//====================================================

		spawner->SpawnItem();

		it = m_pending.erase(it);
	}
}

void ItemManager::Reset()
{
	m_pending.clear();
	m_spawners.clear();
	m_worldItems.clear();
	m_maxCountCache.clear();
}

//============================================================
// 登録
//============================================================

void ItemManager::RegisterSpawner(
	const std::shared_ptr<ItemSpawner>& spawner)
{
	if (!spawner)
	{
		return;
	}

	m_spawners.push_back(spawner);

	const std::string& path = spawner->GetItemPath();

	if (CountTotal(path) < GetMaxCount(path))
	{
		spawner->SpawnItem();
	}
}

void ItemManager::RegisterWorldItem(
	const std::shared_ptr<Item>& item)
{
	if (!item)
	{
		return;
	}

	m_worldItems.push_back(item);
}

//============================================================
// 生成
//============================================================

std::shared_ptr<Item> ItemManager::SpawnDropped(
	const std::string& path,
	const Math::Vector3& pos,
	const Math::Vector3& dir)
{
	if (path.empty())
	{
		return nullptr;
	}

	auto item = std::make_shared<Item>();

	item->Init();

	if (!item->LoadItem(path))
	{
		return nullptr;
	}

	item->Drop(pos, dir);

	item->SetScale(
		item->GetItemData().GetWorldScale() *
		SceneManager::Instance().GetSceneScale());

	SceneManager::Instance().AddObject(item);

	RegisterWorldItem(item);

	return item;
}

//============================================================
// 通知
//============================================================

void ItemManager::OnPickedUp(const Item& item)
{
	// Spawnerが出したItemだけ補充対象。
	// 交換で落としたDroppedは補充しない
	if (item.GetMode() != ItemMode::Spawned)
	{
		return;
	}

	m_pending.push_back({ item.GetItemPath(), m_respawnTime });
}

//============================================================
// 数の取得
//============================================================

int ItemManager::CountInWorld(const std::string& path) const
{
	const std::string key = Normalize(path);

	int count = 0;

	for (const auto& wp : m_worldItems)
	{
		auto item = wp.lock();

		if (!item || item->IsExpired())
		{
			continue;
		}

		if (item->GetItemPath() == key)
		{
			++count;
		}
	}

	return count;
}

int ItemManager::CountHeld(const std::string& path) const
{
	const std::string key = Normalize(path);

	int count = 0;

	for (const auto& character :
		SceneManager::Instance().FindObjects<CharacterBase>())
	{
		if (!character)
		{
			continue;
		}

		count += character->GetInventory().CountItems(key);
	}

	return count;
}

int ItemManager::GetMaxCount(const std::string& path)
{
	const std::string key = Normalize(path);

	auto it = m_maxCountCache.find(key);

	if (it != m_maxCountCache.end())
	{
		return it->second;
	}

	int maxCount = m_defaultMaxCount;

	ItemData data;

	if (data.Load(key) && data.GetMaxCount() > 0)
	{
		maxCount = data.GetMaxCount();
	}

	m_maxCountCache[key] = maxCount;

	return maxCount;
}

//============================================================
// Private
//============================================================

std::shared_ptr<ItemSpawner> ItemManager::FindFreeSpawner(
	const std::string& path)
{
	const std::string key = Normalize(path);

	std::vector<std::shared_ptr<ItemSpawner>> candidates;

	for (const auto& wp : m_spawners)
	{
		auto spawner = wp.lock();

		if (!spawner)
		{
			continue;
		}

		if (spawner->GetItemPath() != key)
		{
			continue;
		}

		if (!spawner->IsFree())
		{
			continue;
		}

		candidates.push_back(spawner);
	}

	if (candidates.empty())
	{
		return nullptr;
	}

	// 同種の空き地点からランダムに選ぶ（拾われた場所以外にも出る）
	int index = KdRandom::GetInt(
		0, static_cast<int>(candidates.size()) - 1);

	return candidates[index];
}

void ItemManager::PruneExpired()
{
	m_worldItems.erase(
		std::remove_if(
			m_worldItems.begin(),
			m_worldItems.end(),
			[](const std::weak_ptr<Item>& wp)
			{
				auto item = wp.lock();
				return !item || item->IsExpired();
			}),
		m_worldItems.end());

	m_spawners.erase(
		std::remove_if(
			m_spawners.begin(),
			m_spawners.end(),
			[](const std::weak_ptr<ItemSpawner>& wp)
			{
				auto spawner = wp.lock();
				return !spawner || spawner->IsExpired();
			}),
		m_spawners.end());
}