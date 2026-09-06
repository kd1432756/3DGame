#include "SceneManager.h"

#include "BaseScene/BaseScene.h"
#include "TitleScene/TitleScene.h"
#include "GameScene/GameScene.h"
#include "LoadingScene/LoadingScene.h"
#include "EditScene/EditScene.h"

#include "../GameObject/Object/StaticMeshObject/StaticMeshObject.h"
#include "../GameObject/Object/StaticMeshObject/Item/Item.h"
#include "../GameObject/Object/StaticMeshObject/Ground/Ground.h"
#include "../GameObject/Character/Player/Player.h"
#include "../GameObject/Character/Enemy/Neighbor/Neighbor.h"
#include "../GameObject/Character/Enemy/Police/Police.h"
#include "../GameObject/Object/StaticMeshObject/Tree/Tree.h"
#include "../GameObject/Object/StaticMeshObject/InteractiveObject/Flower/Flower.h"
#include "../GameObject/Object/StaticMeshObject/InteractiveObject/Hose/Hose.h"
#include "../GameObject/Object/StaticMeshObject/InteractiveObject/PaintCan/PaintCan.h"
#include "../GameObject/Object/StaticMeshObject/InteractiveObject/WoodenFence/WoodenFence.h"

void SceneManager::PreUpdate()
{
	// シーン切替
	if (m_currentSceneType != m_nextSceneType)
	{
		ChangeScene(m_nextSceneType);
	}

	m_currentScene->PreUpdate();
}

void SceneManager::Update()
{
	m_currentScene->Update();
}

void SceneManager::PostUpdate()
{
	m_currentScene->PostUpdate();
}

void SceneManager::PreDraw()
{
	m_currentScene->PreDraw();
}

void SceneManager::Draw()
{
	m_currentScene->Draw();
}

void SceneManager::DrawSprite()
{
	m_currentScene->DrawSprite();
}

void SceneManager::DrawDebug()
{
	m_currentScene->DrawDebug();
}

const std::list<std::shared_ptr<KdGameObject>>& SceneManager::GetObjList()
{
	return m_currentScene->GetObjList();
}

void SceneManager::AddObject(const std::shared_ptr<KdGameObject>& _obj)
{
	m_currentScene->AddObject(_obj);
}

void SceneManager::ClearObjects()
{
	m_currentScene->ClearObjects();
}

std::shared_ptr<KdGameObject> SceneManager::CreateObject(const std::string& type)
{
	return CreateObject(type, type);
}

std::shared_ptr<KdGameObject> SceneManager::CreateObject(const std::string& type, const std::string& baseName)
{
	auto obj = m_factory.CreateGameObject(type);

	if (!obj)
	{
		return nullptr;
	}

	obj->Init();

	obj->SetName(
		baseName + "_" +
		std::to_string(m_objectCount[baseName]++));

	AddObject(obj);

	return obj;
}

std::shared_ptr<KdGameObject> SceneManager::DuplicateObject(std::shared_ptr<KdGameObject> src)
{
	if (!src)
	{
		return nullptr;
	}

	std::string baseName = src->GetName();

	size_t underPos =
		baseName.find_last_of('_');

	if (underPos != std::string::npos)
	{
		try
		{
			std::stoi(
				baseName.substr(underPos + 1));

			baseName =
				baseName.substr(0, underPos);
		}
		catch (...)
		{
		}
	}

	auto obj =
		CreateObject(
			src->GetTypeName(),
			baseName);

	if (!obj)
	{
		return nullptr;
	}

	obj->CopyFrom(src);

	return obj;
}

void SceneManager::UpdateObjectCount(const std::string& type, int count)
{
	if (m_objectCount[type] < count)
	{
		m_objectCount[type] = count;
	}
}

std::shared_ptr<KdGameObject> SceneManager::CreateObjectRaw(const std::string& type)
{
	auto obj = m_factory.CreateGameObject(type);

	if (!obj) return nullptr;

	obj->Init();

	return obj;
}

int SceneManager::CountItems(const std::string& itemPath) const
{
	int count = 0;

	for (auto& obj : m_currentScene->GetObjList())
	{
		if (!obj)
		{
			continue;
		}

		auto item = std::dynamic_pointer_cast<Item>(obj);

		if (!item)
		{
			continue;
		}

		if (item->GetItemPath() == itemPath)
		{
			count++;
		}
	}

	return count;
}

SceneManager::StageData SceneManager::GetStageData() const
{
	switch (m_stageType)
	{
	case StageType::Stage1:
		return
		{
			"Asset/Stage/Stage1/scene.json",
			"Asset/Stage/Stage1/astarnode.json",
			"Asset/Stage/Stage1/navmesh.json"
		};

	case StageType::Stage2:
		return
		{
			"Asset/Stage2/scene.json",
			"Asset/Stage2/astarnode.json",
			"Asset/Stage2/navmesh.json"
		};
	}

	return {};
}

void SceneManager::ChangeScene(SceneType _sceneType)
{
	// 次のシーンを作成し、現在のシーンにする
	switch (_sceneType)
	{
	case SceneType::Title:
		m_currentScene = std::make_shared<TitleScene>();
		break;
	case SceneType::Game:
		m_currentScene = std::make_shared<GameScene>();
		break;
	case SceneType::Loading:
		m_currentScene = std::make_shared<LoadingScene>();
		break;
	case SceneType::Edit:
		m_currentScene = std::make_shared<EditScene>();
		break;
	}

	// 現在のシーン情報を更新
	m_currentSceneType = _sceneType;
}

void SceneManager::RegisterGameObjects()
{
	m_factory.RegisterCreateFunction(
		"Player",
		[]()
		{
			return std::make_shared<Player>();
		}
	);

	m_factory.RegisterCreateFunction(
		"Neighbor",
		[]()
		{
			return std::make_shared<Neighbor>();
		}
	);

	m_factory.RegisterCreateFunction(
		"Police",
		[]()
		{
			return std::make_shared<Police>();
		}
	);

	m_factory.RegisterCreateFunction(
		"StaticMeshObject",
		[]()
		{
			return std::make_shared<StaticMeshObject>();
		}
	);

	m_factory.RegisterCreateFunction(
		"Tree",
		[]()
		{
			return std::make_shared<Tree>();
		}
	);

	m_factory.RegisterCreateFunction(
		"Ground",
		[]()
		{
			return std::make_shared<Ground>();
		}
	);

	m_factory.RegisterCreateFunction(
		"Item",
		[]()
		{
			return std::make_shared<Item>();
		}
	);

	m_factory.RegisterCreateFunction(
		"Flower",
		[]()
		{
			return std::make_shared<Flower>();
		}
	);

	m_factory.RegisterCreateFunction(
		"Hose",
		[]()
		{
			return std::make_shared<Hose>();
		}
	);

	m_factory.RegisterCreateFunction(
		"PaintCan",
		[]()
		{
			return std::make_shared<PaintCan>();
		}
	);

	m_factory.RegisterCreateFunction(
		"WoodenFence",
		[]()
		{
			return std::make_shared<WoodenFence>();
		}
	);
}
