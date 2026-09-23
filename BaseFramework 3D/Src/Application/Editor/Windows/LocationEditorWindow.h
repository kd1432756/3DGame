#pragma once

#include "../../System/Location/LocationTag.h"

class LocationPoint;

class LocationEditorWindow
{
public:
	enum class Mode
	{
		Select,
		Create,
		Erase,
	};

	void Draw();
	void DrawContents();

	void SetLayout(const ImVec2& pos, const ImVec2& size)
	{
		m_windowPos = pos;
		m_windowSize = size;
	}

	Mode GetMode() const { return m_mode; }
	void SetMode(Mode mode) { m_mode = mode; }

	int GetSelectedPointID() const { return m_selectedPointID; }
	void SetSelectedPoint(int id) { m_selectedPointID = id; }

	LocationTag GetCreateTag() const { return m_createTag; }

private:
	void DrawToolbar();
	void DrawTagCoverage();
	void DrawTagFilter();
	void DrawPointList();
	void DrawPointInspector();

	ImVec2 m_windowPos = { 0, 0 };
	ImVec2 m_windowSize = { 250, 700 };

	Mode m_mode = Mode::Select;

	int m_selectedPointID = -1;

	// -1 = 全タグ表示
	int m_filterTag = -1;

	// Createモードで新規作成する際に使うタグ
	LocationTag m_createTag = LocationTag::Park;
};