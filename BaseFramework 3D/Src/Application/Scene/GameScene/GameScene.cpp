#include "GameScene.h"
#include"../SceneManager.h"
#include"../CameraManager/CameraManager.h"
#include "../../Editor/Editor.h"

#include"../../System/Node/AStarNode/AStarNodeManager.h"
#include"../../System/Navigation/NavMesh/NavMeshManager.h"

#include "../SceneSerializer/SceneSerializer.h"
#include "../../System/Node/AStarNode/AStarNodeSerializer.h"
#include "../../System/Navigation/NavMesh/NavMeshSerializer.h"

#include "../../GameObject/Camera/FPSCamera/FPSCamera.h"
#include "../../GameObject/Character/Player/Player.h"

//#include "../../System/Node/RoadNode/RoadNodeManager.h"
#include "../../System/Score/ScoreManager.h"

#include "../../System/DayCycle/DayCycle.h"

#include "../../GameObject/UI/GameStateUI/GameStateUI.h"
#include "../../GameObject/UI/ResultUI/ResultUI.h"

#include "../../GameObject/SkyBox/SkyBox.h"

#include "../../GameObject/UI/Reticle/Reticle.h"
#include "../../GameObject/UI/InteractionProgress/InteractionProgress.h"
#include "../../GameObject/UI/InteractionPrompt/InteractionPrompt.h"
#include "../../GameObject/UI/InventoryUI/InventoryUI.h"
#include "../../GameObject/UI/TimeUI/TimeUI.h"
#include "../../GameObject/UI/ScoreUI/ScoreUI.h"
#include "../../GameObject/UI/BushEffect/BushEffect.h"

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

	m_dayCycle->Update();

	if (m_dayCycle->IsFinished())
	{
		m_gameUpdate = false;
		m_state = GameState::Finished;

		m_spGameCamera->SetMouseLock(false);

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
		m_dayCycle->AddTime(-0.5f);
	}
	// Uキー：30分進める
	if (GetAsyncKeyState('U') & 0x8000)
	{
		m_dayCycle->AddTime(0.5f);
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

	m_dayCycle = std::make_shared<DayCycle>();
	m_dayCycle->Init();
	
	auto time = std::make_shared<TimeUI>();
	time->Init();
	time->SetName("TimeUI");
	time->SetDayCycle(m_dayCycle);
	m_objList.push_back(time);

	auto player = SceneManager::Instance().FindObject<Player>();

	float height = 0.8f * SceneManager::Instance().GetSceneScale();
	m_spGameCamera->SetEyeHeight(height);

	auto skyBox = std::make_shared<SkyBox>();
	skyBox->Init();
	m_objList.push_back(skyBox);

	{
		auto bushEffect = std::make_shared<BushEffect>();
		bushEffect->Init();
		bushEffect->SetName("BushEffect");
		m_objList.push_back(bushEffect);

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

	//RoadNodeManager::Instance().Init();
}
