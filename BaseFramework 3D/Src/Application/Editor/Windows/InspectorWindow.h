#pragma once

class StaticMeshObject;
class InteractiveObject;
class Item;
class Neighbor;
class CharacterBase;

class InspectorWindow
{
public:
	void Draw(std::shared_ptr<KdGameObject> object);

	void SetLayout(
		const ImVec2& pos,
		const ImVec2& size)
	{
		m_windowPos = pos;
		m_windowSize = size;
	}

private:

	void DrawTransform(std::shared_ptr<KdGameObject>& obj) {}

	void DrawStaticMesh(std::shared_ptr<StaticMeshObject> obj);
	void DrawItem(std::shared_ptr<Item> obj);
	void DrawNeighbor(std::shared_ptr<Neighbor> obj);
	void DrawCharacter(std::shared_ptr<CharacterBase> obj);

	ImVec2 m_windowPos = { 1030, 20 };
	ImVec2 m_windowSize = { 250, 500 };
};