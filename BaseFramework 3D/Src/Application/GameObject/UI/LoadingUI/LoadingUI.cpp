#include "LoadingUI.h"

void LoadingUI::Init()
{}

void LoadingUI::Update()
{
}

void LoadingUI::DrawSprite()
{
	KdShaderManager::Instance().m_spriteShader.DrawBox(
		0, 0, 640, 360, &kBlackColor);

	// ファイル名だけ表示
	std::string fileName;

	if (!m_currentPath.empty())
	{
		fileName =
			std::filesystem::path(m_currentPath)
			.filename()
			.string();
	}

	std::string text;

	if (m_complete)
	{
		text = "Loading Complete";
	}
	else
	{
		std::string fileName;

		if (!m_currentPath.empty())
		{
			fileName =
				std::filesystem::path(m_currentPath)
				.filename()
				.string();
		}

		text = "Loading: " + fileName;
	}

	text = "Loading";

	// 現在ロード中のパス
	KdShaderManager::Instance().m_spriteShader.DrawFont(
		Math::Vector2{ -500, -250 },
		&kWhiteColor,
		Math::Vector2{ 0.0f, 0.5f },
		FontID::Future24,
		text.c_str());

	constexpr int width = 500;
	constexpr int height = 6;

	constexpr int x = 0;
	constexpr int y = -300;

	Math::Color color;

	// 背景
	color = { 0.5f, 0.5f, 0.5f, 0.5f };
	KdShaderManager::Instance().m_spriteShader.DrawBox(
		x,
		y,
		width,
		height,
		&color);

	// 前景
	float progress = std::clamp(m_progress, 0.0f, 1.0f);

	int currentWidth = static_cast<int>(width * progress);

	if (currentWidth <= 0)
		return;

	color = { 1, 1, 1, 1 };
	KdShaderManager::Instance().m_spriteShader.DrawBox(
		x - width + currentWidth,
		y,
		currentWidth,
		height,
		&color);
}
