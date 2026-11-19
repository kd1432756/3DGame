#pragma once

#include "../GameObject/Camera/EditorCamera/EditorCamera.h"
#include "Windows/SceneWindow.h"

class EditorContext
{
public:
	std::shared_ptr<EditorCamera> camera;
	SceneWindow sceneWindow;
	std::shared_ptr<KdGameObject> selectObject;

	Math::Vector3 GetCursorWorldPos()
	{
		return sceneWindow.GetDropPosition(camera);
	}

	void ClearSelection() { selectObject = nullptr; }
};