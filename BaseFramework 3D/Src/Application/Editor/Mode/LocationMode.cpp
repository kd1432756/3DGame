#include "LocationMode.h"

#include "../../System/Location/LocationPointManager.h"

void LocationMode::ApplyLayout(EditorContext& context)
{
	context.sceneWindow.SetLayout({ 0, 20 }, { 1030, 700 });
	m_window.SetLayout({ 1030, 20 }, { 250, 700 });
}

void LocationMode::Update(EditorContext&, EditorIO&)
{
	if (m_window.GetMode() != LocationEditorWindow::Mode::Select) return;

	auto& manager = LocationPointManager::Instance();

	auto point = manager.FindPoint(m_window.GetSelectedPointID());

	// deleteで場所のノードを消す
	if (point && ImGui::IsKeyPressed(ImGuiKey_Delete))
	{
		manager.DeletePoint(point->GetID());
		m_window.SetSelectedPoint(-1);
	}
}

void LocationMode::DrawWindows(EditorContext&)
{
	m_window.Draw();
}

void LocationMode::DrawDebug(EditorContext& context)
{
	auto& manager = LocationPointManager::Instance();

	for (auto& point : manager.GetPoints())
	{
		if (!point) continue;

		bool selected = false;
		if (point->GetID() == m_window.GetSelectedPointID()) selected = true;

		m_wire.AddDebugSphere(
			point->GetPos(),
			0.3f,
			selected ? kYellowColor : kGreenColor);
	}

	m_wire.Draw();

	// タグ名
	for (auto& point : manager.GetPoints())
	{
		if (!point) continue;

		Math::Vector3 worldPos = point->GetPos();
		worldPos.y += 0.5f;

		Math::Vector3 screenPos = Math::Vector3::Zero;

		// 画面内のみ
		if (!context.camera->ConvertWorldToScreenDetail(worldPos, screenPos)) continue;

		KdShaderManager::Instance().m_spriteShader.DrawFont(
			{ screenPos.x, screenPos.y },
			&kWhiteColor,
			{ 0.5f, 0.5f },
			0.0f,
			FontID::Future16,
			LocationTagToString(point->GetTag()));
	}
}

void LocationMode::OnSceneClick(EditorContext&, const Math::Vector3 & pos)
{
	constexpr float SELECT_RADIUS = 1.0f;

	auto& manager = LocationPointManager::Instance();

	std::shared_ptr<LocationPoint> nearest;
	float nearestSq = SELECT_RADIUS * SELECT_RADIUS;

	// 近くの場所ノードを探す
	for (auto& point : manager.GetPoints())
	{
		if (!point) continue;

		const float distSq = (point->GetPos() - pos).LengthSquared();

		if (distSq <= nearestSq)
		{
			nearestSq = distSq;
			nearest = point;
		}
	}

	switch (m_window.GetMode())
	{
	case LocationEditorWindow::Mode::Select:
		// 選択
		if (nearest)
		{
			m_window.SetSelectedPoint(nearest->GetID());
		}
		else
		{
			m_window.SetSelectedPoint(-1);
		}
		break;

	case LocationEditorWindow::Mode::Create:
		// 近くになかったら作る
		if (!nearest)
		{
			if (auto point = manager.CreatePoint(m_window.GetCreateTag(), pos))
			{
				m_window.SetSelectedPoint(point->GetID());
			}
		}
		break;

	case LocationEditorWindow::Mode::Erase:
		// 消す
		if (nearest)
		{
			manager.DeletePoint(nearest->GetID());
			m_window.SetSelectedPoint(-1);
		}
		break;
	}
}

void LocationMode::DrawFileMenu(EditorIO& io)
{
	if (!ImGui::BeginMenu("Location")) return;

	if (ImGui::MenuItem("Save"))
	{
		io.SaveLocations();
	}

	if (ImGui::MenuItem("Load"))
	{
		if (io.LoadLocations())
		{
			m_window.SetSelectedPoint(-1);
		}
	}

	ImGui::EndMenu();
}

void LocationMode::OnDataReloaded(EditorContext&)
{
	m_window.SetSelectedPoint(-1);
}
