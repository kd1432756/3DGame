#pragma once

class EditorCamera;

class EditorGrid
{
public:
	void Draw(EditorCamera& camera);
private:
	void DrawLevel(const Math::Vector3& camPos, float distance, float gridSize);

	KdDebugWireFrame m_wire;
};