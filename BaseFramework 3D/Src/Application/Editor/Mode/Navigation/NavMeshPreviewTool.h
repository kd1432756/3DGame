#pragma once

#include "../../Windows/NavigationEditorWindow.h"
#include "../../../System/Navigation/NavMesh/NavMeshManager.h"
#include "../../../System/Navigation/NavMesh/NavMeshPathfinder.h"

class NavMeshPreviewTool
{
public:
	void Generate(float agentRadius);
	void Clear();

	void OnClick(const Math::Vector3& pos, NavigationEditorWindow::NavMeshEditMode mode);

	void CalculatePath();
	void ClearPath();

	void DrawDebug(bool showMesh);

private:
	std::shared_ptr<NavMeshPolygon> FindPolygon(const Math::Vector3& pos) const;
	void DrawMesh();

	NavMeshPathfinder m_pathfinder;

	std::shared_ptr<NavMeshPolygon> m_start;
	std::shared_ptr<NavMeshPolygon> m_goal;

	std::vector<Math::Vector3> m_pointPath;

	KdDebugWireFrame m_wire;

	static inline const Math::Color kStartColor{ 0.1f,  0.4f,  1.0f,  1.0f };
	static inline const Math::Color kGoalColor{ 1.0f,  0.15f, 0.15f, 1.0f };
	static inline const Math::Color kActualPathColor{ 1.0f,  0.1f,  0.8f,  1.0f };
	static inline const Math::Color kWaypointColor{ 0.5f,  0.1f,  0.9f,  1.0f };
};