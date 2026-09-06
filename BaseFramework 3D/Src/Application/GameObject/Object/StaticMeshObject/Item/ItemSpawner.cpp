#include "ItemSpawner.h"

#include "../../../../Scene/SceneManager.h"
#include "../../../Character/Player/Player.h"
#include "Item.h"

void ItemSpawner::Update()
{
	if (!m_respawnTimerFlg)
	{
		return;
	}

	m_respawnTimer -= 1.0f / 60.0f;

	if (m_respawnTimer > 0.0f)
	{
		return;
	}

	auto player = SceneManager::Instance().FindObject<Player>();

	// 最大数チェック
	int count = 0;

	if (!player)return;

	count = SceneManager::Instance().CountItems(m_itemPath)
		+ player->GetInventory().CountItems(m_itemPath);

	if (count >= 3)
	{
		return;
	}

	SpawnItem();

	m_respawnTimerFlg = false;
}

void ItemSpawner::StartRespawn()
{
	m_respawnTimer = m_respawnTime;
	m_respawnTimerFlg = true;
}

void ItemSpawner::SpawnItem()
{
	auto item = std::make_shared<Item>();

	item->Init();
	item->LoadItem(m_itemPath);

	item->SetMode(ItemMode::Spawned);
	item->SetPos(m_spawnPos);
	item->SetScale(m_spawnScale);

	item->SetSpawner(
		std::dynamic_pointer_cast<ItemSpawner>(
			shared_from_this()));

	SceneManager::Instance().AddObject(item);
}