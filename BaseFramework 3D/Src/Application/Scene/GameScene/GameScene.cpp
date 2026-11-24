#include "GameScene.h"
#include"../SceneManager.h"
#include"../CameraManager/CameraManager.h"
#include "../../Editor/Editor.h"

#include"../../System/Node/AStarNode/AStarNodeManager.h"
#include"../../System/Navigation/NavMesh/NavMeshManager.h"
#include"../../System/Location/LocationPointManager.h"

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
#include "../../GameObject/UI/ConfettiUI/ConfettiUI.h"
#include "../../GameObject/UI/ResultUI/ResultUI.h"

#include "../../GameObject/SkyBox/SkyBox.h"

#include "../../GameObject/UI/ReticleUI/ReticleUI.h"
#include "../../GameObject/UI/InteractionProgress/InteractionProgress.h"
#include "../../GameObject/UI/InteractionPrompt/InteractionPrompt.h"
#include "../../GameObject/UI/InventoryUI/InventoryUI.h"
#include "../../GameObject/UI/TimeUI/TimeUI.h"
#include "../../GameObject/UI/ScoreUI/ScoreUI.h"
#include "../../GameObject/UI/BonusUI/BonusUI.h"
#include "../../GameObject/UI/HomeDirectionUI/HomeDirectionUI.h"
#include "../../GameObject/UI/HinderUI/HinderUI.h"
#include "../../GameObject/UI/ReportUI/ReportUI.h"
#include "../../GameObject/UI/DamageIndicatorUI/DamageIndicatorUI.h"
#include "../../GameObject/UI/DamageDirectionUI/DamageDirectionUI.h"

#include "../../GameObject/Effect/DeliveryPointEffect/DeliveryPointEffect.h"

#include "../../GameObject/UI/NotificationUI/NotificationManager.h"

#include "../../System/Damage/DamageDirector.h"

void GameScene::Update()
{
	if (!m_started)
	{
		Start();
		m_started = true;
	}

	// UIはゲーム停止中でも更新する
	for (auto& obj : m_objList)
	{
		if (obj->GetUpdateGroup() == KdGameObject::UpdateGroup::World) continue;
		obj->Update();
	}

	// シーン毎のイベント処理
	Event();

	if (!m_gameUpdate) return;

	DayCycle::Instance().Update();

	NotificationManager::Instance().Update();

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

		KdAudioManager::Instance().StopAllSound();

		m_gameStateUI->ShowFinish();

		if (auto se = KdAudioManager::Instance().Play("Asset/Sound/SE/Finish.wav"))
		{
			se->SetVolume(0.2f);
		}

		return;
	}

	// KdGameObjectを継承した全てのオブジェクトの更新 (ポリモーフィズム)
	for (auto& obj : m_objList)
	{
		if (obj->GetUpdateGroup() == KdGameObject::UpdateGroup::Always)		continue;
		obj->Update();
	}
}

void GameScene::PostUpdate()
{
	if (!m_gameUpdate)
	{
		return;
	}

	for (auto& obj : m_objList)
	{
		obj->PostUpdate();
	}
}

void GameScene::Event()
{
	if (auto player = SceneManager::Instance().FindObject<Player>())
	{
		FootstepSystem::Instance().SetListenerPos(player->GetPos());
	}

	{
		static float time = 0.0f;
		time += 1.0f / 60.0f;	

		// 1. 風の強さ
		float strength =
			0.6f +
			0.25f * sinf(time * 1.3f) +
			0.15f * sinf(time * 3.1f + 1.7f) +
			0.10f * sinf(time * 7.3f + 0.4f);

		// 2. 突風
		float gust = sinf(time * 0.35f);
		gust = std::max(0.0f, gust);
		gust = gust * gust * gust;
		strength += gust * 0.8f;

		// 3. 風向き
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

	char buf[256];
	sprintf_s(buf, "object = %d\n",
		SceneManager::Instance().GetObjList().size());
	OutputDebugStringA(buf);

	FootstepSystem::Instance().Init();

	// 風有効化
	KdShaderManager::Instance().SetWindEnable(true);

	NotificationManager::Instance().Clear();

	auto player = SceneManager::Instance().FindObject<Player>();

	player->SetCamera(m_spGameCamera);

	auto time = std::make_shared<TimeUI>();
	time->Init();
	time->SetName("TimeUI");
	m_objList.push_back(time);

	auto field = std::make_shared<GrassField>();
	SceneManager::Instance().AddObject(field);
	field->Generate(8.0f, 0.3f * scale);
	field->SetPlayer(player);
	field->SetDrawDistance(25.0f * scale);

	float height = 0.8f * SceneManager::Instance().GetSceneScale();
	m_spGameCamera->SetEyeHeight(height);

	auto skyBox = std::make_shared<SkyBox>();
	skyBox->Init();
	m_objList.push_back(skyBox);

	{
		auto damageUI = std::make_shared<DamageDirectionUI>();
		damageUI->Init();
		damageUI->SetName("DamageUI");
		damageUI->SetPlayer(player);
		player->SetDamageUI(damageUI);
		m_objList.push_back(damageUI);

		auto score = std::make_shared<ScoreUI>();
		score->Init();
		score->SetName("ScoreUI");
		score->SetPlayer(player);
		m_objList.push_back(score);

		auto bonus = std::make_shared<BonusUI>();
		bonus->Init();
		bonus->SetName("BonusUI");
		bonus->SetPlayer(player);
		m_objList.push_back(bonus);

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

		auto hinderUI = std::make_shared<HinderUI>();
		hinderUI->Init();
		hinderUI->SetPlayer(player);
		m_objList.push_back(hinderUI);

		auto reportUI = std::make_shared<ReportUI>();
		reportUI->Init();
		reportUI->SetPlayer(player);
		m_objList.push_back(reportUI);

		auto indicator = std::make_shared<DamageIndicatorUI>();
		indicator->Init();
		indicator->SetPlayer(player);
		SceneManager::Instance().AddObject(indicator);
	}

	{
		m_gameStateUI = std::make_shared<GameStateUI>();
		m_gameStateUI->Init();
		m_gameStateUI->SetName("GameStateUI");
		m_gameStateUI->SetGameScene(
			std::dynamic_pointer_cast<GameScene>(
				shared_from_this()));
		m_objList.push_back(m_gameStateUI);

		auto confettiUI = std::make_shared<ConfettiUI>();
		confettiUI->Init();

		auto resultUI = std::make_shared<ResultUI>();
		resultUI->Init();
		resultUI->SetName("ResultUI");
		resultUI->SetGameScene(
			std::dynamic_pointer_cast<GameScene>(
				shared_from_this()));
		resultUI->SetConfetti(confettiUI);

		m_objList.push_back(resultUI);
		m_objList.push_back(confettiUI);
	}

	{
		auto deliveryEffect = std::make_shared<DeliveryPointEffect>();
		deliveryEffect->Init();
		deliveryEffect->SetName("DeliveryPointEffect");
		deliveryEffect->SetPlayer(player);
		SceneManager::Instance().AddObject(deliveryEffect);
	}

	DamageDirector::SpawnInitialDamage(3);
}

void GameScene::Init()
{
	m_spGameCamera = std::make_shared<FPSCamera>();

	m_spGameCamera->Init();
	m_spGameCamera->SetMouseLock(true);

	CameraManager::Instance().SetCamera(m_spGameCamera);

	DayCycle::Instance().Init();

	//RoadNodeManager::Instance().Init();
}

void GameScene::SetGameUpdate(bool flg)
{
	m_gameUpdate = flg;
}

void GameScene::SetGameState(GameState state)
{
	m_state = state;
}
