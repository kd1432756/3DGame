#pragma once

class HierarchyWindow
{
public:
	void Draw(std::shared_ptr<KdGameObject>& selectObject);

	void SetLayout(const ImVec2& pos, const ImVec2& size)
	{
		m_windowPos = pos;
		m_windowSize = size;
	}

private:
	bool IsNameUsed(const std::string& name, std::shared_ptr<KdGameObject> ignoreObj);

	void DrawObject(
		const std::shared_ptr<KdGameObject>& obj,
		std::shared_ptr<KdGameObject>& selectObject);

	void DrawCategory(
		const char* title,
		KdGameObject::ObjectCategory category,
		std::shared_ptr<KdGameObject>& selectObject);

	std::shared_ptr<KdGameObject> m_renameObject = nullptr;
	std::string m_renameBuffer;
	bool m_startRename = false;

	ImVec2 m_windowPos = { 0, 20 };
	ImVec2 m_windowSize = { 250,700 };
};