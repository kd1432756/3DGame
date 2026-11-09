#pragma once

#include "../MenuUI/MenuUI.h"

class TitleUI : public MenuUI
{
public:
	void Init() override;
	void Update() override;
	void DrawSprite() override;

protected:
	// フェードが終わったらローディングシーンへ
	void OnFadeOutFinished() override;

private:
	// ボタン
	enum ButtonID { BtnStart, BtnHowTo, BtnExit, BtnMax };
	MenuButton m_buttons[BtnMax];

	float m_animTime = 0.0f;
	float m_scaleOffset = 0.0f;

	std::shared_ptr<KdTexture> m_titleLogoTex = nullptr;

	// 遊び方
	int  m_howToPage = 0;
	bool m_showHowTo = false;

	// 操作説明（アイコン＋説明文）
	struct ControlInfo
	{
		std::vector<std::shared_ptr<KdTexture>> icons;
		std::string text;
	};
	std::vector<ControlInfo> m_controls;

	std::shared_ptr<KdSoundInstance> m_bgm;
};
