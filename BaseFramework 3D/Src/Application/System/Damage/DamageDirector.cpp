#include "DamageDirector.h"

#include "../../Scene/SceneManager.h"
#include "../../GameObject/Object/StaticMeshObject/InteractiveObject/InteractiveObject.h"

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
	// 朝5 → 夜10
	const float progress = DayCycle::Instance().GetTimeProgress();
	return 5 + static_cast<int>(progress * 5.0f);
}

bool DamageDirector::CanDamageMore()
{
	return CountDamaged() < GetMaxDamaged();
}