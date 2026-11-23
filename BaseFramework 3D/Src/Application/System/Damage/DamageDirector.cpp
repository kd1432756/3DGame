#include "DamageDirector.h"

#include "../../Scene/SceneManager.h"
#include "../../GameObject/Object/StaticMeshObject/InteractiveObject/InteractiveObject.h"
#include "../../GameObject/Character/Player/Player.h"
#include "../DayCycle/DayCycle.h"

void DamageDirector::SpawnInitialDamage(int count)
{
	auto objects = SceneManager::Instance().FindObjects<InteractiveObject>();

	KdRandom::Shuffle(objects);

	int done = 0;

	for (auto& obj : objects)
	{
		if (done >= count) break;

		if (obj->SetInitialDamaged())
		{
			++done;

			if (auto player = SceneManager::Instance().FindObject<Player>())
			{
				player->AddDamageTarget(obj);
			}
		}
	}
}

int DamageDirector::CountDamaged()
{
	int count = 0;

	for (auto& obj : SceneManager::Instance().FindObjects<InteractiveObject>())
	{
		if (obj->IsDamaged())
		{
			++count;
		}
	}

	return count;
}

int DamageDirector::GetMaxDamaged()
{
	// 朝8 → 夜15
	const float progress = DayCycle::Instance().GetTimeProgress();
	return 8 + static_cast<int>(progress * 7.0f);
}

bool DamageDirector::CanDamageMore()
{
	return CountDamaged() < GetMaxDamaged();
}