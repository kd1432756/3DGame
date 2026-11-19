#pragma once

class EditorCamera;
class EditorContext;
class IEditorMode;
class KdGameObject;

class SceneWindow
{
public:
	void Draw(EditorContext& ctx, IEditorMode& mode);

	Math::Vector3 GetDropPosition(std::shared_ptr<EditorCamera>& camera);

	void PickObject(
		std::shared_ptr<EditorCamera>& editorCamera,
		std::shared_ptr<KdGameObject>& selectObject);

	void SetLayout(const ImVec2& pos, const ImVec2& size)
	{
		m_windowPos = pos;
		m_windowSize = size;
	}

private:

	ImVec2 m_windowPos = { 250,20 };
	ImVec2 m_windowSize = { 780,500 };

	ImVec2 m_imagePos;
	ImVec2 m_imageSize;

	ImGuizmo::OPERATION m_operation = ImGuizmo::TRANSLATE;

	void DrawToolbar();
	void DrawSceneImage(std::shared_ptr<EditorCamera>& editorCamera);
	bool ImageClicked();
	void DrawGuizmo(
		std::shared_ptr<EditorCamera>& editorCamera,
		std::shared_ptr<KdGameObject>& selectObject);
};