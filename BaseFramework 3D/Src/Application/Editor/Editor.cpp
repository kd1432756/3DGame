#include "Editor.h"

#include "../Scene/SceneManager.h"
#include "../Scene/CameraManager/CameraManager.h"
#include "../GameObject/Character/Player/Player.h"
#include "../GameObject/Object/StaticMeshObject/StaticMeshObject.h"

#include "../Scene/SceneSerializer/SceneSerializer.h"

#include "../System/Node/AStarNode/AStarNodeSerializer.h"

#include "../System/Navigation/NavMesh/NavMeshSerializer.h"

#include "../GameObject/Object/StaticMeshObject/Ground/Ground.h"

#include "../System/Navigation/NavMesh/NavMeshPolygonBuilder.h"
#include "../System/Navigation/NavMesh/NavMeshCDT.h"

#include "../System/Location/LocationPointSerializer.h"

void Editor::Init()
{
	m_editorCamera = std::make_shared<EditorCamera>();
	m_editorCamera->Init();

	m_stageEditorWindow.Init();

	m_pDebugWire = std::make_unique<KdDebugWireFrame>();
}

void Editor::Update()
{
	switch (m_editMode)
	{
	case EditMode::Object:
		UpdateObjectMode();
		break;

	case EditMode::Stage:
		UpdateStageMode();
		break;
	case EditMode::Navigation:
		UpdateNavigationMode();
		break;
	case EditMode::Location:
		UpdateLocationMode();
		break;
	}
}

void Editor::UpdateObjectMode()
{
	ImGuiIO& io = ImGui::GetIO();

	// Delete
	if (m_selectObject && ImGui::IsKeyPressed(ImGuiKey_Delete))
	{
		m_selectObject->Destroy();
		m_selectObject = nullptr;
	}

	// Ctrl + D
	if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_D))
	{
		if (m_selectObject)
		{
			auto duplicatedObject = SceneManager::Instance().DuplicateObject(m_selectObject);

			if (!duplicatedObject)
			{
				return;
			}

			m_selectObject = duplicatedObject;
		}
	}

	// Ctrl + S
	if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_S))
	{
		SaveScene();
	}
}

void Editor::UpdateStageMode()
{
	UpdateStagePreviewPos();

	//---------------------------------
	// Brush
	//---------------------------------

	if (ImGui::IsKeyPressed(ImGuiKey_B))
	{
		m_stageEditorWindow.SetBrushMode(
			StageEditorWindow::BrushMode::Paint);
	}

	if (ImGui::IsKeyPressed(ImGuiKey_E))
	{
		m_stageEditorWindow.SetBrushMode(
			StageEditorWindow::BrushMode::Erase);
	}

	//---------------------------------
	// Rotate
	//---------------------------------

	if (ImGui::IsKeyPressed(ImGuiKey_R))
	{
		m_stageEditorWindow.Rotate90();
	}
}

void Editor::UpdateNavigationMode()
{
	if (m_navigationEditorWindow.GetMode() ==
		NavigationEditorWindow::NavigationMode::Node)
	{
		//------------------------------------------------
		// Node
		//------------------------------------------------

		if (m_selectNode &&
			ImGui::IsKeyPressed(ImGuiKey_Delete))
		{
			int id =
				m_selectNode->GetID();

			m_astarNodeManager.DeleteNode(id);

			m_selectNode = nullptr;

			m_connectionStartNode = nullptr;

			m_nodeEditorWindow.SetSelectedNode(-1);
		}

		return;
	}

	//------------------------------------------------
	// NavMesh
	//------------------------------------------------

	// SceneWindow側からクリックされた場所を
	// ここで受け取る仕組みにする
}

void Editor::UpdateLocationMode()
{
	if (m_locationEditorWindow.GetMode() ==
		LocationEditorWindow::Mode::Select)
	{
		auto point =
			m_locationPointManager.FindPoint(
				m_locationEditorWindow.GetSelectedPointID());

		if (point && ImGui::IsKeyPressed(ImGuiKey_Delete))
		{
			m_locationPointManager.DeletePoint(point->GetID());

			m_locationEditorWindow.SetSelectedPoint(-1);
		}
	}
}

void Editor::UpdateStagePreviewPos()
{
	m_previewPos =
		m_sceneWindow.GetDropPosition(m_editorCamera);

	m_previewPos.x = roundf(m_previewPos.x / GRID_SIZE) * GRID_SIZE;
	m_previewPos.y = roundf(m_previewPos.y / GRID_SIZE) * GRID_SIZE;
	m_previewPos.z = roundf(m_previewPos.z / GRID_SIZE) * GRID_SIZE;
}

void Editor::PlaceStageTile(const Math::Vector3& pos)
{
	if (ExistsStageTile(pos))
	{
		return;
	}

	const auto* tile =
		m_stageEditorWindow.GetSelectedTile();

	if (!tile)
	{
		return;
	}

	auto obj =
		SceneManager::Instance().CreateObject("Ground");

	auto ground =
		std::dynamic_pointer_cast<Ground>(obj);

	if (!ground)
	{
		return;
	}

	ground->SetModelPath(tile->modelPath);

	ground->SetPos(m_previewPos);

	ground->SetScale(GRID_SIZE / 8.0f);

	ground->SetRot({ 0, m_stageEditorWindow.GetRotation(), 0 });

	ground->SetGroundType(tile->groundType);
}

void Editor::EraseStageTile(const Math::Vector3& pos)
{
	GridCoord target = WorldToGrid(pos);

	for (auto& obj : SceneManager::Instance().GetObjList())
	{
		auto mesh =
			std::dynamic_pointer_cast<StaticMeshObject>(obj);

		if (!mesh)
		{
			continue;
		}

		if (WorldToGrid(mesh->GetPos()) == target)
		{
			mesh->Destroy();
			return;
		}
	}
}

void Editor::ApplyObjectLayout()
{
	// Hierarchy
	m_hierarchyWindow.SetLayout(
		{ 0, 20 },
		{ 250, 500 });

	// Scene
	m_sceneWindow.SetLayout(
		{ 250, 20 },
		{ 780, 500 });

	// Inspector
	m_inspectorWindow.SetLayout(
		{ 1030, 20 },
		{ 250, 500 });

	// AssetBrowser
	m_assetBrowserWindow.SetLayout(
		{ 0, 520 },
		{ 1280, 200 });
}

void Editor::ApplyStageLayout()
{
	// Scene
	m_sceneWindow.SetLayout(
		{ 0,20 },
		{ 1030,700 });


	// Stage Editor
	m_stageEditorWindow.SetLayout(
		{ 1030,20 },
		{ 250,700 });
}

void Editor::ApplyNavigationLayout()
{
	// Scene
	m_sceneWindow.SetLayout(
		{ 0, 20 },
		{ 1030, 700 });

	// Navigation Editor
	m_navigationEditorWindow.SetLayout(
		{ 1030, 20 },
		{ 250, 700 });
}

void Editor::ApplyLocationLayout()
{
	m_sceneWindow.SetLayout(
		{ 0, 20 },
		{ 1030, 700 });

	m_locationEditorWindow.SetLayout(
		{ 1030, 20 },
		{ 250, 700 });
}

