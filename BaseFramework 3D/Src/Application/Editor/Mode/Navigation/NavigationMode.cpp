#include "NavigationMode.h"

void NavigationMode::OnExit(EditorContext&)
{
	m_node.ResetConnection();
	m_window.ClearNavMeshEditMode();
}

void NavigationMode::ApplyLayout(EditorContext& context)
{
	context.sceneWindow.SetLayout({ 0, 20 }, { 1030, 700 });
	m_window.SetLayout({ 1030, 20 }, { 250, 700 });
}

void NavigationMode::Update(EditorContext&, EditorIO&)
{
	if (IsNodeMode())
	{
		m_node.Update();
	}
}

void NavigationMode::DrawWindows(EditorContext&)
{
	m_window.Draw(m_node.GetWindow());

	// ウィンドウがわでリクエストした処理を行う
	m_node.HandleWindowEvents();
	HandleRequest();
}

void NavigationMode::DrawDebug(EditorContext& context)
{
	if (IsNodeMode())
	{
		m_node.DrawDebug(context);
	}
	else
	{
		m_navMesh.DrawDebug(m_window.IsShowNavMesh());
	}
}

void NavigationMode::OnSceneClick(EditorContext&, const Math::Vector3 & pos)
{
	if (IsNodeMode())
	{
		m_node.OnClick(pos);
	}
	else
	{
		m_navMesh.OnClick(pos, m_window.GetNavMeshEditMode());
	}
}

void NavigationMode::DrawFileMenu(EditorIO& io)
{
	if (!ImGui::BeginMenu("Navigation")) return;

	if (ImGui::MenuItem("Save Node")) io.SaveNodes();

	if (ImGui::MenuItem("Load Node") && io.LoadNodes())
	{
		m_node.OnLoaded();
	}

	ImGui::Separator();

	if (ImGui::MenuItem("Save NavMesh")) io.SaveNavMesh();

	if (ImGui::MenuItem("Load NavMesh") && io.LoadNavMesh())
	{
		m_navMesh.ClearPath();
	}

	ImGui::EndMenu();
}

void NavigationMode::OnDataReloaded(EditorContext&)
{
	m_node.OnLoaded();
	m_navMesh.ClearPath();
}

void NavigationMode::HandleRequest()
{
	using Request = NavigationEditorWindow::Request;

	switch (m_window.ConsumeRequest())
	{
	// 生成
	case Request::Generate:
		m_navMesh.Generate(m_window.GetAgentRadius(), m_window.GetObstacleMaxHeight());
		break;

	// リセット
	case Request::Clear:
		m_navMesh.Clear();
		m_window.ClearNavMeshEditMode();
		break;

	// 経路探索
	case Request::FindPath:
		m_navMesh.CalculatePath();
		break;

	// 経路リセット
	case Request::ClearPath:
		m_navMesh.ClearPath();
		m_window.ClearNavMeshEditMode();
		break;

	case Request::None:
		break;
	}
}
