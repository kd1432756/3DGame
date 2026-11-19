#include "EditorIO.h"

#include "../Scene/SceneSerializer/SceneSerializer.h"
#include "../System/Node/AStarNode/AStarNodeManager.h"
#include "../System/Node/AStarNode/AStarNodeSerializer.h"
#include "../System/Navigation/NavMesh/NavMeshManager.h"
#include "../System/Navigation/NavMesh/NavMeshSerializer.h"
#include "../System/Location/LocationPointManager.h"
#include "../System/Location/LocationPointSerializer.h"

const char* EditorIO::StageName(SceneManager::StageType stage)
{
	switch (stage)
	{
	case SceneManager::StageType::Stage1: return "Stage1";
	case SceneManager::StageType::Stage2: return "Stage2";
	case SceneManager::StageType::Stage3: return "Stage3";
	}

	return "Unknown";
}

EditorIO::Paths EditorIO::ResolvePaths()  const
{
	const auto data = SceneManager::Instance().GetStageData(m_stage);
	return { data.scenePath,data.astarPath, data.navMeshPath, data.locationPointPath };
}

//============================================================
// ステージ全体
//============================================================

bool EditorIO::Save()
{
	// 1つ失敗しても残りは保存する
	const bool scene = SaveScene();
	const bool nodes = SaveNodes();
	const bool navMesh = SaveNavMesh();
	const bool location = SaveLocations();

	const bool ok = scene && nodes && navMesh && location;

	m_status = ok ? "Saved" : "Save FAILED";

	return ok;
}

bool EditorIO::Load()
{
	// 1つ失敗しても残りは読む
	const bool scene = LoadScene();
	const bool nodes = LoadNodes();
	const bool navMesh = LoadNavMesh();
	const bool location = LoadLocations();

	const bool ok = scene && nodes && navMesh && location;

	m_status = ok ? "Loaded" : "Load FAILED";

	return ok;
}

//------------------------------------------------------------
// Scene
//------------------------------------------------------------
bool EditorIO::SaveScene()
{
	if (!SceneSerializer::Save(ResolvePaths().scene))
	{
		return false;
	}

	return true;
}

bool EditorIO::LoadScene()
{
	if (!SceneSerializer::Load(ResolvePaths().scene))
	{
		return false;
	}

	return true;
}

//------------------------------------------------------------
// A*Node
//------------------------------------------------------------
bool EditorIO::SaveNodes()
{
	if (!AStarNodeSerializer::Save(ResolvePaths().astarNode, AStarNodeManager::Instance()))
	{
		return false;
	}
	return true;
}

bool EditorIO::LoadNodes()
{
	auto& manager = AStarNodeManager::Instance();
	manager.Clear();

	if (!AStarNodeSerializer::Load(ResolvePaths().astarNode, manager))
	{
		return false;
	}
	return true;
}

//------------------------------------------------------------
// NavMesh
//------------------------------------------------------------
bool EditorIO::SaveNavMesh()
{
	if (!NavMeshSerializer::Save(ResolvePaths().navMesh, NavMeshManager::Instance()))
	{
		return false;
	}
	return true;
}

bool EditorIO::LoadNavMesh()
{
	auto& manager = NavMeshManager::Instance();
	manager.Clear();

	if (!NavMeshSerializer::Load(ResolvePaths().navMesh, manager))
	{
		return false;
	}
	return true;
}

//------------------------------------------------------------
// LocationPoint
//------------------------------------------------------------
bool EditorIO::SaveLocations()
{
	if (!LocationPointSerializer::Save(ResolvePaths().location, LocationPointManager::Instance()))
	{
		return false;
	}
	return true;
}

bool EditorIO::LoadLocations()
{
	auto& manager = LocationPointManager::Instance();
	manager.Clear();

	if (!LocationPointSerializer::Load(ResolvePaths().location, manager))
	{
		return false;
	}
	return true;
}