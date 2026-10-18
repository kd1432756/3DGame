#pragma once

class TitleUI : public KdGameObject
{
public:
	void Init() override;
	void Update() override;
	void DrawSprite() override;

	ObjectCategory GetCategory() const override
	{
		return ObjectCategory::UI;
	}

private:
	void GetMousePos(POINT* mousePos);

	bool m_start = false;
	float m_alpha = 0.0f;

	// ボタン
	struct Button
	{
		Math::Vector2 pos;
		Math::Vector2 size;
		const char* text;
		FontID        font;
		bool          hover = false;
	};
	enum ButtonID { BtnStart, BtnHowTo, BtnExit, BtnMax };
	Button m_buttons[BtnMax];

	bool m_isHover = false;

	Math::Vector2 m_startPos = { 0.0f, -250.0f };
	Math::Vector2 m_startSize = { 300.0f, 80.0f };

	float m_animTime = 0.0f;
	float m_animOffsetY = 0.0f;

	float m_mousePosX = 0.0f;
	float m_mousePosY = 0.0f;

	std::shared_ptr<KdTexture> m_pointerTex = nullptr;

	// 遊び方
	int m_howToPage = 0;
	bool m_showHowTo = false;
	bool m_isHoverHowTo = false;
	bool m_prevClick = false;
	Math::Vector2 m_howToPos = { 0.0f, -80.0f };   // 位置は好みで調整
	Math::Vector2 m_howToSize = { 300.0f, 60.0f };

	// 操作説明（アイコン＋説明文）
	struct ControlInfo
	{
		std::vector<std::shared_ptr<KdTexture>> icons;
		std::string text;
	};
	std::vector<ControlInfo> m_controls;
};