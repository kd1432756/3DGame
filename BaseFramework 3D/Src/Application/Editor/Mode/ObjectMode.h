#pragma once
#include "IEditorMode.h"
#include "../Windows/HierarchyWindow.h"
#include "../Windows/InspectorWindow.h"
#include "../Windows/AssetBrowserWindow.h"

class ObjectMode : public IEditorMode
{
public:
	void ApplyLayout(EditorContext&) override;
	void Update(EditorContext&, EditorIO&) override;
	void DrawWindows(EditorContext&) override;
	void OnSceneClick(EditorContext&, const Math::Vector3&) override;
	void DrawMenuBar(EditorContext&) override;
	bool ShowGizmo() const override { return true; }

private:
	// 生成時の初期設定
	struct CreateEntry
	{
		const char* label;	// 名前
		const char* type;	// 種類
		float scale;	// 大きさ（変えない場合は0）
	};

	void DrawCreateItems(EditorContext&, const CreateEntry*, int count);

	static void DuplicateSelected(EditorContext&);
	static void DeleteSelected(EditorContext&);
	static void CreateAndSelect(EditorContext&, const CreateEntry&);

	HierarchyWindow m_hierarchy;
	InspectorWindow m_inspector;
	AssetBrowserWindow m_assetBrowser;
};