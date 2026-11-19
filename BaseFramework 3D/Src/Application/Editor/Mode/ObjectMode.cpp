#include "ObjectMode.h"

#include "../EditorIO.h"
#include "../../Scene/SceneManager.h"

void ObjectMode::ApplyLayout(EditorContext& context)
{
	m_hierarchy.SetLayout({ 0, 20 }, { 250, 500 });
	context.sceneWindow.SetLayout({ 250, 20 }, { 780, 500 });
	m_inspector.SetLayout({ 1030, 20 }, { 250, 500 });
	m_assetBrowser.SetLayout({ 0, 520 }, { 1280, 200 });
}

void ObjectMode::Update(EditorContext& context, EditorIO& io)
{
	const ImGuiIO& gio = ImGui::GetIO();

	// deleteで消す
	if (context.selectObject && ImGui::IsKeyPressed(ImGuiKey_Delete))
	{
		DeleteSelected(context);
	}

	// Ctrl+Dで複製
	if (gio.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_D))
	{
		DuplicateSelected(context);
	}
}

void ObjectMode::DrawWindows(EditorContext& context)
{
	m_hierarchy.Draw(context.selectObject);
	m_inspector.Draw(context.selectObject);
	m_assetBrowser.Draw();
}

void ObjectMode::OnSceneClick(EditorContext& context, const Math::Vector3&)
{
	context.sceneWindow.PickObject(context.camera, context.selectObject);
}

void ObjectMode::DrawMenuBar(EditorContext& context)
{
	// オブジェクト
	static constexpr CreateEntry kBasicItems[] =
	{
		{ "Static Mesh", "StaticMeshObject", 0 },
		{ "Item", "Item", 0 },
	};

	// キャラ
	static constexpr CreateEntry kCharacters[] =
	{
		{ "Player", "Player", 0 },
		{ "Neighbor", "Neighbor", 0 },
		{ "Police", "Police", 0 },
	};

	// 木は大きさを変更
	static constexpr CreateEntry kProps[] =
	{
		{ "Tree", "Tree", 10.0f },
	};

	// インタラクト可能オブジェクト
	static constexpr CreateEntry kInteractive[] =
	{
		{ "Flower", "Flower", 0 },
		{ "Hose", "Hose", 0 },
		{ "PaintCan", "PaintCan", 0 },
		{ "WoodenFence", "WoodenFence", 0 },
		{ "Faucet", "Faucet", 0 },
		{ "Car", "Car", 0 },
		{ "Window", "Window", 0 },
	};

	if (!ImGui::BeginMenu("GameObject")) return;

	if (ImGui::MenuItem("Duplicate", "Ctrl+D")) DuplicateSelected(context);
	if (ImGui::MenuItem("Delete", "Delete")) DeleteSelected(context);

	ImGui::Separator();
	DrawCreateItems(context, kBasicItems, std::size(kBasicItems));

	ImGui::Separator();
	DrawCreateItems(context, kCharacters, std::size(kCharacters));

	ImGui::Separator();
	DrawCreateItems(context, kProps, std::size(kProps));

	ImGui::Separator();
	if (ImGui::BeginMenu("Interactive Object"))
	{
		DrawCreateItems(context, kInteractive, std::size(kInteractive));
		ImGui::EndMenu();
	}

	ImGui::EndMenu();
}

void ObjectMode::DrawCreateItems(EditorContext& context, const CreateEntry* entries, int count)
{
	// 種類別でメニューに表示
	for (int i = 0; i < count; ++i)
	{
		const auto& e = entries[i];

		if (ImGui::MenuItem(e.label))
		{
			CreateAndSelect(context, e);
		}
	}
}

void ObjectMode::DuplicateSelected(EditorContext& context)
{
	if (!context.selectObject) return;

	if (auto obj = SceneManager::Instance().DuplicateObject(context.selectObject))
	{
		context.selectObject = obj;
	}
}

void ObjectMode::DeleteSelected(EditorContext& context)
{
	if (!context.selectObject) return;

	context.selectObject->Destroy();
	context.selectObject = nullptr;
}

void ObjectMode::CreateAndSelect(EditorContext& context, const CreateEntry& entry)
{
	auto obj = SceneManager::Instance().CreateObject(entry.type);

	if (!obj) return;

	if (entry.scale > 0.0f)
	{
		obj->SetScale(entry.scale);
	}

	context.selectObject = obj;
}
