#include "NodeEditorWindow.h"

#include "../../System/Node/AStarNode/AStarNodeManager.h"

void NodeEditorWindow::Draw()
{
	ImGui::SetNextWindowPos(
		m_windowPos,
		ImGuiCond_Always);

	ImGui::SetNextWindowSize(
		m_windowSize,
		ImGuiCond_Always);

	ImGui::Begin(
		"Node Editor",
		nullptr,
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize);

	DrawContents();

	ImGui::End();
}

void NodeEditorWindow::DrawContents()
{
	DrawToolbar();

	ImGui::Separator();

	DrawNodeList();

	ImGui::Separator();

	DrawNodeInspector();
}

void NodeEditorWindow::DrawToolbar()
{
	ImGui::Text("Mode");

	if (ImGui::RadioButton(
		"Select",
		m_mode == Mode::Select))
	{
		SetMode(Mode::Select);
	}

	ImGui::SameLine();

	if (ImGui::RadioButton(
		"Create",
		m_mode == Mode::Create))
	{
		SetMode(Mode::Create);
	}

	ImGui::SameLine();

	if (ImGui::RadioButton(
		"Connect",
		m_mode == Mode::Connect))
	{
		SetMode(Mode::Connect);
	}

//	ImGui::SameLine();

	if (ImGui::RadioButton(
		"Disconnect",
		m_mode == Mode::Disconnect))
	{
		SetMode(Mode::Disconnect);
	}

	ImGui::SameLine();

	if (ImGui::RadioButton(
		"Erase",
		m_mode == Mode::Erase))
	{
		SetMode(Mode::Erase);
	}
}

void NodeEditorWindow::DrawNodeList()
{
	auto& manager =	AStarNodeManager::Instance();

	ImGui::SeparatorText("Nodes");

	const auto& nodes =
		manager.GetNodes();

	for (auto& node : nodes)
	{
		if (!node)
		{
			continue;
		}

		int id = node->GetID();

		char label[64];

		sprintf_s(
			label,
			"Node %d",
			id);

		bool selected =
			(m_selectedNodeID == id);

		if (ImGui::Selectable(label, selected))
		{
			m_selectedNodeID = id;
			m_clickedNodeID = id;
		}
	}
}

void NodeEditorWindow::DrawNodeInspector()
{
	auto& manager = AStarNodeManager::Instance();

	auto node =
		manager.FindNode(m_selectedNodeID);

	if (!node)
	{
		ImGui::TextDisabled(
			"No Node Selected");

		return;
	}

	ImGui::SeparatorText("Inspector");

	//--------------------------------------------------------
	// ID
	//--------------------------------------------------------

	ImGui::Text(
		"ID : %d",
		node->GetID());

	//--------------------------------------------------------
	// Position
	//--------------------------------------------------------

	Math::Vector3 pos =
		node->GetPos();

	if (ImGui::DragFloat3(
		"Position",
		&pos.x,
		0.05f))
	{
		node->SetPos(pos);
	}

	//--------------------------------------------------------
	// Connections
	//--------------------------------------------------------

	ImGui::SeparatorText(
		"Connections");

	const auto& connections =
		node->GetConnections();

	if (connections.empty())
	{
		ImGui::TextDisabled(
			"No Connections");
	}
	else
	{
		for (int id : connections)
		{
			ImGui::BulletText(
				"Node %d",
				id);
		}
	}

	//--------------------------------------------------------
	// Delete
	//--------------------------------------------------------

	ImGui::Separator();

	if (ImGui::Button(
		"Delete Node",
		{ -1, 0 }))
	{
		manager.DeleteNode(
			node->GetID());

		m_selectedNodeID = -1;
	}
}

void NodeEditorWindow::SetMode(Mode mode)
{
	if (m_mode != mode)
	{
		m_modeChanged = true;
	}

	m_mode = mode;
}