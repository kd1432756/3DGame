#pragma once

class DamageDirector
{
public:

	static void SpawnInitialDamage(int count);

	static int CountDamaged();	// 今壊れている物の数
	static int GetMaxDamaged();	// 同時に壊れていて良い上限
	static bool CanDamageMore();
};