#include "TitleUI.h"

#include "../../../Scene/SceneManager.h"
#include "../../../Scene/CameraManager/CameraManager.h"

void TitleUI::Init()
{
	m_pointerTex = std::make_shared<KdTexture>();
	m_pointerTex->Load("Asset/Textures/Cursor/Pointer.png");
}

void TitleUI::Update()
{
	if (m_start)
	{
		m_alpha += 0.02f;

		if (m_alpha > 1.0f)
		{
			SceneManager::Instance().SetNextScene(SceneManager::SceneType::Loading);
		}

		return;
	}

	m_animTime += 0.05f;

	m_animOffsetY = sinf(m_animTime) * 5.0f;

	POINT mouse;
	GetMousePos(&mouse);

	m_mousePosX = static_cast<float>(mouse.x);
	m_mousePosY = static_cast<float>(mouse.y);

	float left = m_startPos.x - m_startSize.x * 0.5f;
	float right = m_startPos.x + m_startSize.x * 0.5f;
	float top = m_startPos.y - m_startSize.y * 0.5f;
	float bottom = m_startPos.y + m_startSize.y * 0.5f;

	m_isHover =
		m_mousePosX >= left &&
		m_mousePosX <= right &&
		m_mousePosY >= top &&
		m_mousePosY <= bottom;

	if (GetAsyncKeyState('Q') & 0x8000)
	{
		m_start = true;
		SceneManager::Instance().SetStage(SceneManager::StageType::Stage1);
	}
	if (GetAsyncKeyState(VK_RETURN) & 0x8000)
	{
		m_start = true;
		SceneManager::Instance().SetStage(SceneManager::StageType::Stage2);
	}

	if (GetAsyncKeyState(VK_SPACE) & 0x8000)
	{
		m_start = true;
		SceneManager::Instance().SetStage(SceneManager::StageType::Stage2);
	}

	if (m_isHover &&
		(GetAsyncKeyState(VK_LBUTTON) & 0x8000))
	{
		m_start = true;
		SceneManager::Instance().SetStage(SceneManager::StageType::Stage2);
	}
}

void TitleUI::DrawSprite()
{
	Math::Vector2 titlePos = { -150, 200 };
	Math::Vector2 pivot = { 0.5f, 0.5f };

	const float outlineSize = 3.0f;

	Math::Color titleColor = { 1.0f, 0.8f, 0.2f, 1.0f };

	const Math::Vector2 offsets[] =
	{
		{-outlineSize, -outlineSize},
		{ 0.0f,       -outlineSize},
		{ outlineSize, -outlineSize},

		{-outlineSize,  0.0f},
		{ outlineSize,  0.0f},

		{-outlineSize,  outlineSize},
		{ 0.0f,         outlineSize},
		{ outlineSize,  outlineSize}
	};

	// 黒縁を描画
	for (const auto& offset : offsets)
	{
		KdShaderManager::Instance().m_spriteShader.DrawFont(
			titlePos + offset,
			&kBlackColor,
			pivot,
			0.0f,
			FontID::Gothic96,
			"ご近所いたずら大戦"
		);
	}

	KdShaderManager::Instance().m_spriteShader.DrawFont(
		titlePos,
		&titleColor,
		pivot,
		0.0f,
		FontID::Gothic96,
		"ご近所いたずら大戦"
	);

	Math::Color color = kWhiteColor;

	if (m_isHover)
	{
		color = Math::Color{ 1.0f, 0.8f, 0.2f, 1.0f };
	}

	Math::Vector2 drawPos = m_startPos;
	drawPos.y += m_animOffsetY;

	KdShaderManager::Instance().m_spriteShader.DrawFont(
		drawPos,
		&color,
		Math::Vector2{ 0.5f, 0.5f },
		0.0f,
		FontID::FutureNormal64,
		"Start"
	);

	KdShaderManager::Instance().m_spriteShader.DrawTex(
		m_pointerTex,
		m_mousePosX,
		m_mousePosY,
		nullptr,
		&kWhiteColor,
		Math::Vector2{ 0.3f,0.8f }
	);

	if (!m_start)
	{
		return;
	}

	Math::Color fadeColor =
	{ 0.0f, 0.0f, 0.0f, m_alpha };

	KdShaderManager::Instance().m_spriteShader.DrawBox(
		0,
		0,
		640,
		360,
		&fadeColor
	);
}

void TitleUI::GetMousePos(POINT* mousePos)
{	
	//ディスプレイ上のマウス座標を取得(PC画面の左上が(0,0))
	GetCursorPos(mousePos);

	//指定のウィンドウ基準のマウス座標に変換(実行画面の左上が(0,0))
	//							↓実行ウィンドウを識別するための番号
	ScreenToClient(Application::Instance().GetWindowHandle(), mousePos);

	static const int ScrWidth = 1280;
	static const int ScrHeight = 720;

	mousePos->x -= ScrWidth / 2;
	mousePos->y -= ScrHeight / 2;
	mousePos->y *= -1;
}
