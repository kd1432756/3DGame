#pragma once
#include "../EditorContext.h"
#include "../EditorIO.h"

class IEditorMode
{
public:
	virtual ~IEditorMode() = default;

	virtual void OnEnter(EditorContext&) {}
	virtual void OnExit(EditorContext&) {}

	// ウィンドウの配置
	virtual void ApplyLayout(EditorContext&) = 0;

	// ショートカットキー
	virtual void Update(EditorContext&, EditorIO&) = 0;

	// ウィンドウ
	virtual void DrawWindows(EditorContext&) = 0;

	// ワイヤー・プレビュー
	virtual void DrawDebug(EditorContext&) {}

	// Sceneウィンドウクリック
	virtual void OnSceneClick(EditorContext&, const Math::Vector3&) {}

	// ファイル関係
	virtual void DrawFileMenu(EditorIO&) {}

	// 選択リセット用
	virtual void OnDataReloaded(EditorContext&) {}

	// メニューバー
	virtual void DrawMenuBar(EditorContext&) {}

	// ImGuiGizmo
	virtual bool ShowGizmo() const { return false; }
};