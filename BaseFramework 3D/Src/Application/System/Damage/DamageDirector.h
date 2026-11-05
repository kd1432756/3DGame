#pragma once

class DamageDirector
{
public:

	static void SpawnInitialDamage(int count);

	static int CountDamaged();		// 今壊れている物の数
	static int GetMaxDamaged();		// 同時に壊れていてよい上限(時間で増える)
	static bool CanDamageMore();
};