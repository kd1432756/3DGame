#pragma once
#include "IEditorMode.h"
#include "../Windows/LocationEditorWindow.h"

class LocationMode : public IEditorMode
{
public:
	void ApplyLayout(EditorContext&) override;
	void Update(EditorContext&, EditorIO&) override;
	void DrawWindows(EditorContext&) override;
	void DrawDebug(EditorContext&) override;
	void OnSceneClick(EditorContext&, const Math::Vector3& pos) override;
	void DrawFileMenu(EditorIO&) override;
	void OnDataReloaded(EditorContext&) override;

private:
	LocationEditorWindow m_window;
	KdDebugWireFrame m_wire;
};