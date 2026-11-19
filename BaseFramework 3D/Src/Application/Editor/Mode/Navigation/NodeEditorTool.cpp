#include "NodeEditorTool.h"

#include "../../EditorContext.h"

void NodeEditorTool::HandleWindowEvents()
{
	// モード切替でノード選択解除
	if (m_window.ConsumeModeChanged())
	{
		ResetConnection();
	}

	const int clickedID = m_window.ConsumeClickedNodeID();

	if (clickedID >= 0)
	{
		Select(AStarNodeManager::Instance().FindNode(clickedID));
	}

	auto& manager = AStarNodeManager::Instance();

	if (m_selected && !manager.FindNode(m_selected->GetID()))
	{
		m_selected = nullptr;
	}

	if (m_connectionStart && !manager.FindNode(m_connectionStart->GetID()))
	{
		m_connectionStart = nullptr;
	}
}

void NodeEditorTool::Update()
{
	// deleteで消す
	if (m_selected && ImGui::IsKeyPressed(ImGuiKey_Delete))
	{
		EraseNode(m_selected);
	}
}

void NodeEditorTool::OnClick(const Math::Vector3 & pos)
{
	const auto nearest = FindNearest(pos);

	switch (m_window.GetMode())
	{
	// 選択
	case NodeEditorWindow::Mode::Select:
		Select(nearest);
		break;

	// 生成
	case NodeEditorWindow::Mode::Create:
		if (!nearest)
		{
			Math::Vector3 nodePos = pos;
			nodePos.y = kNodeHeight;

			if (auto node = AStarNodeManager::Instance().CreateNode(nodePos))
			{
				Select(node);
			}
		}
		break;

	// 削除
	case NodeEditorWindow::Mode::Erase:
		if (nearest)
		{
			EraseNode(nearest);
		}
		break;

	// 接続
	case NodeEditorWindow::Mode::Connect:
		ConnectClick(nearest, true);
		break;

	// 接続解除
	case NodeEditorWindow::Mode::Disconnect:
		ConnectClick(nearest, false);
		break;
	}
}

void NodeEditorTool::DrawDebug(EditorContext & context)
{
	auto& manager = AStarNodeManager::Instance();

	// 接続線
	for (auto& node : manager.GetNodes())
	{
		if (!node) continue;

		for (int connectionID : node->GetConnections())
		{
			auto to = manager.FindNode(connectionID);

			if (!to) continue;

			m_wire.AddDebugLine(node->GetPos(), to->GetPos(), kYellowColor);
		}
	}

	// ノード
	for (auto& node : manager.GetNodes())
	{
		if (!node) continue;

		Math::Color color = kGreenColor;

		if (node == m_selected) color = kYellowColor;
		if (node == m_connectionStart) color = kRedColor;

		m_wire.AddDebugSphere(node->GetPos(), 0.1f, color);
	}

	m_wire.Draw();

	// ID
	const Math::Vector3 camPos = context.camera->GetPos();

	for (auto& node : manager.GetNodes())
	{
		if (!node) continue;

		Math::Vector3 worldPos = node->GetPos();

		const float distance = (worldPos - camPos).Length();

		worldPos.y += std::clamp(distance * 0.02f, 0.3f, 2.0f);

		Math::Vector3 screenPos = Math::Vector3::Zero;

		if (!context.camera->ConvertWorldToScreenDetail(worldPos, screenPos)) continue;

		char text[32];
		sprintf_s(text, "Node %d", node->GetID());

		KdShaderManager::Instance().m_spriteShader.DrawFont(
			{ screenPos.x, screenPos.y },
			&kWhiteColor,
			{ 0.5f, 0.5f },
			0.0f,
			FontID::Future16,
			text);
	}
}

void NodeEditorTool::OnLoaded()
{
	m_selected = nullptr;
	m_connectionStart = nullptr;
	m_window.SetSelectedNode(-1);
}

std::shared_ptr<AStarNode> NodeEditorTool::FindNearest(const Math::Vector3 & pos) const
{
	std::shared_ptr<AStarNode> nearest;
	float nearestSq = kSelectRadius * kSelectRadius;

	// 近くのノードを探す
	for (auto& node : AStarNodeManager::Instance().GetNodes())
	{
		if (!node) continue;

		const float distSq = (node->GetPos() - pos).LengthSquared();

		if (distSq <= nearestSq)
		{
			nearestSq = distSq;
			nearest = node;
		}
	}

	return nearest;
}

void NodeEditorTool::Select(const std::shared_ptr<AStarNode>& node)
{
	m_selected = node;
	m_window.SetSelectedNode(node ? node->GetID() : -1);
}

void NodeEditorTool::EraseNode(const std::shared_ptr<AStarNode>&node)
{
	AStarNodeManager::Instance().DeleteNode(node->GetID());

	if (m_selected == node) m_selected = nullptr;
	if (m_connectionStart == node) m_connectionStart = nullptr;

	m_window.SetSelectedNode(-1);
}

void NodeEditorTool::ConnectClick(const std::shared_ptr<AStarNode>&nearest, bool connect)
{
	if (!nearest)
	{
		m_connectionStart = nullptr;
		return;
	}

	// 1個目
	if (!m_connectionStart)
	{
		m_connectionStart = nearest;
		Select(nearest);
		return;
	}

	// 同じノードを選択で解除
	if (m_connectionStart == nearest)
	{
		m_connectionStart = nullptr;
		return;
	}

	// 2個目
	auto& manager = AStarNodeManager::Instance();

	// 接続
	if (connect)
	{
		manager.ConnectNodes(m_connectionStart->GetID(), nearest->GetID());
	}
	else
	{
		manager.DisconnectNodes(m_connectionStart->GetID(), nearest->GetID());
	}

	m_connectionStart = nullptr;
	Select(nearest);
}
