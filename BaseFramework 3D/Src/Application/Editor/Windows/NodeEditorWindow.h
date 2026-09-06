#pragma once

class NodeEditorWindow
{
public:

	enum class Mode
	{
		Select,
		Create,
		Connect,
		Disconnect,
		Erase
	};

	void Draw();
	void DrawContents();

	void SetLayout(
		const ImVec2& pos,
		const ImVec2& size)
	{
		m_windowPos = pos;
		m_windowSize = size;
	}	
	
	void SetSelectedNode(int id)
	{
		m_selectedNodeID = id;
	}

	int GetSelectedNodeID() const
	{
		return m_selectedNodeID;
	}

	Mode GetMode() const
	{
		return m_mode;
	}

	void SetMode(Mode mode);

private:

	void DrawToolbar();
	void DrawNodeList();
	void DrawNodeInspector();

	ImVec2 m_windowPos = { 1030, 20 };
	ImVec2 m_windowSize = { 250, 700 };

	int m_selectedNodeID = -1;

	Mode m_mode = Mode::Select;
};
