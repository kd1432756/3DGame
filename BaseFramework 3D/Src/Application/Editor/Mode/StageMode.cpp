#include "StageMode.h"

#include "../../Scene/SceneManager.h"
#include "../../GameObject/Object/StaticMeshObject/Ground/Ground.h"
#include "../../System/Stage/StageConst.h"

using StageConst::GRID_SIZE;

void StageMode::ApplyLayout(EditorContext& context)
{
	context.sceneWindow.SetLayout({ 0, 20 }, { 1030, 700 });
	m_window.SetLayout({ 1030, 20 }, { 250, 700 });
}

void StageMode::Update(EditorContext& context, EditorIO&)
{
	// グリッドに位置を合わせる
	m_previewPos = Snap(context.GetCursorWorldPos());

	if (ImGui::IsKeyPressed(ImGuiKey_B))
	{
		m_window.SetBrushMode(StageEditorWindow::BrushMode::Paint);
	}

	if (ImGui::IsKeyPressed(ImGuiKey_E))
	{
		m_window.SetBrushMode(StageEditorWindow::BrushMode::Erase);
	}

	if (ImGui::IsKeyPressed(ImGuiKey_R))
	{
		m_window.Rotate90();
	}
}

void StageMode::DrawWindows(EditorContext&)
{
	m_window.Draw();
}

void StageMode::DrawDebug(EditorContext&)
{
	if (m_window.GetBrushMode() == StageEditorWindow::BrushMode::Paint)
	{
		DrawPlacePreview();
	}
	else
	{
		DrawErasePreview();
	}
}

void StageMode::OnSceneClick(EditorContext&, const Math::Vector3 & pos)
{
	// 配置、消去
	const Math::Vector3 snapped = Snap(pos);

	switch (m_window.GetBrushMode())
	{
	case StageEditorWindow::BrushMode::Paint: PlaceTile(snapped); break;
	case StageEditorWindow::BrushMode::Erase: EraseTile(snapped); break;
	}
}

Math::Vector3 StageMode::Snap(const Math::Vector3& pos)
{
	return
	{
		roundf(pos.x / GRID_SIZE) * GRID_SIZE,
		roundf(pos.y / GRID_SIZE) * GRID_SIZE,
		roundf(pos.z / GRID_SIZE) * GRID_SIZE
	};
}

StageMode::GridCoord StageMode::ToGrid(const Math::Vector3& pos)
{
	return
	{
		(int)round(pos.x / GRID_SIZE),
		(int)round(pos.y / GRID_SIZE),
		(int)round(pos.z / GRID_SIZE)
	};
}

std::shared_ptr<Ground> StageMode::FindGroundAt(const GridCoord& grid) const
{
	// 重なってないか
	for (auto& obj : SceneManager::Instance().GetObjList())
	{
		auto ground = std::dynamic_pointer_cast<Ground>(obj);

		if (ground && ToGrid(ground->GetPos()) == grid)
		{
			return ground;
		}
	}

	return nullptr;
}

void StageMode::PlaceTile(const Math::Vector3& snapped)
{
	// 重なってないか
	if (FindGroundAt(ToGrid(snapped))) return;

	const auto* tile = m_window.GetSelectedTile();

	if (!tile) return;

	auto ground = std::dynamic_pointer_cast<Ground>(SceneManager::Instance().CreateObject("Ground"));

	if (!ground) return;

	ground->SetModelPath(tile->modelPath);
	ground->SetPos(snapped);
	ground->SetScale(GRID_SIZE / 8.0f);
	ground->SetRot({ 0, m_window.GetRotation(), 0 });
	ground->SetGroundType(tile->groundType);
}

void StageMode::EraseTile(const Math::Vector3 & snapped)
{
	// 重なってるか
	if (auto ground = FindGroundAt(ToGrid(snapped)))
	{
		ground->Destroy();
	}
}

void StageMode::DrawPlacePreview()
{
	// 置くときのプレビュー
	const auto* tile = m_window.GetSelectedTile();

	if (!tile) return;

	if (FindGroundAt(ToGrid(m_previewPos))) return;

	// モデルが変わった時だけ読み直す
	if (m_previewPath != tile->modelPath)
	{
		if (!m_previewModel)
		{
			m_previewModel = std::make_shared<KdModelWork>();
		}

		m_previewModel->SetModelData(tile->modelPath);
		m_previewPath = tile->modelPath;
	}

	const Math::Matrix mat =
		Math::Matrix::CreateScale(GRID_SIZE / 8.0f) *
		Math::Matrix::CreateRotationY(DirectX::XMConvertToRadians(m_window.GetRotation())) *
		Math::Matrix::CreateTranslation(m_previewPos);

	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_previewModel, mat);
}

void StageMode::DrawErasePreview()
{
	// 消すやつを赤線で描画
	m_wire.AddDebugBox(
		Math::Matrix::CreateTranslation(m_previewPos),
		{ GRID_SIZE / 2.0f, 0.5f, GRID_SIZE / 2.0f },
		Math::Vector3::Zero,
		true,
		kRedColor);

	m_wire.Draw();
}
