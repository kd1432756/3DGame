#include "StageEditorWindow.h"

void StageEditorWindow::Init()
{
	m_tiles =
	{
		{
			"Straight",
			"Asset/Models/Ground/Straight.gltf",
			Ground::GroundType::Road
		},

		{
			"Corner",
			"Asset/Models/Ground/Corner.gltf",
			Ground::GroundType::Road
		},

		{
			"TJunction",
			"Asset/Models/Ground/TJunction.gltf",
			Ground::GroundType::Road
		},

		{
			"Grass",
			"Asset/Models/Ground/Grass.gltf",
			Ground::GroundType::Grass
		},

		{
			"Dirt",
			"Asset/Models/Ground/Dirt.gltf",
			Ground::GroundType::Dirt
		},

		{
			"Lot",
			"Asset/Models/Ground/Lot.gltf",
			Ground::GroundType::Lot
		},

		{
			"LotBorder",
			"Asset/Models/Ground/LotBorder.gltf",
			Ground::GroundType::Lot
		},

		{
			"LotCorner",
			"Asset/Models/Ground/LotCorner.gltf",
			Ground::GroundType::Lot
		},

		{
			"LotExit",
			"Asset/Models/Ground/LotExit.gltf",
			Ground::GroundType::Lot
		},

		{
			"LotParking",
			"Asset/Models/Ground/LotParking.gltf",
			Ground::GroundType::Lot
		},

		{
			"Concrete",
			"Asset/Models/Ground/Concrete.gltf",
			Ground::GroundType::Concrete
		},
	};
}

void StageEditorWindow::Draw()
{
	ImGui::SetNextWindowPos(
		m_windowPos,
		ImGuiCond_Always);
	ImGui::SetNextWindowSize(
		m_windowSize,
		ImGuiCond_Always);

	ImGui::Begin(
		"Stage Editor",
		nullptr,
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize);

	//----------------------------------
	// Brush
	//----------------------------------

	ImGui::SeparatorText("Brush");

	if (ImGui::RadioButton(
		"Paint (B)",
		m_brushMode == BrushMode::Paint))
	{
		m_brushMode = BrushMode::Paint;
	}

	if (ImGui::RadioButton(
		"Erase (E)",
		m_brushMode == BrushMode::Erase))

	{
		m_brushMode = BrushMode::Erase;
	}

	//----------------------------------
	// Road
	//----------------------------------

	DrawRoadCategory();

	//----------------------------------
	// Current
	//----------------------------------

	ImGui::SeparatorText("Current Tile");

	if (const StageTile* tile = GetSelectedTile())
	{
		ImGui::Text("%s", tile->name.c_str());
	}

	//----------------------------------
	// Placement
	//----------------------------------

	ImGui::SeparatorText("Placement");

	if (ImGui::Button("Rotate 90"))
	{
		Rotate90();
	}

	ImGui::SameLine();
	ImGui::TextDisabled("(R)");

	ImGui::End();
}

void StageEditorWindow::DrawRoadCategory()
{
	ImGui::SeparatorText("Road");

	for (int i = 0; i < m_tiles.size(); i++)
	{
		bool selected = (i == m_selectedIndex);

		if (ImGui::Selectable(
			m_tiles[i].name.c_str(),
			selected))
		{
			m_selectedIndex = i;

			m_brushMode = BrushMode::Paint;
		}
	}
}