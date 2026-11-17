#include "ItemSpawner.h"

#include "../../../../Scene/SceneManager.h"
#include "Item.h"
#include "ItemManager.h"

void ItemSpawner::Update()
{
	if (m_registered)
	{
		return;
	}

	if (m_itemPath.empty())
	{
		return;
	}

	m_registered = true;

	ItemManager::Instance().RegisterSpawner(
		std::dynamic_pointer_cast<ItemSpawner>(
			shared_from_this()));
}

bool ItemSpawner::IsFree() const
{
	auto item = m_current.lock();

	return !item || item->IsExpired();
}

void ItemSpawner::SpawnItem()
{
	auto item = std::make_shared<Item>();

	item->Init();
	item->LoadItem(m_itemPath);

	item->SetMode(ItemMode::Spawned);
	item->SetPos(m_spawnPos);
	item->SetScale(m_spawnScale);

	SceneManager::Instance().AddObject(item);

	m_current = item;

	ItemManager::Instance().RegisterWorldItem(item);
}