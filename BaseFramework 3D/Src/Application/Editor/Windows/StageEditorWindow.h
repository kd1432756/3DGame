#pragma once

class StageEditorWindow
{
public:

	enum class BrushMode
	{
		Paint,
		Erase
	};

	struct StageTile
	{
		std::string name;
		std::string modelPath;
	};

	void Init();
	void Draw();

	const StageTile* GetSelectedTile() const
	{
		if (m_tiles.empty())
		{
			return nullptr;
		}

		return &m_tiles[m_selectedIndex];
	}

	BrushMode GetBrushMode() const
	{
		return m_brushMode;
	}

	void SetBrushMode(BrushMode mode)
	{
		m_brushMode = mode;
	}

	float GetRotation() const
	{
		return m_rotation;
	}

	void Rotate90()
	{
		m_rotation += 90.0f;

		if (m_rotation >= 360.0f)
		{
			m_rotation = 0.0f;
		}
	}	
	
	void SetLayout(
		const ImVec2& pos,
		const ImVec2& size)
	{
		m_windowPos = pos;
		m_windowSize = size;
	}

private:

	void DrawRoadCategory();

private:

	BrushMode m_brushMode = BrushMode::Paint;

	std::vector<StageTile> m_tiles;

	int m_selectedIndex = 0;

	float m_rotation = 0.0f;

	ImVec2 m_windowPos = { 1030, 20 };
	ImVec2 m_windowSize = { 250,500 };
};