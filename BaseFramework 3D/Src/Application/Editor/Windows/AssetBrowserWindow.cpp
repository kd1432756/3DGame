#include "AssetBrowserWindow.h"

namespace fs = std::filesystem;

void AssetBrowserWindow::Draw()
{
	ImGui::SetNextWindowPos(m_windowPos, ImGuiCond_Always);
	ImGui::SetNextWindowSize(m_windowSize, ImGuiCond_Always);

	ImGui::Begin("Asset Browser", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

	DrawDirectory("Asset");

	ImGui::End();
}

void AssetBrowserWindow::DrawDirectory(const std::string& path)
{
	for (auto& entry : fs::directory_iterator(path))
	{
		std::string filenameStr = entry.path().filename().string();

		// "Asset" 直下の階層である場合、"Items" と "Models" 以外はスキップする
		if (path == "Asset" && entry.is_directory())
		{
			if (filenameStr != "Items" && filenameStr != "Models")
			{
				continue;
			}
		}

		if (entry.is_directory())
		{
			if (ImGui::TreeNode(entry.path().filename().string().c_str()))
			{
				DrawDirectory(entry.path().string());

				ImGui::TreePop();
			}
		}
		else
		{
			const auto extension = entry.path().extension();

			if (extension != ".gltf" && extension != ".item")
			{
				continue;
			}

			if (ImGui::Selectable(entry.path().filename().string().c_str(), m_selectedPath == entry.path().string()))
			{
				m_selectedPath = entry.path().string();
			}

			if (ImGui::BeginDragDropSource())
			{
				std::string path = entry.path().string();

				if (entry.path().extension() == ".item")
				{
					ImGui::SetDragDropPayload(
						"ITEM_PATH",
						path.c_str(),
						path.size() + 1);

					ImGui::Text("%s", entry.path().filename().string().c_str());
				}
				else
				{
					ImGui::SetDragDropPayload(
						"ASSET_PATH",
						path.c_str(),
						path.size() + 1);

					ImGui::Text("%s", path.c_str());
				}

				ImGui::EndDragDropSource();
			}
		}
	}
}