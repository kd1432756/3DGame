#include "SceneWindow.h"
#include "../../Scene/SceneManager.h"
#include"../../GameObject/Camera/EditorCamera/EditorCamera.h"
#include "../Editor.h"

#include "../../GameObject/Object/StaticMeshObject/StaticMeshObject.h"
#include "../../GameObject/Object/StaticMeshObject/Item/Item.h"

void SceneWindow::Draw(
	std::shared_ptr<EditorCamera>& editorCamera,
	std::shared_ptr<KdGameObject>& selectObject)
{
	ImGui::SetNextWindowPos(
		m_windowPos,
		ImGuiCond_Always);
	ImGui::SetNextWindowSize(
		m_windowSize,
		ImGuiCond_Always);

	ImGui::Begin("Scene", nullptr,
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize);

	//---------------------------------
	// Camera
	//---------------------------------

	bool enableInput = ImGui::IsWindowHovered();

	editorCamera->Update(enableInput);
	if (selectObject)
	{
		if (ImGui::IsKeyPressed(ImGuiKey_F))
		{
			editorCamera->Focus(selectObject->GetPos());
		}
	}
	//editorCamera->PreDraw();
	
	//---------------------------------
	// Toolbar
	//---------------------------------

	if (Editor::Instance().GetEditMode() == Editor::EditMode::Object)
	{
		DrawToolbar();
	}

	//---------------------------------
	// Scene
	//---------------------------------

	DrawSceneImage(editorCamera);

	//---------------------------------
	// Guizmo
	//---------------------------------

	if (Editor::Instance().GetEditMode() == Editor::EditMode::Object)
	{
		DrawGuizmo(editorCamera, selectObject);
	}

	//---------------------------------
	// Picking
	//---------------------------------

	if (ImageClicked())
	{
		if (!ImGuizmo::IsOver() && !ImGuizmo::IsUsing())
		{
			switch (Editor::Instance().GetEditMode())
			{
			case Editor::EditMode::Object:

				PickObject(editorCamera, selectObject);

				break;

			case Editor::EditMode::Stage:

				Editor::Instance().ApplyStageBrush(
					GetDropPosition(editorCamera));

				break;

			case Editor::EditMode::Navigation:
			{
				Math::Vector3 pos =
					GetDropPosition(editorCamera);

				auto& editor =
					Editor::Instance();

				if (editor.GetNavigationMode() ==
					NavigationEditorWindow::NavigationMode::Node)
				{
					editor.OnNodeClick(pos);
				}
				else
				{
					editor.SelectNavMeshPolygon(pos);
				}

				break;
			}
			case Editor::EditMode::Location:

				Math::Vector3 pos =
					GetDropPosition(editorCamera);

				Editor::Instance().OnLocationClick(pos);

				break;
			}
		}
	}

	ImGui::End();
}

void SceneWindow::DrawToolbar()
{
	if (ImGui::Button("Move"))
		m_operation = ImGuizmo::TRANSLATE;

	ImGui::SameLine();

	if (ImGui::Button("Rotate"))
		m_operation = ImGuizmo::ROTATE;

	ImGui::SameLine();

	if (ImGui::Button("Scale"))
		m_operation = ImGuizmo::SCALE;
}

void SceneWindow::DrawSceneImage(
	std::shared_ptr<EditorCamera>& editorCamera)
{
	m_imagePos = ImGui::GetCursorScreenPos();
	m_imageSize = ImGui::GetContentRegionAvail();

	auto tex =
		KdShaderManager::Instance()
		.m_postProcessShader
		.GetPostEffectTexture();

	m_imageSize.y = m_imageSize.x / 16.0f * 9.0f;

	if (tex)
	{
		ImGui::Image(
			(ImTextureID)tex->WorkSRView(),
			m_imageSize);
	}

	m_imagePos = ImGui::GetItemRectMin();
	m_imageSize = ImGui::GetItemRectSize();

	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload =
			ImGui::AcceptDragDropPayload("ASSET_PATH"))
		{
			const char* path =
				(const char*)payload->Data;

			std::filesystem::path file(path);

			auto obj =
				SceneManager::Instance().CreateObject(
					"StaticMeshObject",
					file.stem().string());

			auto mesh =
				std::dynamic_pointer_cast<StaticMeshObject>(obj);


			mesh->SetModelPath(path);

			mesh->SetPos(GetDropPosition(editorCamera));
		}

		if (const ImGuiPayload* payload =
			ImGui::AcceptDragDropPayload("ITEM_PATH"))
		{
			const char* path =
				(const char*)payload->Data;

			auto obj =
				SceneManager::Instance().CreateObject("Item");

			auto item =
				std::dynamic_pointer_cast<Item>(obj);

			if (item)
			{
				item->LoadItem(path);

				item->SetPos(
					GetDropPosition(editorCamera));
			}
		}

		ImGui::EndDragDropTarget();
	}
}

