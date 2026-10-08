#include "InspectorWindow.h"

#include "../../Scene/SceneManager.h"

#include "../../GameObject/Object/StaticMeshObject/StaticMeshObject.h"
#include "../../GameObject/Object/StaticMeshObject/InteractiveObject/InteractiveObject.h"
#include "../../GameObject/Object/StaticMeshObject/Item/Item.h"
#include "../../GameObject/Character/Enemy/Neighbor/Neighbor.h"

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

	auto neighbor =
		std::dynamic_pointer_cast<Neighbor>(obj);

	if (neighbor)
	{
		DrawNeighbor(neighbor);
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

	// 現在設定されているOwner
	std::string currentOwner = obj->GetOwner();

	// Comboに表示する名前
	const char* previewName =
		currentOwner.empty() ? "None" : currentOwner.c_str();

	if (ImGui::BeginCombo("Owner", previewName))
	{
		// 現在存在しているCharacterBaseの一覧を取得
		const auto& characters =
			SceneManager::Instance().FindObjects<CharacterBase>();

		// Ownerなし
		bool isNoneSelected = currentOwner.empty();

		if (ImGui::Selectable("None", isNoneSelected))
		{
			obj->SetOwner("");
		}

		if (isNoneSelected)
		{
			ImGui::SetItemDefaultFocus();
		}

		// Character一覧
		for (const auto& character : characters)
		{
			if (!character)
			{
				continue;
			}

			const std::string& name =
				character->GetName();

			bool isSelected =
				(currentOwner == name);

			if (ImGui::Selectable(
				name.c_str(),
				isSelected))
			{
				// Characterの名前をOwnerに設定
				obj->SetOwner(name);
			}

			if (isSelected)
			{
				ImGui::SetItemDefaultFocus();
			}
		}

		ImGui::EndCombo();
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
}

void InspectorWindow::DrawNeighbor(std::shared_ptr<Neighbor> obj)
{
	ImGui::SeparatorText("Neighbor");

	LocationTag currentTag = obj->GetHouseTag();

	const char* currentName =
		LocationTagToString(currentTag);

	if (ImGui::BeginCombo("House Tag", currentName))
	{
		for (int i = 0; i < static_cast<int>(LocationTag::Max); ++i)
		{
			LocationTag tag = static_cast<LocationTag>(i);

			//============================================================
			// 家として使えるTagのみ表示
			//============================================================

			if (tag != LocationTag::NeighborHouse1 &&
				tag != LocationTag::NeighborHouse2 &&
				tag != LocationTag::NeighborHouse3)
			{
				continue;
			}

			bool isSelected = (tag == currentTag);

			if (ImGui::Selectable(LocationTagToString(tag), isSelected))
			{
				obj->SetHouseTag(tag);
			}

			if (isSelected)
			{
				ImGui::SetItemDefaultFocus();
			}
		}

		ImGui::EndCombo();
	}
}