void Editor::Draw()
{
	if (!m_isOpen) return;

	DrawMenuBar();

	switch (m_editMode)
	{
	case EditMode::Object:
		ApplyObjectLayout();
		break;

	case EditMode::Stage:
		ApplyStageLayout();
		break;

	case EditMode::Navigation:
		ApplyNavigationLayout();
		break;

	case EditMode::Location:
		ApplyLocationLayout();
		break;
	}

	if (m_showScene)
	{
		m_sceneWindow.Draw(
			m_editorCamera,
			m_selectObject);
	}

	switch (m_editMode)
	{
	case EditMode::Object:

		if (m_showHierarchy)
		{
			m_hierarchyWindow.Draw(m_selectObject);
		}

		if (m_showInspector)
		{
			m_inspectorWindow.Draw(m_selectObject);
		}

		if (m_showAssetBrowser)
		{
			m_assetBrowserWindow.Draw();
		}

		break;

	case EditMode::Stage:

		m_stageEditorWindow.Draw();

		break;

	case EditMode::Navigation:

		m_navigationEditorWindow.Draw(m_nodeEditorWindow);

		break;

	case EditMode::Location:

		m_locationEditorWindow.Draw();

		break;
	}
}

void Editor::DrawDebug()
{
	if (!m_isOpen) return;

	//DrawGrid();

	if (m_editMode == EditMode::Stage)
	{
		if (m_stageEditorWindow.GetBrushMode() ==
			StageEditorWindow::BrushMode::Paint)
		{
			DrawStagePreview();
		}
		else
		{
			DrawErasePreview();
		}
	}

	if (m_editMode == EditMode::Navigation)
	{
		if (m_navigationEditorWindow.GetMode() ==
			NavigationEditorWindow::NavigationMode::Node)
		{
			DrawAStarNodes();
		}
		else
		{
			if (m_navigationEditorWindow.IsShowNavMesh())
			{
				DrawNavMesh();
			}

			//------------------------------------------------
			// Start
			//------------------------------------------------

			if (m_navMeshStartPolygon)
			{
				m_pDebugWire->AddDebugSphere(
					m_navMeshStartPolygon->GetCenter(),
					0.3f,
					kStartColor);
			}

			//------------------------------------------------
			// Goal
			//------------------------------------------------

			if (m_navMeshGoalPolygon)
			{
				m_pDebugWire->AddDebugSphere(
					m_navMeshGoalPolygon->GetCenter(),
					0.3f,
					kGoalColor);
			}

			//------------------------------------------------
			// A* Path
			//------------------------------------------------

/*			for (size_t i = 1;
				i < m_navMeshPath.size();
				++i)
			{
				auto from =
					NavMeshManager::Instance().FindPolygon(
						m_navMeshPath[i - 1]);

				auto to =
					NavMeshManager::Instance().FindPolygon(
						m_navMeshPath[i]);

				if (!from || !to)
				{
					continue;
				}

				m_pDebugWire->AddDebugLine(
					from->GetCenter(),
					to->GetCenter(),
					kAStarPathColor);
			}*/

			//------------------------------------------------
			// Actual Point Path
			//
			// A* Polygon Path
			//     ↓
			// Portal
			//     ↓
			// Funnel
			//     ↓
			// 保存済みPoint
			//------------------------------------------------

			//------------------------------------------------
			// Actual Route
			//
			// 緑線 = Funnel後の実際の経路
			//------------------------------------------------

			for (size_t i = 1;
				i < m_navMeshPointPath.size();
				++i)
			{
				m_pDebugWire->AddDebugLine(
					m_navMeshPointPath[i - 1],
					m_navMeshPointPath[i],
					kActualPathColor);
			}

			//------------------------------------------------
			// Waypoint
			//
			// Funnelで生成されたPointを表示
			//------------------------------------------------

			for (size_t i = 1;
				i + 1 < m_navMeshPointPath.size();
				++i)
			{
				m_pDebugWire->AddDebugSphere(
					m_navMeshPointPath[i],
					0.1f,
					kWaypointColor);
			}

			m_pDebugWire->Draw();
		}
	}

	if (m_editMode == EditMode::Location)
	{
		DrawLocationPoints();
	}
}

std::string Editor::GetNavigationDirectory() const
{
	std::filesystem::path scenePath(m_currentScenePath);

	std::string sceneName =
		scenePath.stem().string();

	return "Asset/Navigation/" + sceneName;
}

void Editor::SaveScene()
{
	if (!SceneSerializer::Save(m_currentScenePath))
	{
		assert(false && "Scene Save Failed");
	}
}

void Editor::OpenScene(const std::string& path)
{
	m_currentScenePath = path;
	LoadScene();
}

void Editor::SaveNavigation()
{
	SaveNode();
	SaveNavMesh();
}

void Editor::SaveNode()
{
/*	const std::string dir =
		GetNavigationDirectory();

	std::filesystem::create_directories(dir);

	AStarNodeSerializer::Save(
		dir + "/nodes.json",
		m_astarNodeManager);*/

	if (!AStarNodeSerializer::Save(
		"Asset/Data/astarnode.json",
		m_astarNodeManager))
	{
		assert(false && "Navigation Save Failed");
	}
}

void Editor::SaveNavMesh()
{
/*	const std::string dir =
		GetNavigationDirectory();

	std::filesystem::create_directories(dir);

	AStarNodeSerializer::Save(
		dir + "/navmesh.json",
		m_astarNodeManager);*/

	if (!NavMeshSerializer::Save(
		"Asset/Data/navmesh.json",
		NavMeshManager::Instance()))
	{
		assert(false && "NavMesh Save Failed");
	}
}

void Editor::SaveLocation()
{
	if (!LocationPointSerializer::Save(
		"Asset/Data/location.json",
		m_locationPointManager))
	{
		assert(false && "Location Save Failed");
	}
}

void Editor::LoadScene()
{
	if (!SceneSerializer::Load(m_currentScenePath))
	{
		assert(false && "Scene Load Failed");
	}
}

void Editor::LoadNavigation()
{
	LoadNode();
	LoadNavMesh();
}

void Editor::LoadNode()
{
	if (!AStarNodeSerializer::Load(
		"Asset/Data/astarnode.json",
		m_astarNodeManager))
	{
		assert(false && "Navigation Load Failed");
	}

	m_selectNode = nullptr;

	m_connectionStartNode = nullptr;

	m_nodeEditorWindow.SetSelectedNode(-1);
}

void Editor::LoadNavMesh()
{
	if (!NavMeshSerializer::Load(
		"Asset/Data/navmesh.json",
		NavMeshManager::Instance()))
	{
		assert(false && "NavMesh Load Failed");
	}

	ClearNavMeshPath();
}

void Editor::LoadLocation()
{
	if (!LocationPointSerializer::Load(
		"Asset/Data/location.json",
		m_locationPointManager))
	{
		assert(false && "Location Load Failed");
	}

	m_locationEditorWindow.SetSelectedPoint(-1);
}

void Editor::DrawGrid()
{
	KdDebugWireFrame wire;

	Math::Vector3 camPos = m_editorCamera->GetPos();

	float distance = m_editorCamera->GetDistance();

	float sceneHeight = 720.0f;

	std::array<float, 4> gridLevels = { 1, 5, 10, 50 };

	for (float size : gridLevels)
	{
		DrawGridLevel(wire, camPos, distance, sceneHeight, size);
	}

	wire.Draw();
}

