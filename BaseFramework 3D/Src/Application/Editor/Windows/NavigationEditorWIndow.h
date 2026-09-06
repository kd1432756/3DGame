#pragma once

#include "NodeEditorWindow.h"

class NavigationEditorWindow
{
public:

	enum class NavigationMode
	{
		Node,
		NavMesh
	};

	enum class NavMeshEditMode
	{
		None,
		SetStart,
		SetGoal
	};

	void Draw(NodeEditorWindow& nodeEditorWindow);

	void SetLayout(
		const ImVec2& pos,
		const ImVec2& size)
	{
		m_windowPos = pos;
		m_windowSize = size;
	}

	NavigationMode GetMode() const
	{
		return m_mode;
	}

	void SetMode(NavigationMode mode)
	{
		m_mode = mode;

		if (m_mode == NavigationMode::Node)
		{
			m_navMeshEditMode =
				NavMeshEditMode::None;
		}
	}

	bool IsShowNavMesh() const
	{
		return m_showNavMesh;
	}

	NavMeshEditMode GetNavMeshEditMode() const
	{
		return m_navMeshEditMode;
	}

	void SetNavMeshEditMode(
		NavMeshEditMode mode)
	{
		m_navMeshEditMode = mode;
	}

	void ClearNavMeshEditMode()
	{
		m_navMeshEditMode =
			NavMeshEditMode::None;
	}

private:

	void DrawToolbar();
	void DrawNavigationSettings();
	void DrawNavigationActions();

	ImVec2 m_windowPos = { 1030, 20 };
	ImVec2 m_windowSize = { 250, 700 };

	NavigationMode m_mode = NavigationMode::Node;

	bool m_showNavMesh = false;

	NavMeshEditMode m_navMeshEditMode =
		NavMeshEditMode::None;

	float m_navMeshAgentRadius = 0.2f;
};
