#include "ItemManager.h"

#include "../../../../Scene/SceneManager.h"
#include "../../../Character/CharacterBase.h"
#include "Item.h"
#include "ItemSpawner.h"

std::string ItemManager::Normalize(std::string path)
{
	// パスの正規化
	std::replace(path.begin(), path.end(), '\\', '/');
	return path;
}

void ItemManager::Update()
{
	PruneExpired();

	for (auto it = m_pending.begin(); it != m_pending.end();)
	{
		it->timer -= 1.0f / 60.0f;

		if (it->timer > 0.0f)
		{
			++it;
			continue;
		}

		// 上限チェック（ワールド + 所持品）
		if (CountTotal(it->path) >= GetMaxCount(it->path))
		{
			it->timer = m_retryInterval;
			++it;
			continue;
		}

		// 空いているSpawnerを探す		
		auto spawner = FindFreeSpawner(it->path);

		if (!spawner)
		{
			it->timer = m_retryInterval;
			++it;
			continue;
		}

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

void ItemManager::RegisterSpawner(const std::shared_ptr<ItemSpawner>& spawner)
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

void ItemManager::RegisterWorldItem(const std::shared_ptr<Item>& item)
{
	if (!item)
	{
		return;
	}

	m_worldItems.push_back(item);
}

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

	item->SetScale(item->GetItemData().GetWorldScale() * SceneManager::Instance().GetSceneScale());

	SceneManager::Instance().AddObject(item);

	RegisterWorldItem(item);

	return item;
}

void ItemManager::OnPickedUp(const Item& item)
{
	if (item.GetMode() != ItemMode::Spawned)
	{
		return;
	}

	m_pending.push_back({ item.GetItemPath(), m_respawnTime });
}

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

	for (const auto& character : SceneManager::Instance().FindObjects<CharacterBase>())
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

std::shared_ptr<ItemSpawner> ItemManager::FindFreeSpawner(const std::string& path)
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

	int index = KdRandom::GetInt(0, static_cast<int>(candidates.size()) - 1);

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