void Editor::DrawStagePreview()
{
	const auto tile =
		m_stageEditorWindow.GetSelectedTile();

	if (!tile) return;

	if (ExistsStageTile(m_previewPos))
	{
		return;
	}

	//------------------------------------------------
	// モデル変更時だけ読み直す
	//------------------------------------------------

	if (m_previewPath != tile->modelPath)
	{
		if (!m_spStagePreviewModel)
		{
			m_spStagePreviewModel =
				std::make_shared<KdModelWork>();
		}

		m_spStagePreviewModel->SetModelData(
			tile->modelPath);

		m_previewPath = tile->modelPath;
	}

/*	//------------------------------------------------
	// 位置
	//------------------------------------------------

	m_previewPos =
		m_sceneWindow.GetDropPosition(m_editorCamera);

	//------------------------------------------------
	// GridSnap
	//------------------------------------------------
	
	const float grid = 8.0f;

	m_previewPos.x = roundf(m_previewPos.x / grid) * grid;
	m_previewPos.y = roundf(m_previewPos.y / grid) * grid;
	m_previewPos.z = roundf(m_previewPos.z / grid) * grid;*/

	//------------------------------------------------
	// Matrix
	//------------------------------------------------

	Math::Matrix scale =
		Math::Matrix::CreateScale(GRID_SIZE / 8.0f);

	Math::Matrix rot =
		Math::Matrix::CreateRotationY(
			DirectX::XMConvertToRadians(
				m_stageEditorWindow.GetRotation()));

	Math::Matrix trans =
		Math::Matrix::CreateTranslation(m_previewPos);

	Math::Matrix previewMatrix =
		scale * rot * trans;

	//------------------------------------------------
	// 描画
	//------------------------------------------------

	KdShaderManager::Instance().m_StandardShader.DrawModel(
		*m_spStagePreviewModel,
		previewMatrix
	);
}

void Editor::DrawErasePreview()
{
	Math::Matrix mat =
		Math::Matrix::CreateTranslation(m_previewPos);

	m_pDebugWire->AddDebugBox(
		mat,
		{ GRID_SIZE / 2.0f, 0.5f, GRID_SIZE / 2.0f },      // 半サイズ
		Math::Vector3::Zero,
		true,
		kRedColor);

	m_pDebugWire->Draw();
}

void Editor::DrawAStarNodes()
{
	if (!m_pDebugWire)
	{
		return;
	}

	//------------------------------------------------
	// Connection
	//------------------------------------------------

	for (auto& node :
		m_astarNodeManager.GetNodes())
	{
		if (!node)
		{
			continue;
		}

		const Math::Vector3& from =
			node->GetPos();

		for (int connectionID :
		node->GetConnections())
		{
			auto to =
				m_astarNodeManager.FindNode(connectionID);

			if (!to)
			{
				continue;
			}

			m_pDebugWire->AddDebugLine(
				from,
				to->GetPos(),
				kConnectionColor);
		}
	}

	//------------------------------------------------
	// Node Sphere
	//------------------------------------------------

	for (auto& node :
		m_astarNodeManager.GetNodes())
	{
		if (!node)
		{
			continue;
		}

		const Math::Vector3& pos =
			node->GetPos();

		Math::Matrix mat =
			Math::Matrix::CreateTranslation(pos);

		Math::Color color = kGreenColor;

		if (node == m_selectNode)
		{
			color = kYellowColor;
		}

		if (node == m_connectionStartNode)
		{
			color = kRedColor;
		}

		m_pDebugWire->AddDebugSphere(
			pos,
			0.1f,
			color);
	}

	m_pDebugWire->Draw();

	//------------------------------------------------
	// Node ID
	//------------------------------------------------

	for (auto& node :
		m_astarNodeManager.GetNodes())
	{
		if (!node)
		{
			continue;
		}

		Math::Vector3 worldPos =
			node->GetPos();

		Math::Vector3 camPos =
			m_editorCamera->GetPos();

		float distance =
			(worldPos - camPos).Length();

		float offset =
			std::clamp(
				distance * 0.02f,
				0.3f,
				2.0f);

		worldPos.y += offset;

		Math::Vector3 screenPos =
			Math::Vector3::Zero;

		if (!m_editorCamera->ConvertWorldToScreenDetail(
			worldPos,
			screenPos))
		{
			continue;
		}

		char text[32];

		sprintf_s(
			text,
			"Node %d",
			node->GetID());

		// Font描画
		KdShaderManager::Instance().m_spriteShader.DrawFont(
			{ screenPos.x, screenPos.y },
			&kWhiteColor,
			{ 0.5f, 0.5f },
			0.0f,
			FontID::Future16,
			text);
	}
}

void Editor::DrawNavMesh()
{
	if (!m_pDebugWire)
	{
		return;
	}

	const auto& polygons =
		NavMeshManager::Instance().GetPolygons();

	for (const auto& polygon : polygons)
	{
		if (!polygon)
		{
			continue;
		}

		const auto& v =
			polygon->GetVertices();

		Math::Color color =
			kGreenColor;

		//------------------------------------------------
		// Triangle
		//------------------------------------------------

		m_pDebugWire->AddDebugLine(
			v[0],
			v[1],
			color);

		m_pDebugWire->AddDebugLine(
			v[1],
			v[2],
			color);

		m_pDebugWire->AddDebugLine(
			v[2],
			v[0],
			color);

		//------------------------------------------------
		// Connection
		//------------------------------------------------

		for (int connectionID :
		polygon->GetConnections())
		{
			if (polygon->GetID() >= connectionID)
			{
				continue;
			}

			auto connection =
				NavMeshManager::Instance().FindPolygon(
					connectionID);

			if (!connection)
			{
				continue;
			}

			m_pDebugWire->AddDebugLine(
				polygon->GetCenter(),
				connection->GetCenter(),
				kYellowColor);
		}
	}

	m_pDebugWire->Draw();
}

void Editor::DrawLocationPoints()
{
	if (!m_pDebugWire)
	{
		return;
	}

	for (auto& point : m_locationPointManager.GetPoints())
	{
		if (!point)
		{
			continue;
		}

		Math::Color color = kGreenColor;

		if (point->GetID() == m_locationEditorWindow.GetSelectedPointID())
		{
			color = kYellowColor;
		}

		m_pDebugWire->AddDebugSphere(
			point->GetPos(),
			0.3f,
			color);
	}

	m_pDebugWire->Draw();

	// Node IDと同じ要領でタグ名テキストも描画する場合はここに追加
	for (auto& point : m_locationPointManager.GetPoints())
	{
		if (!point)
		{
			continue;
		}

		Math::Vector3 worldPos = point->GetPos();

		worldPos.y += 0.5f;

		Math::Vector3 screenPos = Math::Vector3::Zero;

		if (!m_editorCamera->ConvertWorldToScreenDetail(worldPos, screenPos))
		{
			continue;
		}

		KdShaderManager::Instance().m_spriteShader.DrawFont(
			{ screenPos.x, screenPos.y },
			&kWhiteColor,
			{ 0.5f, 0.5f },
			0.0f,
			FontID::Future16,
			LocationTagToString(point->GetTag()));
	}
}

bool Editor::ExistsStageTile(const Math::Vector3& pos)
{
	GridCoord targetGrid = WorldToGrid(pos);

	for (auto& obj : SceneManager::Instance().GetObjList())
	{
		auto ground =
			std::dynamic_pointer_cast<Ground>(obj);

		if (!ground)
		{
			continue;
		}

		GridCoord objGrid =
			WorldToGrid(ground->GetPos());

		if (objGrid == targetGrid)
		{
			return true;
		}
	}

	return false;
}

void Editor::ApplyStageBrush(const Math::Vector3& pos)
{
	switch (m_stageEditorWindow.GetBrushMode())
	{
	case StageEditorWindow::BrushMode::Paint:
		PlaceStageTile(pos);
		break;

	case StageEditorWindow::BrushMode::Erase:
		EraseStageTile(pos);
		break;
	}
}

