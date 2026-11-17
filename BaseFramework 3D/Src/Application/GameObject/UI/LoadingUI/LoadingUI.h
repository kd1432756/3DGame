#pragma once

#include "../UIBase/UIBase.h"

class LoadingUI : public UIBase
{
public:
	void DrawSprite() override;

	void SetProgress(float progress)
	{
		m_progress = progress;
	}

	// パスを表示する場合
	void SetCurrentPath(const std::string& path)
	{
		m_currentPath = path;
	}

	void SetComplete(bool complete)
	{
		m_complete = complete;
	}

private:
	float m_progress = 0.0f;

	std::string m_currentPath;

	bool m_complete = false;
};
