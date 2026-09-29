#include "ItemSpawner.h"

#include "../../../../Scene/SceneManager.h"
#include "Item.h"
#include "ItemManager.h"

void ItemSpawner::Update()
{
	// SetItemPath などの設定が終わった後に登録したいので、
	// Initではなく初回Updateで登録する
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

	// 未生成、拾われて消えた、Destroy済み → 空き
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

	// 数の管理はManager。ここで即登録するので二重出現しない
	ItemManager::Instance().RegisterWorldItem(item);
}