void Editor::OnNodeClick(
	const Math::Vector3& pos)
{
	constexpr float NODE_SELECT_RADIUS = 1.0f;

	std::shared_ptr<AStarNode> nearestNode = nullptr;

	float nearestDistanceSq =
		NODE_SELECT_RADIUS * NODE_SELECT_RADIUS;

	for (auto& node :
		m_astarNodeManager.GetNodes())
	{
		if (!node)
		{
			continue;
		}

		Math::Vector3 diff =
			node->GetPos() - pos;

		float distanceSq =
			diff.LengthSquared();

		if (distanceSq <= nearestDistanceSq)
		{
			nearestDistanceSq = distanceSq;
			nearestNode = node;
		}
	}

	auto mode =
		m_nodeEditorWindow.GetMode();

	//------------------------------------------------
	// Select
	//------------------------------------------------

	if (mode ==
		NodeEditorWindow::Mode::Select)
	{
		SelectAStarNode(pos);
		return;
	}

	//------------------------------------------------
	// Create
	//------------------------------------------------

	if (mode ==
		NodeEditorWindow::Mode::Create)
	{
		if (!nearestNode)
		{
			CreateAStarNode(pos);
		}

		return;
	}

	//------------------------------------------------
	// Erase
	//------------------------------------------------

	if (mode ==
		NodeEditorWindow::Mode::Erase)
	{
		if (nearestNode)
		{
			int id =
				nearestNode->GetID();

			m_astarNodeManager.DeleteNode(id);

			if (m_selectNode == nearestNode)
			{
				m_selectNode = nullptr;
			}

			if (m_connectionStartNode == nearestNode)
			{
				m_connectionStartNode = nullptr;
			}

			m_nodeEditorWindow.SetSelectedNode(-1);
		}

		return;
	}

	//------------------------------------------------
	// Connect
	//------------------------------------------------

	if (mode ==
		NodeEditorWindow::Mode::Connect)
	{
		if (!nearestNode)
		{
			m_connectionStartNode = nullptr;
			return;
		}

		//------------------------------------------------
		// 1個目
		//------------------------------------------------

		if (!m_connectionStartNode)
		{
			m_connectionStartNode = nearestNode;

			m_selectNode = nearestNode;

			m_nodeEditorWindow.SetSelectedNode(
				nearestNode->GetID());

			return;
		}

		//------------------------------------------------
		// 同じNode
		//------------------------------------------------

		if (m_connectionStartNode == nearestNode)
		{
			m_connectionStartNode = nullptr;
			return;
		}

		//------------------------------------------------
		// 2個目
		//------------------------------------------------

		m_astarNodeManager.ConnectNodes(
			m_connectionStartNode->GetID(),
			nearestNode->GetID());

		m_connectionStartNode = nullptr;

		m_selectNode = nearestNode;

		m_nodeEditorWindow.SetSelectedNode(
			nearestNode->GetID());
	}

	//------------------------------------------------
	// Disconnect
	//------------------------------------------------

	if (mode ==
		NodeEditorWindow::Mode::Disconnect)
	{
		if (!nearestNode)
		{
			m_connectionStartNode = nullptr;
			return;
		}

		//------------------------------------------------
		// 1個目
		//------------------------------------------------

		if (!m_connectionStartNode)
		{
			m_connectionStartNode = nearestNode;

			m_selectNode = nearestNode;

			m_nodeEditorWindow.SetSelectedNode(
				nearestNode->GetID());

			return;
		}

		//------------------------------------------------
		// 同じNode
		//------------------------------------------------

		if (m_connectionStartNode == nearestNode)
		{
			m_connectionStartNode = nullptr;
			return;
		}

		//------------------------------------------------
		// 2個目
		//------------------------------------------------

		m_astarNodeManager.DisconnectNodes(
			m_connectionStartNode->GetID(),
			nearestNode->GetID());

		m_connectionStartNode = nullptr;

		m_selectNode = nearestNode;

		m_nodeEditorWindow.SetSelectedNode(
			nearestNode->GetID());
	}

}

void Editor::CreateAStarNode(const Math::Vector3& pos)
{
	Math::Vector3 nodePos = pos;
	nodePos.y = 0.5f;

	auto node = m_astarNodeManager.CreateNode(nodePos);

	if (!node)
	{
		return;
	}

	m_selectNode = node;

	m_nodeEditorWindow.SetSelectedNode(
		node->GetID());
}

void Editor::SelectAStarNode(const Math::Vector3& pos)
{
	constexpr float NODE_SELECT_RADIUS = 1.0f;

	std::shared_ptr<AStarNode> nearestNode = nullptr;

	float nearestDistanceSq =
		NODE_SELECT_RADIUS * NODE_SELECT_RADIUS;

	for (auto& node :
		m_astarNodeManager.GetNodes())
	{
		if (!node)
		{
			continue;
		}

		Math::Vector3 diff =
			node->GetPos() - pos;

		float distanceSq =
			diff.LengthSquared();

		if (distanceSq <= nearestDistanceSq)
		{
			nearestDistanceSq = distanceSq;
			nearestNode = node;
		}
	}

	if (nearestNode)
	{
		m_selectNode = nearestNode;

		// NodeEditorWindow側の選択も同期
		m_nodeEditorWindow.SetSelectedNode(
			nearestNode->GetID());
	}
	else
	{
		m_selectNode = nullptr;

		m_nodeEditorWindow.SetSelectedNode(-1);
	}
}

void Editor::SelectAStarNodeByID(int id)
{
	auto node =
		m_astarNodeManager.FindNode(id);

	if (!node)
	{
		m_selectNode = nullptr;
		m_nodeEditorWindow.SetSelectedNode(-1);
		return;
	}

	m_selectNode = node;

	m_nodeEditorWindow.SetSelectedNode(
		node->GetID());
}

void Editor::GenerateNavMesh()
{
	m_navMeshStartPolygon = nullptr;
	m_navMeshGoalPolygon = nullptr;
	m_navMeshPath.clear();

	//--------------------------------------------------------
	// Generator入力
	//--------------------------------------------------------

	std::vector<std::pair<
		std::shared_ptr<KdModelData>,
		Math::Matrix
		>> meshes;

	std::vector<NavMeshGenerator::NavMeshObstacleBounds> obstacles;

	//--------------------------------------------------------
	// Scene Object取得
	//--------------------------------------------------------

	for (auto& obj :
		SceneManager::Instance().GetObjList())
	{
		//----------------------------------------------------
		// Ground
		//----------------------------------------------------

		if (auto ground =
			std::dynamic_pointer_cast<Ground>(obj))
		{
			auto modelData =
				ground->GetModelData();

			if (!modelData)
			{
				continue;
			}

			Math::Matrix worldMatrix =
				ground->GetMatrix();

			meshes.push_back(
				{
					modelData,
					worldMatrix
				});

			continue;
		}

		//----------------------------------------------------
		// StaticMeshObject
		//----------------------------------------------------

		auto staticMeshObject =
			std::dynamic_pointer_cast<StaticMeshObject>(obj);

		if (!staticMeshObject ||
			staticMeshObject->GetCategory() != KdGameObject::ObjectCategory::Prop)
		{
			continue;
		}

		//----------------------------------------------------
		// ModelData取得
		//----------------------------------------------------

		auto modelData =
			staticMeshObject->GetModelData();

		if (!modelData)
		{
			continue;
		}

		//----------------------------------------------------
		// World Matrix
		//----------------------------------------------------

		const Math::Matrix worldMatrix =
			staticMeshObject->GetMatrix();

		//----------------------------------------------------
		// Node取得
		//----------------------------------------------------

		const auto& nodes =
			modelData->GetOriginalNodes();

		const auto& collisionNodeIndices =
			modelData->GetCollisionMeshNodeIndices();

		//----------------------------------------------------
		// COL Nodeごとに処理
		//----------------------------------------------------

		for (int nodeIndex :
		collisionNodeIndices)
		{
			if (nodeIndex < 0 ||
				nodeIndex >= static_cast<int>(nodes.size()))
			{
				continue;
			}

			const auto& node =
				nodes[nodeIndex];

			if (!node.m_spMesh)
			{
				continue;
			}

			//------------------------------------------------
			// Local AABB
			//------------------------------------------------

			const auto& localAABB =
				node.m_spMesh->GetBoundingBox();

			//------------------------------------------------
			// World AABB
			//------------------------------------------------

			DirectX::BoundingBox worldAABB;

			Math::Matrix transform =
				node.m_worldTransform *
				worldMatrix;

			localAABB.Transform(
				worldAABB,
				transform);

			//------------------------------------------------
			// XZ Bounds
			//------------------------------------------------

			NavMeshGenerator::NavMeshObstacleBounds obstacle;

			obstacle.minX =
				worldAABB.Center.x -
				worldAABB.Extents.x;

			obstacle.maxX =
				worldAABB.Center.x +
				worldAABB.Extents.x;

			obstacle.minZ =
				worldAABB.Center.z -
				worldAABB.Extents.z;

			obstacle.maxZ =
				worldAABB.Center.z +
				worldAABB.Extents.z;

			obstacles.push_back(
				obstacle);
		}
	}

	//--------------------------------------------------------
	// Generate
	//--------------------------------------------------------

	m_navMeshGenerator.Generate(
		NavMeshManager::Instance(),
		meshes,
		obstacles);
}

