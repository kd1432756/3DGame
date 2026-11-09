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

	// ※ 現在は "Loading" 固定表示のため描画には使っていない(呼び出し側の互換用)
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
