#pragma once
#include "../IEditorMode.h"
#include "../../Windows/NavigationEditorWindow.h"
#include "NodeEditorTool.h"
#include "NavMeshPreviewTool.h"

class NavigationMode : public IEditorMode
{
public:
	void OnExit(EditorContext&) override;
	void ApplyLayout(EditorContext&) override;
	void Update(EditorContext&, EditorIO&) override;
	void DrawWindows(EditorContext&) override;
	void DrawDebug(EditorContext&) override;
	void OnSceneClick(EditorContext&, const Math::Vector3& pos) override;
	void DrawFileMenu(EditorIO&) override;
	void OnDataReloaded(EditorContext&) override;

private:
	void HandleRequest();
	bool IsNodeMode() const
	{
		return m_window.GetMode() == NavigationEditorWindow::NavigationMode::Node;
	}

	NavigationEditorWindow m_window;
	NodeEditorTool         m_node;
	NavMeshPreviewTool     m_navMesh;
};