void Editor::GenerateNavMeshVer2()
{
	m_navMeshStartPolygon = nullptr;
	m_navMeshGoalPolygon = nullptr;
	m_navMeshPath.clear();

	//--------------------------------------------------------
	// Generator入力
	//--------------------------------------------------------

	std::vector<NavMeshGenerator::NavMeshTile>
		stageTiles;

	std::vector<NavMeshGenerator::NavMeshObstacleBounds>
		obstacles;

	//--------------------------------------------------------
	// Scene Object取得
	//--------------------------------------------------------

	for (auto& obj :
		SceneManager::Instance().GetObjList())
	{
		//----------------------------------------------------
		// Ground
		//----------------------------------------------------

		if (auto ground =
			std::dynamic_pointer_cast<Ground>(obj))
		{
			NavMeshGenerator::NavMeshTile tile;

			const float halfSize =
				GRID_SIZE;

			const Math::Matrix worldMatrix =
				ground->GetMatrix();

			auto TransformTilePoint =
				[&](float x, float z)
				{
					return Math::Vector3::Transform(
						Math::Vector3(x, 0.0f, z),
						worldMatrix);
				};

			tile.v0 =
				TransformTilePoint(
					-halfSize,
					-halfSize);

			tile.v1 =
				TransformTilePoint(
					halfSize,
					-halfSize);

			tile.v2 =
				TransformTilePoint(
					-halfSize,
					halfSize);

			tile.v3 =
				TransformTilePoint(
					halfSize,
					halfSize);

			stageTiles.push_back(tile);

			continue;
		}

		//----------------------------------------------------
		// StaticMeshObject
		//----------------------------------------------------

		auto staticMeshObject =
			std::dynamic_pointer_cast<StaticMeshObject>(obj);

		if (!staticMeshObject ||
			staticMeshObject->GetCategory() !=
			KdGameObject::ObjectCategory::Prop)
		{
			continue;
		}

		if (staticMeshObject->IsGround())
		{
			continue;
		}

		//----------------------------------------------------
		// ModelData取得
		//----------------------------------------------------

		auto modelData =
			staticMeshObject->GetModelData();

		if (!modelData)
		{
			continue;
		}

		//----------------------------------------------------
		// World Matrix
		//----------------------------------------------------

		const Math::Matrix worldMatrix =
			staticMeshObject->GetMatrix();

		//----------------------------------------------------
		// Node取得
		//----------------------------------------------------

		const auto& nodes =
			modelData->GetOriginalNodes();

		const auto& collisionNodeIndices =
			modelData->GetCollisionMeshNodeIndices();

		//----------------------------------------------------
		// COL Nodeごとに処理
		//----------------------------------------------------

		for (int nodeIndex :
		collisionNodeIndices)
		{
			if (nodeIndex < 0 ||
				nodeIndex >=
				static_cast<int>(nodes.size()))
			{
				continue;
			}

			const auto& node =
				nodes[nodeIndex];

			if (!node.m_spMesh)
			{
				continue;
			}

			//------------------------------------------------
			// Local AABB
			//------------------------------------------------

			const auto& localAABB =
				node.m_spMesh->GetBoundingBox();

			//------------------------------------------------
			// World AABB
			//------------------------------------------------

			DirectX::BoundingBox worldAABB;

			Math::Matrix transform =
				node.m_worldTransform *
				worldMatrix;

			localAABB.Transform(
				worldAABB,
				transform);

			//------------------------------------------------
			// XZ Bounds
			//------------------------------------------------

			NavMeshGenerator::NavMeshObstacleBounds obstacle;

			obstacle.minX =
				worldAABB.Center.x -
				worldAABB.Extents.x;

			obstacle.maxX =
				worldAABB.Center.x +
				worldAABB.Extents.x;

			obstacle.minZ =
				worldAABB.Center.z -
				worldAABB.Extents.z;

			obstacle.maxZ =
				worldAABB.Center.z +
				worldAABB.Extents.z;

			obstacles.push_back(
				obstacle);
		}
	}

	//--------------------------------------------------------
	// Generate
	//--------------------------------------------------------
	for (const auto& tile : stageTiles)
	{
		Math::Vector3 a = tile.v1 - tile.v0;
		Math::Vector3 b = tile.v2 - tile.v0;

		Math::Vector3 normal =
			a.Cross(b);

		float area =
			normal.Length() * 0.5f;
	}

	m_navMeshGenerator.Generate(
		NavMeshManager::Instance(),
		stageTiles,
		obstacles);
}

