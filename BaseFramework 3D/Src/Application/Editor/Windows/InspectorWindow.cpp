#include "InspectorWindow.h"

#include "../../GameObject/Object/StaticMeshObject/StaticMeshObject.h"
#include "../../GameObject/Object/StaticMeshObject/InteractiveObject/InteractiveObject.h"
#include "../../GameObject/Object/StaticMeshObject/Item/Item.h"

void InspectorWindow::Draw(std::shared_ptr<KdGameObject> obj)
{
	ImGui::SetNextWindowPos(
		m_windowPos,
		ImGuiCond_Always);
	ImGui::SetNextWindowSize(
		m_windowSize,
		ImGuiCond_Always);

	ImGui::Begin("Inspector", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

	if (obj)
	{
		ImGui::InputText("Name", &obj->GetName());

		ImGui::SeparatorText("Transform");

		// Position
		Math::Vector3 pos = obj->GetPos();
		if(ImGui::DragFloat3("Position", &pos.x, 0.05f))
		{
			obj->SetPos(pos);
		}

		// Rotation
		Math::Vector3 rot = obj->GetRot();
		if(ImGui::DragFloat3("Rotation", &rot.x, 0.5f))
		{
			obj->SetRot(rot);
		}

		// Scale
		Math::Vector3 scale = obj->GetScale();
		if(ImGui::DragFloat3("Scale", &scale.x, 0.001f, 0.001f, 100.0f))
		{
			obj->SetScale(scale);
		}

		if (obj)
		{
			Math::Color color = obj->GetColor();

			if (ImGui::ColorEdit3(
				"Color",
				&color.x))
			{
				obj->SetColor(color);
			}
		}
	}

	auto meshObj =
		std::dynamic_pointer_cast<StaticMeshObject>(obj);

	if (meshObj)
	{
		DrawStaticMesh(meshObj);
	}

	auto interactive =
		std::dynamic_pointer_cast<InteractiveObject>(obj);

	if (interactive)
	{
		DrawInteractive(interactive);
	}

	auto item =
		std::dynamic_pointer_cast<Item>(obj);

	if (item)
	{
		DrawItem(item);
	}

	ImGui::End();
}

void InspectorWindow::DrawStaticMesh(
	std::shared_ptr<StaticMeshObject> obj)
{
	ImGui::SeparatorText("Static Mesh");

	char buf[260];

	strcpy_s(
		buf,
		obj->GetModelPath().c_str());

	if (ImGui::InputText(
		"Model Path",
		buf,
		sizeof(buf),
		ImGuiInputTextFlags_EnterReturnsTrue))
	{
		obj->SetModelPath(buf);
	}

	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload =
			ImGui::AcceptDragDropPayload("ASSET_PATH"))
		{
			const char* path =
				(const char*)payload->Data;

			obj->SetModelPath(path);
		}

		ImGui::EndDragDropTarget();
	}
}

void InspectorWindow::DrawInteractive(std::shared_ptr<InteractiveObject> obj)
{
	ImGui::SeparatorText("Interactive");

	std::string owner = obj->GetOwner();

	if (ImGui::InputText("Owner", &owner))
	{
		obj->SetOwner(owner);
	}
}

void InspectorWindow::DrawItem(std::shared_ptr<Item> obj)
{
	ImGui::SeparatorText("Item");

	char buf[260];

	strcpy_s(
		buf,
		obj->GetItemPath().c_str());

	// Enterで変更
	if (ImGui::InputText(
		"Item File",
		buf,
		sizeof(buf),
		ImGuiInputTextFlags_EnterReturnsTrue))
	{
		obj->LoadItem(buf);
	}

	// AssetBrowserからドラッグ&ドロップ
	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload =
			ImGui::AcceptDragDropPayload("ITEM_PATH"))
		{
			const char* path =
				(const char*)payload->Data;

			obj->LoadItem(path);
		}

		ImGui::EndDragDropTarget();
	}

	// 読み込まれている情報を表示
	const ItemData& data = obj->GetItemData();

	ImGui::Text("Name : %s", data.GetName().c_str());
	ImGui::Text("Action : %d", (int)data.GetAction());
}