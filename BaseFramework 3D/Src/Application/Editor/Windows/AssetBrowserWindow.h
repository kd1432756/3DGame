#pragma once

class AssetBrowserWindow
{
public:
	void Draw();

	void SetLayout(const ImVec2& pos, const ImVec2& size)
	{
		m_windowPos = pos;
		m_windowSize = size;
	}

private:
	void DrawDirectory(const std::string& path);

	std::string m_selectedPath;

	ImVec2 m_windowPos = { 0,520 };
	ImVec2 m_windowSize = { 1280,200 };
};