void Editor::GenerateNavMeshVer3(float agentRadius)
{
	NavMeshGenerator generator;

	NavMeshGenerator::Input input;

	input.tileSize = GRID_SIZE;

	input.agentRadius = agentRadius;

	//============================================================
	// Walkable Tile
	//============================================================

	for (auto& obj :
		SceneManager::Instance().GetObjList())
	{
		if (auto ground =
			std::dynamic_pointer_cast<Ground>(obj))
		{
			NavMeshPolygonBuilder::Tile tile;

			tile.centerPos =
				Math::Vector2{
					ground->GetPos().x,
					ground->GetPos().z
			};

			input.tiles.push_back(
				tile);
		}
	}


	//============================================================
	// Obstacles
	//============================================================

	for (auto& obj :
		SceneManager::Instance().GetObjList())
	{
		if (auto staticMeshObject =
			std::dynamic_pointer_cast<StaticMeshObject>(obj))
		{
			if (!staticMeshObject ||
				staticMeshObject->GetCategory() !=
				KdGameObject::ObjectCategory::Prop)
			{
				continue;
			}

			//----------------------------------------------------
			// ModelData取得
			//----------------------------------------------------

			auto modelData =
				staticMeshObject->GetModelData();

			if (!modelData)
			{
				continue;
			}

			//----------------------------------------------------
			// World Matrix
			//----------------------------------------------------

			const Math::Matrix worldMatrix =
				staticMeshObject->GetMatrix();

			//----------------------------------------------------
			// Node取得
			//----------------------------------------------------

			const auto& nodes =
				modelData->GetOriginalNodes();

			const auto& collisionNodeIndices =
				modelData->GetCollisionMeshNodeIndices();

			//----------------------------------------------------
			// COL Nodeごとに処理
			//----------------------------------------------------

			for (int nodeIndex :
			collisionNodeIndices)
			{
				if (nodeIndex < 0 ||
					nodeIndex >=
					static_cast<int>(nodes.size()))
				{
					continue;
				}

				const auto& node =
					nodes[nodeIndex];

				if (!node.m_spMesh)
				{
					continue;
				}

				//------------------------------------------------
				// Local AABB
				//------------------------------------------------

				const auto& localAABB =
					node.m_spMesh->GetBoundingBox();

				//------------------------------------------------
				// World AABB
				//------------------------------------------------

				DirectX::BoundingBox worldAABB;

				Math::Matrix transform =
					node.m_worldTransform *
					worldMatrix;

				localAABB.Transform(
					worldAABB,
					transform);

				//------------------------------------------------
				// XZ Bounds
				//------------------------------------------------

				NavMeshPolygonBuilder::Obstacle obstacle;

				obstacle.minX =
					worldAABB.Center.x -
					worldAABB.Extents.x;

				obstacle.maxX =
					worldAABB.Center.x +
					worldAABB.Extents.x;

				obstacle.minZ =
					worldAABB.Center.z -
					worldAABB.Extents.z;

				obstacle.maxZ =
					worldAABB.Center.z +
					worldAABB.Extents.z;

				input.obstacles.push_back(
					obstacle);
			}
		}
	}

	//============================================================
	// Obstacles (InteractiveObject)
	//============================================================

	for (auto& obj :
		SceneManager::Instance().GetObjList())
	{
		auto interactive =
			std::dynamic_pointer_cast<InteractiveObject>(obj);

		if (!interactive ||
			!interactive->IsCollisionEnabled())
		{
			continue;
		}

		//----------------------------------------------------
		// ModelData取得
		//----------------------------------------------------

		auto modelData =
			interactive->GetModelData();

		if (!modelData)
		{
			continue;
		}

		const Math::Matrix worldMatrix =
			interactive->GetMatrix();

		const auto& nodes =
			modelData->GetOriginalNodes();

		const auto& collisionNodeIndices =
			modelData->GetCollisionMeshNodeIndices();

		//----------------------------------------------------
		// COL Nodeごとに処理
		//----------------------------------------------------

		for (int nodeIndex : collisionNodeIndices)
		{
			if (nodeIndex < 0 ||
				nodeIndex >= static_cast<int>(nodes.size()))
			{
				continue;
			}

			const auto& node = nodes[nodeIndex];

			if (!node.m_spMesh)
			{
				continue;
			}

			DirectX::BoundingBox worldAABB;

			node.m_spMesh->GetBoundingBox().Transform(
				worldAABB,
				node.m_worldTransform * worldMatrix);

			NavMeshPolygonBuilder::Obstacle obstacle;

			obstacle.minX = worldAABB.Center.x - worldAABB.Extents.x;
			obstacle.maxX = worldAABB.Center.x + worldAABB.Extents.x;
			obstacle.minZ = worldAABB.Center.z - worldAABB.Extents.z;
			obstacle.maxZ = worldAABB.Center.z + worldAABB.Extents.z;

			input.obstacles.push_back(obstacle);
		}
	}

	//============================================================
	// Polygon Boolean
	//============================================================

	const auto navMeshPolygons =
		generator.Generate(input);


	//============================================================
	// CDT
	//============================================================

	NavMeshCDT navMeshCDT;

	const auto triangles =
		navMeshCDT.Triangulate(
			navMeshPolygons);

	//============================================================
	// NavMeshManager
	//============================================================

	auto& manager =
		NavMeshManager::Instance();

	manager.Clear();

	for (const auto& triangle :
		triangles)
	{
		manager.CreatePolygon(
			triangle.ToVector3());
	}

	//============================================================
	// Connection
	//============================================================

	const auto& polygons =
		manager.GetPolygons();

	//--------------------------------------------------------
	// 全Polygonを比較
	//--------------------------------------------------------

	for (size_t i = 0;
		i < polygons.size();
		++i)
	{
		for (size_t j = i + 1;
			j < polygons.size();
			++j)
		{
			const auto& polygonA =
				polygons[i];

			const auto& polygonB =
				polygons[j];

			//------------------------------------------------
			// 辺を共有しているか
			//------------------------------------------------

			if (!polygonA->SharesEdgeWith(
				*polygonB))
			{
				continue;
			}

			//------------------------------------------------
			// 接続
			//------------------------------------------------

			manager.ConnectPolygons(
				polygonA->GetID(),
				polygonB->GetID());
		}
	}

}

void Editor::ClearNavMesh()
{
	NavMeshManager::Instance().Clear();

	ClearNavMeshPath();

	m_navigationEditorWindow.SetNavMeshEditMode(
		NavigationEditorWindow::NavMeshEditMode::None);
}

void Editor::ClearNavMeshPath()
{
	m_navMeshStartPolygon = nullptr;
	m_navMeshGoalPolygon = nullptr;

	m_navMeshPath.clear();
	m_navMeshPointPath.clear();
}

void Editor::SelectNavMeshPolygon(
	const Math::Vector3& pos)
{
	auto polygon =
		FindNavMeshPolygon(pos);

	if (!polygon)
	{
		return;
	}

	auto mode =
		m_navigationEditorWindow.GetNavMeshEditMode();

	//------------------------------------------------
	// Set Start
	//------------------------------------------------

	if (mode ==
		NavigationEditorWindow::NavMeshEditMode::SetStart)
	{
		m_navMeshStartPolygon = polygon;

		// 古い経路を消す
		m_navMeshPath.clear();
		m_navMeshPointPath.clear();

		return;
	}

	//------------------------------------------------
	// Set Goal
	//------------------------------------------------

	if (mode ==
		NavigationEditorWindow::NavMeshEditMode::SetGoal)
	{
		m_navMeshGoalPolygon = polygon;

		// 古い経路を消す
		m_navMeshPath.clear();
		m_navMeshPointPath.clear();

		return;
	}
}

void Editor::CalculateNavMeshPath()
{
	m_navMeshPath.clear();

	if (!m_navMeshStartPolygon ||
		!m_navMeshGoalPolygon)
	{
		return;
	}

	const int startID =
		m_navMeshStartPolygon->GetID();

	const int goalID =
		m_navMeshGoalPolygon->GetID();

	//------------------------------------------------
	// Same Polygon
	//------------------------------------------------

	if (startID == goalID)
	{
		m_navMeshPath.push_back(startID);
		return;
	}

	struct NodeRecord
	{
		int id = -1;
		float g = FLT_MAX;
		float f = FLT_MAX;
		int parent = -1;
	};

	std::unordered_map<int, NodeRecord> records;

	//------------------------------------------------
	// Record作成
	//------------------------------------------------

	for (const auto& polygon :
		NavMeshManager::Instance().GetPolygons())
	{
		if (!polygon)
		{
			continue;
		}

		NodeRecord record;

		record.id =
			polygon->GetID();

		records[record.id] =
			record;
	}

	//------------------------------------------------
	// Start / Goal
	//------------------------------------------------

	auto start =
		NavMeshManager::Instance().FindPolygon(startID);

	auto goal =
		NavMeshManager::Instance().FindPolygon(goalID);

	if (!start || !goal)
	{
		return;
	}

	records[startID].g = 0.0f;

	records[startID].f =
		(start->GetCenter() -
			goal->GetCenter()).Length();

	std::vector<int> openList;

	std::unordered_set<int> closedList;

	openList.push_back(startID);

	bool foundGoal = false;

	//------------------------------------------------
	// A*
	//------------------------------------------------

	while (!openList.empty())
	{
		//------------------------------------------------
		// F最小を検索
		//------------------------------------------------

		int currentID =
			openList.front();

		for (int id : openList)
		{
			if (records[id].f <
				records[currentID].f)
			{
				currentID = id;
			}
		}

		//------------------------------------------------
		// Goal
		//------------------------------------------------

		if (currentID == goalID)
		{
			foundGoal = true;
			break;
		}

		//------------------------------------------------
		// Openから削除
		//------------------------------------------------

		openList.erase(
			std::remove(
				openList.begin(),
				openList.end(),
				currentID),
			openList.end());

		closedList.insert(currentID);

		auto current =
			NavMeshManager::Instance().FindPolygon(
				currentID);

		if (!current)
		{
			continue;
		}

		//------------------------------------------------
		// Neighbor
		//------------------------------------------------

		for (int neighborID :
		current->GetConnections())
		{
			if (closedList.contains(
				neighborID))
			{
				continue;
			}

			auto neighbor =
				NavMeshManager::Instance().FindPolygon(
					neighborID);

			if (!neighbor)
			{
				continue;
			}

			float cost =
				(current->GetCenter() -
					neighbor->GetCenter()).Length();

			float newG =
				records[currentID].g +
				cost;

			if (newG <
				records[neighborID].g)
			{
				records[neighborID].parent =
					currentID;

				records[neighborID].g =
					newG;

				float h =
					(neighbor->GetCenter() -
						goal->GetCenter()).Length();

				records[neighborID].f =
					newG + h;

				if (std::find(
					openList.begin(),
					openList.end(),
					neighborID)
					== openList.end())
				{
					openList.push_back(
						neighborID);
				}
			}
		}
	}

	//------------------------------------------------
	// 到達不能
	//------------------------------------------------

	if (!foundGoal)
	{
		return;
	}

	//------------------------------------------------
	// Path復元
	//------------------------------------------------

	int currentID =
		goalID;

	while (currentID != -1)
	{
		m_navMeshPath.push_back(
			currentID);

		if (currentID == startID)
		{
			break;
		}

		currentID =
			records[currentID].parent;
	}

	//------------------------------------------------
	// Start -> Goal
	//------------------------------------------------

	std::reverse(
		m_navMeshPath.begin(),
		m_navMeshPath.end());
}

