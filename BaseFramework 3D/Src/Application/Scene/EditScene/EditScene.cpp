#include "EditScene.h"
#include "../SceneManager.h"

void EditScene::Event()
{
	if (GetAsyncKeyState('T') & 0x8000)
	{
		SceneManager::Instance().SetNextScene
		(
			SceneManager::SceneType::Title
		);
	}
}

void EditScene::Init()
{
}
