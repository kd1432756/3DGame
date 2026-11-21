#include "LocationEditorWindow.h"

#include "../../System/Location/LocationPoint.h"
#include "../../System/Location/LocationPointManager.h"

void LocationEditorWindow::Draw()
{
	ImGui::SetNextWindowPos(m_windowPos, ImGuiCond_Always);
	ImGui::SetNextWindowSize(m_windowSize, ImGuiCond_Always);

	ImGui::Begin("Location Editor", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

	DrawContents();

	ImGui::End();
}

void LocationEditorWindow::DrawContents()
{
	DrawToolbar();

	ImGui::Separator();

	DrawTagCoverage();

	ImGui::Separator();

	DrawTagFilter();

	DrawPointList();

	ImGui::Separator();

	DrawPointInspector();
}

//============================================================
// Toolbar
//============================================================

void LocationEditorWindow::DrawToolbar()
{
	ImGui::Text("Mode");

	if (ImGui::RadioButton("Select", m_mode == Mode::Select))
	{
		SetMode(Mode::Select);
	}

	ImGui::SameLine();

	if (ImGui::RadioButton("Create", m_mode == Mode::Create))
	{
		SetMode(Mode::Create);
	}

	ImGui::SameLine();

	if (ImGui::RadioButton("Erase", m_mode == Mode::Erase))
	{
		SetMode(Mode::Erase);
	}

	if (m_mode == Mode::Create)
	{
		int currentTag = static_cast<int>(m_createTag);

		if (ImGui::BeginCombo("Create Tag", LocationTagToString(m_createTag)))
		{
			for (int i = 0; i < static_cast<int>(LocationTag::Max); ++i)
			{
				bool selected = (currentTag == i);

				if (ImGui::Selectable(LocationTagToString(static_cast<LocationTag>(i)), selected))
				{
					m_createTag = static_cast<LocationTag>(i);
				}
			}

			ImGui::EndCombo();
		}

		ImGui::TextDisabled("Click in Scene to place");
	}
}

//============================================================
// Tag Coverage（配置漏れ・重複チェック）
//============================================================

void LocationEditorWindow::DrawTagCoverage()
{
	ImGui::SeparatorText("Tag Coverage");

	auto& manager = LocationPointManager::Instance();

	for (int i = 0; i < static_cast<int>(LocationTag::Max); ++i)
	{
		LocationTag tag = static_cast<LocationTag>(i);

		auto points = manager.FindAllByTag(tag);

		ImVec4 color;
		const char* status;

		if (points.empty())
		{
			color = { 1.0f, 0.4f, 0.4f, 1.0f };
			status = "MISSING";
		}
		else if (points.size() > 1 && IsUniqueLocationTag(tag))
		{
			color = { 1.0f, 0.8f, 0.2f, 1.0f };
			status = "DUPLICATE";
		}
		else
		{
			color = { 0.4f, 1.0f, 0.4f, 1.0f };
			status = "OK";
		}

		ImGui::TextColored(
			color,
			"%-16s x%d [%s]",
			LocationTagToString(tag),
			static_cast<int>(points.size()),
			status);
	}
}

//============================================================
// Tag Filter
//============================================================

void LocationEditorWindow::DrawTagFilter()
{
	ImGui::SeparatorText("Filter");

	std::string preview = (m_filterTag < 0) ? "All" : LocationTagToString(static_cast<LocationTag>(m_filterTag));

	if (ImGui::BeginCombo("Tag Filter", preview.c_str()))
	{
		if (ImGui::Selectable("All", m_filterTag < 0))
		{
			m_filterTag = -1;
		}

		for (int i = 0; i < static_cast<int>(LocationTag::Max); ++i)
		{
			bool selected = (m_filterTag == i);

			if (ImGui::Selectable(LocationTagToString(static_cast<LocationTag>(i)), selected))
			{
				m_filterTag = i;
			}
		}

		ImGui::EndCombo();
	}
}

//============================================================
// Point List
//============================================================

void LocationEditorWindow::DrawPointList()
{
	ImGui::SeparatorText("Points");

	auto& manager = LocationPointManager::Instance();

	for (auto& point : manager.GetPoints())
	{
		if (!point)
		{
			continue;
		}

		if (m_filterTag >= 0 && point->GetTag() != static_cast<LocationTag>(m_filterTag))
		{
			continue;
		}

		char label[64];

		sprintf_s(
			label,
			"[%d] %s",
			point->GetID(),
			LocationTagToString(point->GetTag()));

		bool selected = (m_selectedPointID == point->GetID());

		if (ImGui::Selectable(label, selected))
		{
			m_selectedPointID = point->GetID();
		}
	}
}

//============================================================
// Point Inspector
//============================================================

void LocationEditorWindow::DrawPointInspector()
{
	auto& manager = LocationPointManager::Instance();

	auto point = manager.FindPoint(m_selectedPointID);

	if (!point)
	{
		ImGui::TextDisabled("No Point Selected");
		return;
	}

	ImGui::SeparatorText("Inspector");

	//--------------------------------------------------------
	// ID
	//--------------------------------------------------------

	ImGui::Text("ID : %d", point->GetID());

	//--------------------------------------------------------
	// Tag
	//--------------------------------------------------------

	if (ImGui::BeginCombo("Tag", LocationTagToString(point->GetTag())))
	{
		for (int i = 0; i < static_cast<int>(LocationTag::Max); ++i)
		{
			bool selected = (point->GetTag() == static_cast<LocationTag>(i));

			if (ImGui::Selectable(LocationTagToString(static_cast<LocationTag>(i)), selected))
			{
				point->SetTag(static_cast<LocationTag>(i));
			}
		}

		ImGui::EndCombo();
	}

	//--------------------------------------------------------
	// Position
	//--------------------------------------------------------

	Math::Vector3 pos = point->GetPos();

	if (ImGui::DragFloat3("Position", &pos.x, 0.05f))
	{
		point->SetPos(pos);
	}

	//--------------------------------------------------------
	// Delete
	//--------------------------------------------------------

	ImGui::Separator();

	if (ImGui::Button("Delete Point", { -1, 0 }))
	{
		manager.DeletePoint(point->GetID());

		m_selectedPointID = -1;
	}
}