void Editor::CalculateNavMeshPointPath()
{
	m_navMeshPath.clear();
	m_navMeshPointPath.clear();

	if (!m_navMeshStartPolygon ||
		!m_navMeshGoalPolygon)
	{
		return;
	}

	//------------------------------------------------
	// A*
	//------------------------------------------------

	CalculateNavMeshPath();

	if (m_navMeshPath.empty())
	{
		return;
	}

	//------------------------------------------------
	// Funnel
	//------------------------------------------------

	const Math::Vector3 start =
		m_navMeshStartPolygon->GetCenter();

	const Math::Vector3 goal =
		m_navMeshGoalPolygon->GetCenter();

	m_navMeshPointPath =
		m_navMeshPathfinder.FindPathPoints(
			NavMeshManager::Instance(),
			start,
			goal);
}

bool Editor::IsPointInTriangleXZ(
	const Math::Vector3& p,
	const Math::Vector3& a,
	const Math::Vector3& b,
	const Math::Vector3& c)
{
	float cross1 =
		(b.x - a.x) * (p.z - a.z) -
		(b.z - a.z) * (p.x - a.x);

	float cross2 =
		(c.x - b.x) * (p.z - b.z) -
		(c.z - b.z) * (p.x - b.x);

	float cross3 =
		(a.x - c.x) * (p.z - c.z) -
		(a.z - c.z) * (p.x - c.x);

	constexpr float EPS = 0.001f;

	bool hasNegative =
		(cross1 < -EPS) ||
		(cross2 < -EPS) ||
		(cross3 < -EPS);

	bool hasPositive =
		(cross1 > EPS) ||
		(cross2 > EPS) ||
		(cross3 > EPS);

	return !(hasNegative && hasPositive);
}

std::shared_ptr<NavMeshPolygon>
Editor::FindNavMeshPolygon(
	const Math::Vector3& pos)
{
	for (const auto& polygon :
		NavMeshManager::Instance().GetPolygons())
	{
		if (!polygon)
		{
			continue;
		}

		const auto& v =
			polygon->GetVertices();

		if (IsPointInTriangleXZ(
			pos,
			v[0],
			v[1],
			v[2]))
		{
			return polygon;
		}
	}

	return nullptr;
}

void Editor::OnLocationClick(const Math::Vector3& pos)
{
	constexpr float SELECT_RADIUS = 1.0f;

	std::shared_ptr<LocationPoint> nearest = nullptr;

	float nearestDistanceSq = SELECT_RADIUS * SELECT_RADIUS;

	for (auto& point : m_locationPointManager.GetPoints())
	{
		if (!point)
		{
			continue;
		}

		Math::Vector3 diff = point->GetPos() - pos;

		float distSq = diff.LengthSquared();

		if (distSq <= nearestDistanceSq)
		{
			nearestDistanceSq = distSq;
			nearest = point;
		}
	}

	auto mode = m_locationEditorWindow.GetMode();

	//------------------------------------------------
	// Select
	//------------------------------------------------

	if (mode == LocationEditorWindow::Mode::Select)
	{
		if (nearest)
		{
			m_locationEditorWindow.SetSelectedPoint(nearest->GetID());
		}
		else
		{
			m_locationEditorWindow.SetSelectedPoint(-1);
		}

		return;
	}

	//------------------------------------------------
	// Create
	//------------------------------------------------

	if (mode == LocationEditorWindow::Mode::Create)
	{
		if (!nearest)
		{
			auto point =
				m_locationPointManager.CreatePoint(
					m_locationEditorWindow.GetCreateTag(),
					pos);

			m_locationEditorWindow.SetSelectedPoint(point->GetID());
		}

		return;
	}

	//------------------------------------------------
	// Erase
	//------------------------------------------------

	if (mode == LocationEditorWindow::Mode::Erase)
	{
		if (nearest)
		{
			m_locationPointManager.DeletePoint(nearest->GetID());

			m_locationEditorWindow.SetSelectedPoint(-1);
		}
	}
}

