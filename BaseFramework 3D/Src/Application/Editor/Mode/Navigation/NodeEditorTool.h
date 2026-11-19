#pragma once

#include "../../Windows/NodeEditorWindow.h"
#include "../../../System/Node/AStarNode/AStarNodeManager.h"

class EditorContext;

class NodeEditorTool
{
public:

	NodeEditorWindow& GetWindow() { return m_window; }

	void HandleWindowEvents();

	void Update();
	void OnClick(const Math::Vector3&);
	void DrawDebug(EditorContext&);
	void ResetConnection() { m_connectionStart = nullptr; }
	void OnLoaded();

private:
	std::shared_ptr<AStarNode> FindNearest(const Math::Vector3& pos) const;

	void Select(const std::shared_ptr<AStarNode>& node);
	void EraseNode(const std::shared_ptr<AStarNode>& node);
	void ConnectClick(const std::shared_ptr<AStarNode>& nearest, bool connect);

	NodeEditorWindow m_window;

	std::shared_ptr<AStarNode> m_selected;
	std::shared_ptr<AStarNode> m_connectionStart;

	KdDebugWireFrame m_wire;

	static constexpr float kSelectRadius = 1.0f;
	static constexpr float kNodeHeight = 0.5f;
};