#include "LoadingScene.h"

#include "../SceneManager.h"
#include "../SceneSerializer/SceneSerializer.h"
#include "../../System/Node/AStarNode/AStarNodeSerializer.h"
#include "../../System/Navigation/NavMesh/NavMeshSerializer.h"
#include "../../System/Location/LocationPointSerializer.h"
#include "../../System/Location/LocationPointManager.h"

#include "../../GameObject/UI/LoadingUI/LoadingUI.h"

void LoadingScene::Event()
{
	// 最初の1フレームをLoading画面表示に使う
	if (!m_startedLoading)
	{
		m_startedLoading = true;

		m_loadingUI->SetProgress(0.0f);
		m_loadingUI->SetCurrentPath("");

		return;
	}

	auto stage = SceneManager::Instance().GetStageData();

	//------------------------------------------------------------
	// AStarNode
	//------------------------------------------------------------

	if (m_loadStep == LoadStep::AStarNode)
	{
		m_loadingUI->SetCurrentPath("AStarNode");

		AStarNodeSerializer::LoadWithScale(
			stage.astarPath,
			AStarNodeManager::Instance(),
			SceneManager::Instance().GetSceneScale());

		m_loadingUI->SetProgress(0.1f);

		m_loadStep = LoadStep::NavMeshStart;

		return;
	}

	//------------------------------------------------------------
	// NavMesh 前処理
	//------------------------------------------------------------
		
	if (m_loadStep == LoadStep::NavMeshStart)
	{
		m_loadingUI->SetCurrentPath("NavMesh");
		m_loadingUI->SetProgress(0.1f);

		m_loadStep = LoadStep::NavMesh;

		return;
	}

	//------------------------------------------------------------
	// NavMesh
	//------------------------------------------------------------

	if (m_loadStep == LoadStep::NavMesh)
	{
		m_loadingUI->SetCurrentPath(
			"NavMesh"
		);

		NavMeshSerializer::LoadWithScale(
			stage.navMeshPath,
			NavMeshManager::Instance(),
			SceneManager::Instance().GetSceneScale()
		);

		m_loadingUI->SetProgress(0.2f);

		m_loadStep = LoadStep::LocationPoint;

		return;
	}

	//------------------------------------------------------------
	// LocationPoint
	//------------------------------------------------------------

	if (m_loadStep == LoadStep::LocationPoint)
	{
		m_loadingUI->SetCurrentPath(
			"LocationPoint"
		);

		LocationPointSerializer::LoadWithScale(
			stage.locationPointPath,
			LocationPointManager::Instance(),
			SceneManager::Instance().GetSceneScale()
		);

		m_loadingUI->SetProgress(0.3f);

		m_loadStep = LoadStep::Model;

		return;
	}

	//------------------------------------------------------------
	// Model
	//------------------------------------------------------------

	if (m_loadStep == LoadStep::Model)
	{
		while (m_modelIndex < m_modelPaths.size() &&
			KdAssets::Instance().m_modeldatas.IsLoaded(m_modelPaths[m_modelIndex]))
		{
			++m_modelIndex;
		}

		if (m_modelIndex < m_modelPaths.size())
		{
			const std::string& modelPath =
				m_modelPaths[m_modelIndex];

			m_loadingUI->SetCurrentPath(modelPath);

			OutputDebugStringA(
				("[Loading] " + modelPath + "\n").c_str()
			);

			KdAssets::Instance()
				.m_modeldatas
				.GetData(modelPath);

			++m_modelIndex;

			// 30% ～ 100% をモデルロードに割り当てる
			float modelProgress = 0.0f;

			if (!m_modelPaths.empty())
			{
				modelProgress =
					static_cast<float>(m_modelIndex)
					/
					static_cast<float>(m_modelPaths.size());
			}

			float progress =
				0.3f + modelProgress * 0.7f;

			m_loadingUI->SetProgress(progress);

			return;
		}

		//--------------------------------------------------------
		// 全ロード完了
		//--------------------------------------------------------

		m_loadingUI->SetProgress(1.0f);
		m_loadingUI->SetCurrentPath("Complete");
		m_loadingUI->SetComplete(true);

		m_loadStep = LoadStep::Finished;

		return;
	}

	//------------------------------------------------------------
	// Finished
	//------------------------------------------------------------

	if (m_loadStep == LoadStep::Finished)
	{
		if (!m_loadFinished)
		{
			m_loadFinished = true;

			SceneManager::Instance().SetNextScene(
				SceneManager::SceneType::Game
			);
		}
	}
}

void LoadingScene::Init()
{
	SceneManager::Instance().SetSceneScale(0.2f);

	m_loadStep = LoadStep::AStarNode;

	m_modelIndex = 0;

	m_startedLoading = false;
	m_loadFinished = false;
	m_modelPaths.clear();

	m_loadingUI = std::make_shared<LoadingUI>();
	m_loadingUI->Init();

	m_objList.push_back(m_loadingUI);

	// GameSceneで使用するモデル一覧を取得
	if (!SceneSerializer::GetModelPaths(
		SceneManager::Instance().GetStageData().scenePath,
		m_modelPaths))
	{
		// JSONの読み込みに失敗
		m_loadFinished = true;
	}
}

float LoadingScene::GetProgress() const
{
	if (m_modelPaths.empty())
	{
		return 1.0f;
	}

	return static_cast<float>(m_modelIndex)
		/ static_cast<float>(m_modelPaths.size());
}