bool SceneWindow::ImageClicked()
{
	return ImGui::IsItemClicked(ImGuiMouseButton_Left);
}

void SceneWindow::PickObject(
	std::shared_ptr<EditorCamera>& editorCamera,
	std::shared_ptr<KdGameObject>& selectObject)
{
	ImVec2 mouse = ImGui::GetMousePos();

	POINT pt;

	pt.x = (LONG)(mouse.x - m_imagePos.x);
	pt.y = (LONG)(mouse.y - m_imagePos.y);

	Math::Vector3 rayPos;
	Math::Vector3 rayDir;
	float rayRange;

	editorCamera->GetCamera()->GenerateRayInfoFromClientPos(
		pt,
		m_imageSize.x,
		m_imageSize.y,
		rayPos,
		rayDir,
		rayRange);

	KdCollider::RayInfo ray(
		KdCollider::Type::TypeEditor,
		rayPos,
		rayDir,
		rayRange);

	float maxOverlap = 0.0f;

	std::shared_ptr<KdGameObject> hitObj = nullptr;

	for (auto& obj : SceneManager::Instance().GetObjList())
	{
		if (!obj) continue;

		std::list<KdCollider::CollisionResult> results;

		if (!obj->Intersects(ray, &results))
			continue;

		for (auto& r : results)
		{
			if (r.m_overlapDistance > maxOverlap)
			{
				maxOverlap = r.m_overlapDistance;
				hitObj = obj;
			}
		}
	}

	if (hitObj)
	{
		selectObject = hitObj;
	}
}

void SceneWindow::DrawGuizmo(std::shared_ptr<EditorCamera>& editorCamera, std::shared_ptr<KdGameObject>& selectObject)
{
	if (editorCamera && selectObject)
	{
		auto camera = editorCamera->GetCamera();

		const Math::Matrix& view = camera->GetCameraViewMatrix();
		const Math::Matrix& proj = camera->GetProjMatrix();

		Math::Matrix world = selectObject->GetMatrix();

		ImGuizmo::SetOrthographic(false);

		ImGuizmo::SetDrawlist();

		ImGuizmo::SetRect(
			m_imagePos.x,
			m_imagePos.y,
			m_imageSize.x,
			m_imageSize.y);

		if (ImGuizmo::Manipulate(
			(float*)&view, (float*)&proj,
			m_operation,
			ImGuizmo::WORLD,
			(float*)&world))
		{
			Math::Vector3 scale;
			Math::Quaternion rot;
			Math::Vector3 pos;

			world.Decompose(scale, rot, pos);

			Math::Vector3 euler = rot.ToEuler();

			euler.x = DirectX::XMConvertToDegrees(euler.x);
			euler.y = DirectX::XMConvertToDegrees(euler.y);
			euler.z = DirectX::XMConvertToDegrees(euler.z);

			selectObject->SetPos(pos);
			selectObject->SetRot(euler);
			selectObject->SetScale(scale);
		}
	}
}

Math::Vector3 SceneWindow::GetDropPosition(
    std::shared_ptr<EditorCamera>& camera)
{
	ImVec2 mouse = ImGui::GetMousePos();

	POINT pt;

	pt.x = (LONG)(mouse.x - m_imagePos.x);
	pt.y = (LONG)(mouse.y - m_imagePos.y);

	Math::Vector3 rayPos;
	Math::Vector3 rayDir;
	float rayRange;

	camera->GetCamera()->GenerateRayInfoFromClientPos(
		pt,
		m_imageSize.x,
		m_imageSize.y,
		rayPos,
		rayDir,
		rayRange);

	KdCollider::RayInfo ray(
		KdCollider::Type::TypeEditor,
		rayPos,
		rayDir,
		rayRange);

	float maxOverlap = 0.0f;
	Math::Vector3 hitPos;
	bool hit = false;

	for (auto& obj : SceneManager::Instance().GetObjList())
	{
		if (!obj) continue;

		std::list<KdCollider::CollisionResult> results;

		if (!obj->Intersects(ray, &results))
			continue;

		for (auto& r : results)
		{
			if (r.m_overlapDistance > maxOverlap)
			{
				maxOverlap = r.m_overlapDistance;
				hitPos = r.m_hitPos;
				hit = true;
			}
		}
	}

	if (hit)
	{
		return hitPos;
	}

	if (fabs(rayDir.y) < 0.0001f)
		return { 0,0,0 };

	float t = -rayPos.y / rayDir.y;

	return rayPos + rayDir * t;
}

/*void SceneWindow::OnNodeClick(const Math::Vector3& pos)
{
	static constexpr float NODE_SELECT_RADIUS = 1.0f;
}*/
