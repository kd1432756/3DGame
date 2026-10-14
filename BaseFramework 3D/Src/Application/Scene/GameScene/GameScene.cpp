#include "GameScene.h"
#include"../SceneManager.h"
#include"../CameraManager/CameraManager.h"
#include "../../Editor/Editor.h"

#include"../../System/Node/AStarNode/AStarNodeManager.h"
#include"../../System/Navigation/NavMesh/NavMeshManager.h"

#include "../SceneSerializer/SceneSerializer.h"
#include "../../System/Node/AStarNode/AStarNodeSerializer.h"
#include "../../System/Navigation/NavMesh/NavMeshSerializer.h"
#include "../../System/Location/LocationPointSerializer.h"

#include "../../GameObject/Camera/FPSCamera/FPSCamera.h"
#include "../../GameObject/Character/Player/Player.h"

//#include "../../System/Node/RoadNode/RoadNodeManager.h"
#include "../../System/Score/ScoreManager.h"

#include "../../System/DayCycle/DayCycle.h"

#include "../../System/GrassField/GrassField.h"

#include "../../System/FootStep/FootStep.h"

#include "../../GameObject/UI/GameStateUI/GameStateUI.h"
#include "../../GameObject/UI/ResultUI/ResultUI.h"

#include "../../GameObject/SkyBox/SkyBox.h"

#include "../../GameObject/UI/Reticle/Reticle.h"
#include "../../GameObject/UI/InteractionProgress/InteractionProgress.h"
#include "../../GameObject/UI/InteractionPrompt/InteractionPrompt.h"
#include "../../GameObject/UI/InventoryUI/InventoryUI.h"
#include "../../GameObject/UI/TimeUI/TimeUI.h"
#include "../../GameObject/UI/ScoreUI/ScoreUI.h"

void GameScene::Update()
{
	if (!m_started)
	{
		Start();
		m_started = true;
	}

	// UIはゲーム停止中でも更新する
	m_gameStateUI->Update();
	m_resultUI->Update();

	// シーン毎のイベント処理
	Event();

	if (!m_gameUpdate)
	{
		return;
	}

	DayCycle::Instance().Update();

	if (DayCycle::Instance().IsFinished())
	{
		m_gameUpdate = false;
		m_state = GameState::Finished;

		// 終了処理
		auto camera = CameraManager::Instance().GetCamera();
		if (camera)
		{
			camera->SetMouseLock(false);
		}

		m_gameStateUI->ShowFinish();

		return;
	}

	// KdGameObjectを継承した全てのオブジェクトの更新 (ポリモーフィズム)
	for (auto& obj : m_objList)
	{
		if (obj == m_gameStateUI)
		{
			continue;
		}

		if (obj == m_resultUI)
		{
			continue;
		}

		obj->Update();
	}
}

void GameScene::Event()
{
	auto player = SceneManager::Instance().FindObject<Player>();
	FootstepSystem::Instance().SetListenerPos(player->GetPos());

	{
		static float time = 0.0f;
		time += 1.0f / 60.0f;	// フレーム固定なら。dtが取れるならそちらを使う

		// 1. 風の強さ: 周期の違う波を重ねて不規則にする
		float strength =
			0.6f +
			0.25f * sinf(time * 1.3f) +
			0.15f * sinf(time * 3.1f + 1.7f) +
			0.10f * sinf(time * 7.3f + 0.4f);	// 細かい揺らぎ(ブレ)

		// 2. 突風: たまに強くなる
		float gust = sinf(time * 0.35f);
		gust = std::max(0.0f, gust);
		gust = gust * gust * gust;				// 立ち上がりを鋭く
		strength += gust * 0.8f;

		// 3. 風向き: ゆっくり回転 + 小さなブレ
		float angle = 0.6f + 0.5f * sinf(time * 0.2f) + 0.1f * sinf(time * 1.1f);

		const float scale = SceneManager::Instance().GetSceneScale();
		strength *= scale * 0.2f;

		KdShaderManager::Instance().SetWindValue({
			cosf(angle) * strength,
			0,
			sinf(angle) * strength });
	}

	if (GetAsyncKeyState('T') & 0x8000)
	{
		SceneManager::Instance().SetNextScene
		(
			SceneManager::SceneType::Title
		);
	}

	// Yキー：30分戻す
	if (GetAsyncKeyState('Y') & 0x8000)
	{
		DayCycle::Instance().AddTime(-0.5f);
	}
	// Uキー：30分進める
	if (GetAsyncKeyState('U') & 0x8000)
	{
		DayCycle::Instance().AddTime(0.5f);
	}
}

