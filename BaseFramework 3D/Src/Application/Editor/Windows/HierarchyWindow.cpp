#include "HierarchyWindow.h"

#include "../../Scene/SceneManager.h"

void HierarchyWindow::Draw(std::shared_ptr<KdGameObject>& selectObject)
{
	ImGui::SetNextWindowPos(
		m_windowPos,
		ImGuiCond_Always);
	ImGui::SetNextWindowSize(
		m_windowSize,
		ImGuiCond_Always);

	ImGui::Begin("Hierarchy", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

	if (ImGui::TreeNode("Scene"))
	{
		DrawCategory(
			"Character",
			KdGameObject::ObjectCategory::Character,
			selectObject);

		DrawCategory(
			"Stage",
			KdGameObject::ObjectCategory::Stage,
			selectObject);

		DrawCategory(
			"Prop",
			KdGameObject::ObjectCategory::Prop,
			selectObject);

		DrawCategory(
			"Interactive",
			KdGameObject::ObjectCategory::Interactive,
			selectObject);

		DrawCategory(
			"Item",
			KdGameObject::ObjectCategory::Item,
			selectObject);

		DrawCategory(
			"Other",
			KdGameObject::ObjectCategory::Other,
			selectObject);

		ImGui::TreePop();
	}

	ImGui::End();
}

bool HierarchyWindow::IsNameUsed(const std::string& name,
	std::shared_ptr<KdGameObject> ignoreObj)
{
	auto& objList = SceneManager::Instance().GetObjList();

	for (auto& obj : objList)
	{
		if (obj == ignoreObj)
		{
			continue;
		}

		if (obj->GetName() == name)
		{
			return true;
		}
	}

	return false;
}

void HierarchyWindow::DrawObject(const std::shared_ptr<KdGameObject>& obj, std::shared_ptr<KdGameObject>& selectObject)
{
	bool isSelected = (obj == selectObject);

	if (obj == m_renameObject)
	{
		ImGui::SetNextItemWidth(-FLT_MIN);

		if (m_startRename)
		{
			ImGui::SetKeyboardFocusHere();
			m_startRename = false;
		}

		if (ImGui::InputText("##Rename", &m_renameBuffer,
			ImGuiInputTextFlags_EnterReturnsTrue))
		{
			if (!IsNameUsed(m_renameBuffer, obj))
			{
				obj->SetName(m_renameBuffer);
			}
			m_renameObject = nullptr;
		}

		if (ImGui::IsItemDeactivatedAfterEdit())
		{
			if (!IsNameUsed(m_renameBuffer, obj))
			{
				obj->SetName(m_renameBuffer);
			}

			m_renameObject = nullptr;
		}
	}
	else
	{
		if (ImGui::Selectable(obj->GetName().c_str(), isSelected))
		{
			selectObject = obj;
		}

		if (isSelected && ImGui::IsKeyPressed(ImGuiKey_F2))
		{
			m_renameObject = obj;
			m_renameBuffer = obj->GetName();
			m_startRename = true;
		}
	}

	if (ImGui::BeginPopupContextItem())
	{
		if (ImGui::MenuItem("Rename"))
		{
			m_renameObject = obj;
			m_renameBuffer = obj->GetName();
			m_startRename = true;
		}

		if (ImGui::MenuItem("Duplicate"))
		{
			SceneManager::Instance().DuplicateObject(obj);
		}

		if (ImGui::MenuItem("Delete"))
		{
			obj->Destroy();

			if (selectObject == obj)
			{
				selectObject = nullptr;
			}
		}

		ImGui::EndPopup();
	}
}

void HierarchyWindow::DrawCategory(const char* title, KdGameObject::ObjectCategory category, std::shared_ptr<KdGameObject>& selectObject)
{
	if (!ImGui::TreeNode(title))
	{
		return;
	}

	auto& objList = SceneManager::Instance().GetObjList();

	for (auto& obj : objList)
	{
		if (obj->GetCategory() != category)
		{
			continue;
		}

		DrawObject(obj, selectObject);
	}

	ImGui::TreePop();
}
