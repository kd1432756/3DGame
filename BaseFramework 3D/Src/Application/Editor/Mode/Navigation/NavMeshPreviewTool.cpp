#include "NavMeshPreviewTool.h"

#include "../../../Scene/SceneManager.h"
#include "../../../System/Stage/StageConst.h"
#include "../../../System/Navigation/NavMesh/NavMeshBuilder.h"

void NavMeshPreviewTool::Generate(float agentRadius)
{
	ClearPath();

	// パラメータセット
	NavMeshBuilder::Params params;
	params.tileSize = StageConst::GRID_SIZE;
	params.agentRadius = agentRadius;

	NavMeshBuilder::Build(
		SceneManager::Instance().GetObjList(),
		NavMeshManager::Instance(),
		params);
}

void NavMeshPreviewTool::Clear()
{
	NavMeshManager::Instance().Clear();
	ClearPath();
}

void NavMeshPreviewTool::ClearPath()
{
	m_start = nullptr;
	m_goal = nullptr;
	m_pointPath.clear();
}

std::shared_ptr<NavMeshPolygon> NavMeshPreviewTool::FindPolygon(const Math::Vector3& pos) const
{
	for (const auto& polygon : NavMeshManager::Instance().GetPolygons())
	{
		if (!polygon) continue;

		// posがポリゴン内か
		if (polygon->ContainsPoint(pos))
		{
			return polygon;
		}
	}

	return nullptr;
}

void NavMeshPreviewTool::OnClick(const Math::Vector3& pos, NavigationEditorWindow::NavMeshEditMode mode)
{
	auto polygon = FindPolygon(pos);

	if (!polygon) return;

	// 経路探索の位置決め
	switch (mode)
	{
	case NavigationEditorWindow::NavMeshEditMode::SetStart:
		m_start = polygon;
		m_pointPath.clear();
		break;

	case NavigationEditorWindow::NavMeshEditMode::SetGoal:
		m_goal = polygon;
		m_pointPath.clear();
		break;

	default:
		break;
	}
}

void NavMeshPreviewTool::CalculatePath()
{
	m_pointPath.clear();

	if (!m_start || !m_goal) return;

	m_pointPath = m_pathfinder.FindPathPoints(
		NavMeshManager::Instance(),
		m_start->GetCenter(),
		m_goal->GetCenter());
}

void NavMeshPreviewTool::DrawMesh()
{
	auto& manager = NavMeshManager::Instance();

	for (const auto& polygon : manager.GetPolygons())
	{
		if (!polygon) continue;

		const auto& v = polygon->GetVertices();

		// NavMeshの三角形
		m_wire.AddDebugLine(v[0], v[1], kGreenColor);
		m_wire.AddDebugLine(v[1], v[2], kGreenColor);
		m_wire.AddDebugLine(v[2], v[0], kGreenColor);

		// 隣接していたら線でつなぐ
		for (int connectionID : polygon->GetConnections())
		{
			if (polygon->GetID() >= connectionID) continue;

			auto connection = manager.FindPolygon(connectionID);

			if (!connection) continue;

			m_wire.AddDebugLine(
				polygon->GetCenter(),
				connection->GetCenter(),
				kYellowColor);
		}
	}
}

void NavMeshPreviewTool::DrawDebug(bool showMesh)
{
	if (showMesh)
	{
		DrawMesh();
	}

	if (m_start)
	{
		m_wire.AddDebugSphere(m_start->GetCenter(), 0.3f, kStartColor);
	}

	if (m_goal)
	{
		m_wire.AddDebugSphere(m_goal->GetCenter(), 0.3f, kGoalColor);
	}

	// 経路
	for (size_t i = 1; i < m_pointPath.size(); ++i)
	{
		m_wire.AddDebugLine(m_pointPath[i - 1], m_pointPath[i], kActualPathColor);
	}

	// 曲がる角
	for (size_t i = 1; i + 1 < m_pointPath.size(); ++i)
	{
		m_wire.AddDebugSphere(m_pointPath[i], 0.1f, kWaypointColor);
	}

	m_wire.Draw();
}