void GameScene::Start()
{
	SceneManager::Instance().SetSceneScale(0.2f);

	auto stage = SceneManager::Instance().GetStageData();
	auto scale = SceneManager::Instance().GetSceneScale();

	AStarNodeSerializer::LoadWithScale(
		stage.astarPath,
		AStarNodeManager::Instance(),
		scale);

	NavMeshSerializer::LoadWithScale(
		stage.navMeshPath,
		NavMeshManager::Instance(),
		scale);

	SceneSerializer::LoadWithScale(
		stage.scenePath,
		scale);

	LocationPointSerializer::LoadWithScale(
		stage.locationPointPath,
		LocationPointManager::Instance(),
		scale);

//	KdShaderManager::Instance().WorkAmbientController().SetFogEnable(true, false);
//	KdShaderManager::Instance().WriteCBDistanceFog({ 1,1,1 }, 0.0005f);

	FootstepSystem::Instance().Init();

	// 風有効化
	KdShaderManager::Instance().SetWindEnable(true);

	auto player = SceneManager::Instance().FindObject<Player>();

	auto time = std::make_shared<TimeUI>();
	time->Init();
	time->SetName("TimeUI");
	m_objList.push_back(time);

	auto field = std::make_shared<GrassField>();
	SceneManager::Instance().AddObject(field);
	field->Generate(5.0f, 0.3f * scale);
	field->SetPlayer(player);
	field->SetDrawDistance(25.0f * scale);

	float height = 0.8f * SceneManager::Instance().GetSceneScale();
	m_spGameCamera->SetEyeHeight(height);

	auto skyBox = std::make_shared<SkyBox>();
	skyBox->Init();
	m_objList.push_back(skyBox);

	{
		auto score = std::make_shared<ScoreUI>();
		score->Init();
		score->SetName("ScoreUI");
		score->SetPlayer(player);
		m_objList.push_back(score);

		auto reticle = std::make_shared<Reticle>();
		reticle->Init();
		reticle->SetName("Reticle");
		reticle->SetPlayer(player);
		m_objList.push_back(reticle);

		auto progress = std::make_shared<InteractionProgress>();
		progress->Init();
		progress->SetName("InteractionProgress");
		progress->SetPlayer(player);
		m_objList.push_back(progress);

		auto prompt = std::make_shared<InteractionPrompt>();
		prompt->Init();
		prompt->SetName("InteractionPrompt");
		prompt->SetPlayer(player);
		m_objList.push_back(prompt);

		auto inventory = std::make_shared<InventoryUI>();
		inventory->Init();
		inventory->SetName("InventoryUI");
		inventory->SetPlayer(player);
		m_objList.push_back(inventory);
	}

	{
		m_gameStateUI = std::make_shared<GameStateUI>();
		m_gameStateUI->Init();
		m_gameStateUI->SetName("GameStateUI");
		m_gameStateUI->SetGameScene(
			std::dynamic_pointer_cast<GameScene>(
				shared_from_this()));
		m_objList.push_back(m_gameStateUI);

		m_resultUI = std::make_shared<ResultUI>();
		m_resultUI->Init();
		m_resultUI->SetName("ResultUI");
		m_resultUI->SetGameScene(
			std::dynamic_pointer_cast<GameScene>(
				shared_from_this()));
		m_objList.push_back(m_resultUI);
	}

	{
		for (auto& obj : m_objList)
		{
			if (obj == m_gameStateUI)
			{
				continue;
			}

			if (obj == m_resultUI)
			{
				continue;
			}

			obj->Update();
		}
	}
}

void GameScene::Init()
{
	m_spGameCamera = std::make_shared<FPSCamera>();

	m_spGameCamera->Init();
	m_spGameCamera->SetMouseLock(true);

	Editor::Instance().SetGameCamera(m_spGameCamera);

	CameraManager::Instance().SetCamera(m_spGameCamera);

	ScoreManager::Instance().Init();

	DayCycle::Instance().Init();

	//RoadNodeManager::Instance().Init();
}