void Editor::DrawMenuBar()
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("Mode"))
		{
			bool objectMode = m_editMode == EditMode::Object;
			bool stageMode = m_editMode == EditMode::Stage;
			bool nodeMode = m_editMode == EditMode::Navigation;
			bool locationMode = m_editMode == EditMode::Location;

			if (ImGui::MenuItem("Object", nullptr, objectMode))
			{
				m_editMode = EditMode::Object;
			}

			if (ImGui::MenuItem("Stage", nullptr, stageMode))
			{
				m_editMode = EditMode::Stage;
			}

			if (ImGui::MenuItem("Navigation", nullptr, nodeMode))
			{
				m_editMode = EditMode::Navigation;
			}

			if (ImGui::MenuItem("Location", nullptr, locationMode))
			{
				m_editMode = EditMode::Location;
			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::BeginMenu("Open...", "Ctrl+O"))
			{
				DrawOpenSceneMenu();

				ImGui::EndMenu();
			}

			if (ImGui::MenuItem("Save", "Ctrl+S"))
			{
				SaveScene();
			}

			ImGui::Separator();

			ImGui::TextDisabled("Current:");
			ImGui::Text("%s", m_currentScenePath.c_str());

			ImGui::Separator();

			if (m_editMode == EditMode::Navigation)
			{
				if (ImGui::BeginMenu("Navigation"))
				{
					if (ImGui::MenuItem("Save Node"))
					{
						SaveNode();
					}

					if (ImGui::MenuItem("Load Node"))
					{
						LoadNode();
					}

					ImGui::Separator();

					if (ImGui::MenuItem("Save NavMesh"))
					{
						SaveNavMesh();
					}

					if (ImGui::MenuItem("Load NavMesh"))
					{
						LoadNavMesh();
					}

					ImGui::EndMenu();
				}
			}

			if (m_editMode == EditMode::Location)
			{
				if (ImGui::BeginMenu("Location"))
				{
					if (ImGui::MenuItem("Save"))
					{
						SaveLocation();
					}

					if (ImGui::MenuItem("Load"))
					{
						LoadLocation();
					}

					ImGui::EndMenu();
				}
			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Window"))
		{
			ImGui::MenuItem("Hierarchy", nullptr, &m_showHierarchy);
			ImGui::MenuItem("Inspector", nullptr, &m_showInspector);
			ImGui::MenuItem("Scene", nullptr, &m_showScene);
			ImGui::MenuItem("Asset Browser", nullptr, &m_showAssetBrowser);

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("GameObject"))
		{
			if (ImGui::MenuItem("Duplicate", "Ctrl+D"))
			{
				if (m_selectObject)
				{
					auto duplicatedObject = SceneManager::Instance().DuplicateObject(m_selectObject);

					if (!duplicatedObject)
					{
						return;
					}

					m_selectObject = duplicatedObject;
				}
			}

			if (ImGui::MenuItem("Delete", "Delete"))
			{
				if (m_selectObject)
				{
					m_selectObject->Destroy();
					m_selectObject = nullptr;
				}
			}

			ImGui::Separator();

			if (ImGui::MenuItem("Static Mesh"))
			{
				auto obj = SceneManager::Instance().CreateObject("StaticMeshObject");

				if (obj)
				{
					obj->SetName("StaticMesh");

					m_selectObject = obj;
				}
			}

			if (ImGui::MenuItem("Item"))
			{
				auto obj = SceneManager::Instance().CreateObject("Item");

				if (obj)
				{
					obj->SetName("Item");

					m_selectObject = obj;
				}
			}

			if (ImGui::MenuItem("Player"))
			{
				static int playerCount = 0;

				auto player = SceneManager::Instance().CreateObject("Player");

				if (player)
				{
					player->SetName("Player_" + std::to_string(playerCount++));

					m_selectObject = player;
				}
			}

			if (ImGui::MenuItem("Neighbor"))
			{
				static int neighborCount = 0;

				auto neighbor = SceneManager::Instance().CreateObject("Neighbor");

				if (neighbor)
				{
					neighbor->SetName("Neighbor_" + std::to_string(neighborCount++));

					m_selectObject = neighbor;
				}
			}

			if (ImGui::MenuItem("Police"))
			{
				static int policeCount = 0;

				auto police = SceneManager::Instance().CreateObject("Police");

				if (police)
				{
					police->SetName("Police_" + std::to_string(policeCount++));

					m_selectObject = police;
				}
			}

			ImGui::Separator();

			if (ImGui::MenuItem("Tree"))
			{
				static int treeCount = 0;

				auto obj = SceneManager::Instance().CreateObject("Tree");

				if (obj)
				{
					obj->SetName("Tree_" + std::to_string(treeCount++));

					obj->SetScale(10.0f);

					m_selectObject = obj;
				}
			}

			ImGui::Separator();

			if (ImGui::BeginMenu("Interctive Object"))
			{
				if (ImGui::MenuItem("Flower"))
				{
					auto obj = SceneManager::Instance().CreateObject("Flower");

					if (obj)
					{
						obj->SetName("Flower");

						m_selectObject = obj;
					}
				}

				if (ImGui::MenuItem("Hose"))
				{
					auto obj = SceneManager::Instance().CreateObject("Hose");

					if (obj)
					{
						obj->SetName("Hose");

						m_selectObject = obj;
					}
				}

				if (ImGui::MenuItem("PaintCan"))
				{
					auto obj = SceneManager::Instance().CreateObject("PaintCan");

					if (obj)
					{
						obj->SetName("PaintCan");

						m_selectObject = obj;
					}
				}

				if (ImGui::MenuItem("WoodenFence"))
				{
					auto obj = SceneManager::Instance().CreateObject("WoodenFence");

					if (obj)
					{
						obj->SetName("WoodenFence");

						m_selectObject = obj;
					}
				}

				if (ImGui::MenuItem("Faucet"))
				{
					auto obj = SceneManager::Instance().CreateObject("Faucet");

					if (obj)
					{
						obj->SetName("Faucet");

						m_selectObject = obj;
					}
				}

				if (ImGui::MenuItem("Car"))
				{
					auto obj = SceneManager::Instance().CreateObject("Car");

					if (obj)
					{
						obj->SetName("Car");

						m_selectObject = obj;
					}
				}

				if (ImGui::MenuItem("Window"))
				{
					auto obj = SceneManager::Instance().CreateObject("Window");

					if (obj)
					{
						obj->SetName("Window");

						m_selectObject = obj;
					}
				}

				ImGui::EndMenu();
			}

			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}
}

void Editor::DrawOpenSceneMenu()
{
	for (auto& file : std::filesystem::directory_iterator("Asset/Scenes"))
	{
		if (file.path().extension() != ".json")
		{
			continue;
		}

		std::string path = file.path().string();

		if (ImGui::MenuItem(file.path().filename().string().c_str()))
		{
			OpenScene(path);
		}
	}
}

void Editor::DrawGridLevel(
	KdDebugWireFrame& wire,
	const Math::Vector3& camPos,
	float distance,
	float screenHeight,
	float gridSize)
{
	//-------------------------------------------------
	// 画面上でのグリッド間隔(px)
	//-------------------------------------------------

	float pixelSize = gridSize / distance * screenHeight;

	//-------------------------------------------------
	// 小さすぎるなら描かない
	//-------------------------------------------------

	constexpr float kMinPixelSize = 10.0f;

	if (pixelSize < kMinPixelSize)
		return;

	//-------------------------------------------------
	// Alpha
	//-------------------------------------------------

	float alpha = std::clamp(
		(pixelSize - kMinPixelSize) / 40.0f,
		0.0f,
		1.0f);

	//-------------------------------------------------
	// Infinite Grid
	//-------------------------------------------------

	float centerX =
		std::floor(camPos.x / gridSize) * gridSize;

	float centerZ =
		std::floor(camPos.z / gridSize) * gridSize;

	constexpr int halfLine = 150;

	float minX = centerX - halfLine * gridSize;
	float maxX = centerX + halfLine * gridSize;

	float minZ = centerZ - halfLine * gridSize;
	float maxZ = centerZ + halfLine * gridSize;

	//-------------------------------------------------
	// Draw
	//-------------------------------------------------

	for (int i = -halfLine; i <= halfLine; i++)
	{
		float x = centerX + i * gridSize;
		float z = centerZ + i * gridSize;

		//-----------------------------------------
		// X方向
		//-----------------------------------------

		Math::Color xColor;

		if (fabs(x) < 0.001f)
		{
			xColor = Math::Color(1, 0, 0, 1);
		}
		else
		{
			xColor = Math::Color(
				0.45f,
				0.45f,
				0.45f,
				alpha);
		}

		wire.AddDebugLine(
			{ x,0,minZ },
			{ x,0,maxZ },
			xColor);

		//-----------------------------------------
		// Z方向
		//-----------------------------------------

		Math::Color zColor;

		if (fabs(z) < 0.001f)
		{
			zColor = Math::Color(0, 0.5f, 1, 1);
		}
		else
		{
			zColor = Math::Color(
				0.45f,
				0.45f,
				0.45f,
				alpha);
		}

		wire.AddDebugLine(
			{ minX,0,z },
			{ maxX,0,z },
			zColor);
	}
}

void Editor::SetOpen(bool open)
{
	m_isOpen = open;

	if (open)
	{
		m_returnCamera = CameraManager::Instance().GetCamera();
		CameraManager::Instance().SetCamera(m_editorCamera);
	}
	else
	{
		CameraManager::Instance().SetCamera(m_returnCamera.lock());
	}
}