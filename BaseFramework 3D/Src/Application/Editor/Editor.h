#pragma once

#include "Windows/HierarchyWindow.h"
#include "Windows/InspectorWindow.h"
#include "Windows/SceneWindow.h"
#include "Windows/AssetBrowserWindow.h"
#include "Windows/StageEditorWindow.h"

#include "../GameObject/Camera/EditorCamera/EditorCamera.h"

#include "../System/Node/AStarNode/AStarNodeManager.h"
#include "Windows/NodeEditorWindow.h"

#include "../System/Navigation/NavMesh/NavMeshGenerator.h"
#include "../System/Navigation/NavMesh/NavMeshManager.h"
#include "../System/Navigation/NavMesh/NavMeshPathfinder.h"
#include "Windows/NavigationEditorWindow.h"

#include "../System/Location/LocationPointManager.h"
#include "Windows/LocationEditorWindow.h"

class Editor
{
public:

	void Init();
	void Update();
	void Draw();

	void DrawDebug();
	void DrawGridLevel(
		KdDebugWireFrame& wire,
		const Math::Vector3& camPos,
		float distance,
		float screenHeight,
		float gridSize);

	void DrawGrid();

	void SetGameCamera(
		const std::shared_ptr<CameraBase>& camera)
	{
		m_gameCamera = camera;
	}

	void SetOpen(bool open);

	bool IsOpen() const
	{
		return m_isOpen;
	}

	enum class EditMode
	{
		Object,
		Stage,
		Navigation,
		Location,
	};

	EditMode GetEditMode() const
	{
		return m_editMode;
	}

	void SetEditMode(EditMode mode)
	{
		m_editMode = mode;
	}

	void ApplyStageBrush(const Math::Vector3& pos);


	void CreateAStarNode(const Math::Vector3& pos);

	void OnNodeClick(const Math::Vector3& pos);

	void SelectAStarNode(const Math::Vector3& pos);

	void SelectAStarNodeByID(int id);

	void ResetNodeConnection()
	{
		m_connectionStartNode = nullptr;
	}

	void GenerateNavMesh();
	void GenerateNavMeshVer2();
	void GenerateNavMeshVer3(float agentRadius);
	void ClearNavMesh();

	NavigationEditorWindow::NavigationMode
		GetNavigationMode() const
	{
		return m_navigationEditorWindow.GetMode();
	}

	void SelectNavMeshPolygon(const Math::Vector3& pos);

	void ClearNavMeshPath();

	void CalculateNavMeshPointPath();

	void OnLocationClick(const Math::Vector3& pos);

private:

	Editor() = default;

	std::unique_ptr<KdDebugWireFrame> m_pDebugWire = nullptr;

	bool m_isOpen = false;
	std::weak_ptr<CameraBase> m_returnCamera;

	void UpdateObjectMode();
	void UpdateStageMode();
	void UpdateNavigationMode();
	void UpdateLocationMode();

	std::vector<std::shared_ptr<KdGameObject>> m_objects;
	std::shared_ptr<KdGameObject> m_selectObject = nullptr;

	HierarchyWindow m_hierarchyWindow;
	InspectorWindow m_inspectorWindow;
	SceneWindow m_sceneWindow;
	AssetBrowserWindow m_assetBrowserWindow;

	bool m_showHierarchy = true;
	bool m_showInspector = true;
	bool m_showScene = true;
	bool m_showAssetBrowser = true;

	void DrawMenuBar();
	void DrawOpenSceneMenu();

	//void DeleteSelectedObject();
	//void DuplicateSelectedObject();

	std::string GetNavigationDirectory() const;

	void SaveScene();
	void LoadScene();
	void OpenScene(const std::string& path);

	std::string m_currentScenePath =
		"Asset/Scenes/---.json";

	std::shared_ptr<EditorCamera> m_editorCamera;

	EditMode m_editMode = EditMode::Object;

	void ApplyObjectLayout();

	std::weak_ptr<CameraBase> m_gameCamera;

	// ===== stage editor =====

	StageEditorWindow m_stageEditorWindow;

	static constexpr float GRID_SIZE = 4.0f;

	void ApplyStageLayout();

	std::shared_ptr<KdModelWork> m_spStagePreviewModel = nullptr;
	std::string m_previewPath;
	Math::Vector3 m_previewPos;

	struct GridCoord
	{
		int x;
		int y;
		int z;

		bool operator==(const GridCoord& other) const
		{
			return x == other.x &&
				y == other.y &&
				z == other.z;
		}
	};

	GridCoord WorldToGrid(Math::Vector3 pos)
	{
		return
		{
			(int)round(pos.x / GRID_SIZE),
			(int)round(pos.y / GRID_SIZE),
			(int)round(pos.z / GRID_SIZE)
		};
	}

	void UpdateStagePreviewPos();

	void DrawStagePreview();
	void DrawErasePreview();

	bool ExistsStageTile(const Math::Vector3& pos);

	void PlaceStageTile(const Math::Vector3& pos);
	void EraseStageTile(const Math::Vector3& pos);
	
	StageEditorWindow::BrushMode GetBrushMode() const
	{
		return m_stageEditorWindow.GetBrushMode();
	}

	// ===== navigation =====

	NavigationEditorWindow m_navigationEditorWindow;

	void SaveNavigation();
	void LoadNavigation();

	// ===== astar node editor =====

	NodeEditorWindow m_nodeEditorWindow;

	std::shared_ptr<AStarNode> m_selectNode;
	std::shared_ptr<AStarNode> m_connectionStartNode;

	void ApplyNavigationLayout();

	void DrawAStarNodes(); 

	void SaveNode();
	void LoadNode();

	// ===== nav mesh editor =====

	NavMeshGenerator m_navMeshGenerator;
	NavMeshPathfinder m_navMeshPathfinder;

	std::shared_ptr<NavMeshPolygon> m_navMeshStartPolygon;
	std::shared_ptr<NavMeshPolygon> m_navMeshGoalPolygon;

	std::vector<int> m_navMeshPath;

	std::vector<Math::Vector3> m_navMeshPointPath;

	void DrawNavMesh();

	void SaveNavMesh();
	void LoadNavMesh();

	void CalculateNavMeshPath();

	bool IsPointInTriangleXZ(
		const Math::Vector3& p,
		const Math::Vector3& a,
		const Math::Vector3& b,
		const Math::Vector3& c);

	std::shared_ptr<NavMeshPolygon>
		FindNavMeshPolygon(
			const Math::Vector3& pos);

	//============================================================
	// Navigation Debug Colors
	//============================================================

	const Math::Color kNavMeshColor =
	{
		0.1f, 0.8f, 0.35f, 1.0f
	};

	const Math::Color kConnectionColor =
	{
		1.0f, 0.65f, 0.05f, 1.0f
	};

	const Math::Color kStartColor =
	{
		0.1f, 0.4f, 1.0f, 1.0f
	};

	const Math::Color kGoalColor =
	{
		1.0f, 0.15f, 0.15f, 1.0f
	};

	const Math::Color kAStarPathColor =
	{
		0.9f, 0.9f, 1.0f, 1.0f
	};

	const Math::Color kActualPathColor =
	{
		1.0f, 0.1f, 0.8f, 1.0f
	};

	const Math::Color kWaypointColor =
	{
		0.5f, 0.1f, 0.9f, 1.0f
	};

	// ===== location editor =====

	LocationEditorWindow m_locationEditorWindow;

	void ApplyLocationLayout();
	void DrawLocationPoints();

	void SaveLocation();
	void LoadLocation();

public:
	static Editor& Instance()
	{
		static Editor instance;
		return instance;
	}

};