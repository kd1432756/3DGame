#include "TitleScene.h"
#include "../SceneManager.h"

#include "../../Scene/CameraManager/CameraManager.h"

#include "../../GameObject/Camera/EventCamera/EventCamera.h"

#include "../../GameObject/UI/TitleUI/TitleUI.h"

#include "../../GameObject/SkyBox/SkyBox.h"

#include "../../System/DayCycle/DayCycle.h"

#include "../../GameObject/Object/TitleProp/TitleProp.h"
#include "../../GameObject/Character/TitleChaser/TitleChaser.h"

void TitleScene::Event()
{
	if (GetAsyncKeyState(VK_F2) & 0x8000)
	{
		SceneManager::Instance().SetNextScene
		(
			SceneManager::SceneType::Edit
		);
	}

	if (GetAsyncKeyState('P') & 0x8000)
	{
		KdAudioManager::Instance().SetMasterVolume(3.0f);
	}
}

void TitleScene::Init()
{
	const KdShaderManager::cbLight& cbLight = KdShaderManager::Instance().GetLightCB();

	Math::Vector3 DLColor{ 2.5f,2.5f,2.5f };
	Math::Vector3 DLDir = { 0.577f,-0.577f,0.577f };
	Math::Vector4 ALColor = { 0.3f,0.3f,0.3f,1.0f };

	KdShaderManager::Instance().WorkAmbientController().SetDirLight(DLDir, DLColor);
	KdShaderManager::Instance().WorkAmbientController().SetAmbientLight(ALColor);

	DayCycle::Instance().Init();

	auto camera = std::make_shared<EventCamera>();
	camera->Init();
	camera->SetPos({ -3.2, 0.35f,-5.4f });
	camera->SetRot({0,-10,0});
	CameraManager::Instance().SetCamera(camera);

	auto skyBox = std::make_shared<SkyBox>();
	skyBox->Init();
	m_objList.push_back(skyBox);

	auto titleUI = std::make_shared<TitleUI>();
	titleUI->Init();
	m_objList.push_back(titleUI);

	auto parking = std::make_shared<TitleProp>();
	parking->Init();
	parking->SetModel("Asset/Models/Ground/LotParking.gltf");
	parking->SetPos({ 0,-1,0 });
	parking->SetScale({ 1,1,1 });
	m_objList.push_back(parking);

	const int   GRID = 9;
	const float TILE = 4.0f;

	for (int i = 0; i < GRID * GRID; ++i)
	{
		int col = i % GRID;
		int row = i / GRID;

		if (col == GRID / 2 && row == GRID / 2) continue;

		// 中心が(0,0)になるようにずらす(9x9なら -4〜+4)
		float x = (col - GRID / 2) * TILE;
		float z = (row - GRID / 2) * TILE;

		auto ground = std::make_shared<TitleProp>();
		ground->Init();
		ground->SetModel("Asset/Models/Ground/Lot.gltf");
		ground->SetPos({ x, -1, z });
		ground->SetScale({ 1, 1, 1 });
		m_objList.push_back(ground);
	}

	{
		auto tree = std::make_shared<TitleProp>();
		tree->Init();
		tree->SetModel("Asset/Models/Props/Tree/Tree.gltf");
		tree->SetPos({ 0,0,0 });
		tree->SetScale(10.0f);
		m_objList.push_back(tree);
	}

	{
		auto paintCan = std::make_shared<TitleProp>();
		paintCan->Init();
		paintCan->SetModel("Asset/Models/Props/PaintCan/SpilledPaintCan.gltf");
		paintCan->SetPos({ -4,0,-5 });
		paintCan->SetScale(3);
		paintCan->SetRot({ 0,-130,0 });
		m_objList.push_back(paintCan);
	}

	{
		auto house = std::make_shared<TitleProp>();
		house->Init();
		house->SetModel("Asset/Models/Props/House/House_Source.gltf");
		house->SetPos({ -10,0,16 });
		house->SetScale(10);
		house->SetRot({ 0,180,0 });
		m_objList.push_back(house);
	}

	{
		auto fence = std::make_shared<TitleProp>();
		fence->Init();
		fence->SetModel("Asset/Models/Props/Fence/WoodFence.gltf");
		fence->SetPos({ -4,0,6.5f });
		fence->SetScale({0.2f,0.2f,0.4f});
		fence->SetRot({ 0,90,0 });
		m_objList.push_back(fence);
	}
	{
		auto fence = std::make_shared<TitleProp>();
		fence->Init();
		fence->SetModel("Asset/Models/Props/Fence/WoodFence.gltf");
		fence->SetPos({ -16,0,6.5f });
		fence->SetScale({ 0.2f,0.2f,0.4f });
		fence->SetRot({ 0,90,0 });
		m_objList.push_back(fence);
	}
	{
		auto fence = std::make_shared<TitleProp>();
		fence->Init();
		fence->SetModel("Asset/Models/Props/Fence/WoodFence.gltf");
		fence->SetPos({ 4,0,6.5f });
		fence->SetScale({ 0.2f,0.2f,0.4f });
		fence->SetRot({ 0,90,0 });
		m_objList.push_back(fence);
	}

	// 逃げるプレイヤー
	auto player = std::make_shared<TitleChaser>();
	player->Init();
	player->SetModel("Asset/Models/Character/Player/Player.gltf");
	player->SetCenter({ 0, 0.7f, 0 });
	player->SetRadius(3.5f);
	player->SetAngleSpeed(0.02f);
	m_objList.push_back(player);

	// 追う警察（少し後ろの角度から同じ速さで）
	auto police = std::make_shared<TitleChaser>();
	police->Init();
	police->SetModel("Asset/Models/Character/Police/Police2.gltf");
	police->SetCenter({ 0, 0.7f, 0 });
	police->SetRadius(3.5f);
	police->SetAngleSpeed(0.02f);
	police->SetStartAngle(-0.8f);   // プレイヤーより後ろ
	m_objList.push_back(police);
}
