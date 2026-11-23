#include "NavigationEditorWindow.h"

#include "../Editor.h"

void NavigationEditorWindow::Draw(NodeEditorWindow& nodeEditorWindow)
{
	ImGui::SetNextWindowPos(m_windowPos, ImGuiCond_Always);

	ImGui::SetNextWindowSize(m_windowSize, ImGuiCond_Always);

	ImGui::Begin("Navigation Editor", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

	DrawToolbar();

	ImGui::Separator();

	if (m_mode == NavigationMode::Node)
	{
		nodeEditorWindow.DrawContents();
	}
	else
	{
		DrawNavigationSettings();
		DrawNavigationActions();
	}

	ImGui::End();
}

void NavigationEditorWindow::DrawToolbar()
{
	ImGui::Text("Mode");

	if (ImGui::RadioButton("Node", m_mode == NavigationMode::Node))
	{
		SetMode(NavigationMode::Node);
	}

	ImGui::SameLine();

	if (ImGui::RadioButton("NavMesh", m_mode == NavigationMode::NavMesh))
	{
		SetMode(NavigationMode::NavMesh);
	}
}

void NavigationEditorWindow::DrawNavigationSettings()
{
	ImGui::SeparatorText("NavMesh Settings");

	ImGui::Checkbox("Show NavMesh", &m_showNavMesh);

	ImGui::SliderFloat(
		"Agent Radius",
		&m_navMeshAgentRadius,
		0.0f,
		1.0f,
		"%.2f");

	ImGui::SliderFloat(
		"Obstacle Max Height",
		&m_navMeshObstacleMaxHeight,
		0.0f,
		10.0f,
		"%.2f");
}

void NavigationEditorWindow::DrawNavigationActions()
{
	ImGui::SeparatorText("NavMesh");

	if (ImGui::Button("Generate NavMesh", { -1, 0 }))
	{
		m_request = Request::Generate;
	}

	if (ImGui::Button("Clear NavMesh", { -1, 0 }))
	{
		m_request = Request::Clear;
	}

	ImGui::SeparatorText("Preview");

	if (ImGui::RadioButton("Set Start", m_navMeshEditMode == NavMeshEditMode::SetStart))
	{
		m_navMeshEditMode = NavMeshEditMode::SetStart;
	}

	if (ImGui::RadioButton("Set Goal", m_navMeshEditMode == NavMeshEditMode::SetGoal))
	{
		m_navMeshEditMode = NavMeshEditMode::SetGoal;
	}

	ImGui::TextDisabled("Click a NavMesh polygon in Scene");

	if (ImGui::Button("Find Path", { -1, 0 }))
	{
		m_request = Request::FindPath;
	}

	if (ImGui::Button("Clear Path", { -1, 0 }))
	{
		m_request = Request::ClearPath;
	}
}