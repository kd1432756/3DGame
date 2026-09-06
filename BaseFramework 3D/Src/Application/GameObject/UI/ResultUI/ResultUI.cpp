#include "ResultUI.h"

#include "../../../Scene/GameScene/GameScene.h"

#include "../../../Scene/SceneManager.h"

#include "../../../System/Score/ScoreManager.h"

#include "../../Character/CharacterBase.h"

void ResultUI::Init()
{
	m_ranking.clear();

	m_visible = false;
	m_resultInitialized = false;

	m_timer = 0.0f;
	m_displayRankCount = 0;

	m_isTitleButtonHover = false;

	m_fadeOut = false;
	m_alpha = 0.0f;

	m_animTime = 0.0f;
	m_animOffsetY = 0.0f;

	m_pointerTex = std::make_shared<KdTexture>();
	m_pointerTex->Load("Asset/Textures/Cursor/Pointer.png");
}

void ResultUI::Update()
{
	auto gameScene = m_wpGameScene.lock();

	if (!gameScene)
	{
		return;
	}

	// Result状態になるまで何もしない
	if (gameScene->GetGameState() != GameScene::GameState::Result)
	{
		return;
	}

	// 一度だけランキングを取得
	if (!m_resultInitialized)
	{
		ShowResult();
		return;
	}

	// Fade中
	if (m_fadeOut)
	{
		m_alpha += 0.02f;

		if (m_alpha >= 1.0f)
		{
			m_alpha = 1.0f;

			SceneManager::Instance().SetNextScene(
				SceneManager::SceneType::Title
			);
		}

		return;
	}

	// ランキング表示
	if (m_displayRankCount < static_cast<int>(m_ranking.size()))
	{
		m_timer += Application::Instance().GetDeltaTime();

		if (m_timer >= 0.5f)
		{
			m_timer = 0.0f;
			m_displayRankCount++;
		}

		return;
	}

	m_animTime += 0.05f;

	m_animOffsetY = sinf(m_animTime) * 5.0f;

	// Titleボタン
	POINT mouse;
	GetMousePos(&mouse);

	m_mousePosX = static_cast<float>(mouse.x);
	m_mousePosY = static_cast<float>(mouse.y);

	const float left =
		m_titleButtonPos.x - m_titleButtonSize.x * 0.5f;

	const float right =
		m_titleButtonPos.x + m_titleButtonSize.x * 0.5f;

	const float top =
		m_titleButtonPos.y + m_titleButtonSize.y * 0.5f;

	const float bottom =
		m_titleButtonPos.y - m_titleButtonSize.y * 0.5f;

	m_isTitleButtonHover =
		static_cast<float>(mouse.x) >= left &&
		static_cast<float>(mouse.x) <= right &&
		static_cast<float>(mouse.y) <= top &&
		static_cast<float>(mouse.y) >= bottom;

	// クリック
	if (m_isTitleButtonHover &&
		(GetAsyncKeyState(VK_LBUTTON) & 0x8000))
	{
		m_fadeOut = true;
	}

}

void ResultUI::DrawSprite()
{
	if (!m_visible)
	{
		return;
	}

	// =============================
	// 背景
	// =============================

	Math::Color bgColor = { 0.0f, 0.0f, 0.0f, 0.8f };

	KdShaderManager::Instance().m_spriteShader.DrawBox(
		0,
		0,
		640,
		360,
		&bgColor
	);

	// =============================
	// RESULT
	// =============================

	KdShaderManager::Instance().m_spriteShader.DrawFont(
		Math::Vector2{ 0, 260 },
		&kWhiteColor,
		Math::Vector2{ 0.5f, 0.5f },
		FontID::Future128,
		"RESULT"
	);

	// =============================
	// ランキング
	// =============================

	for (int i = 0; i < m_displayRankCount; ++i)
	{
		int rankingIndex =
			static_cast<int>(m_ranking.size()) - 1 - i;

		auto& chara = m_ranking[rankingIndex];

		if (!chara)
		{
			continue;
		}

		// 順位
		std::string rankText;

		switch (rankingIndex)
		{
		case 0:
			rankText = "1st";
			break;

		case 1:
			rankText = "2nd";
			break;

		case 2:
			rankText = "3rd";
			break;

		default:
			rankText = std::to_string(rankingIndex + 1) + "th";
			break;
		}

		// 名前
		std::string nameText = chara->GetName();

		// スコア
		std::string scoreText =
			std::to_string(chara->GetScore()) + " pt";

		const float RankingBottomY = -140.0f;
		const float RankingHeight = 100.0f;

		const float y =
			RankingBottomY +
			(static_cast<int>(m_ranking.size()) - 1 - rankingIndex)
			* RankingHeight;

		// 順位
		KdShaderManager::Instance().m_spriteShader.DrawFont(
			Math::Vector2{ -250, y },
			&kWhiteColor,
			Math::Vector2{ 0.0f, 0.5f },
			FontID::Future64,
			rankText.c_str()
		);

		// 名前
		KdShaderManager::Instance().m_spriteShader.DrawFont(
			Math::Vector2{ 250, y },
			&kWhiteColor,
			Math::Vector2{ 1.0f, 0.5f },
			FontID::Future48,
			nameText.c_str()
		);

		// スコア
		KdShaderManager::Instance().m_spriteShader.DrawFont(
			Math::Vector2{ 250, y - 45.0f },
			&kWhiteColor,
			Math::Vector2{ 1.0f, 0.5f },
			FontID::Future48,
			scoreText.c_str()
		);
	}

	// =============================
	// Titleに戻るボタン
	// =============================

	if (m_displayRankCount >= static_cast<int>(m_ranking.size()))
	{
		Math::Color buttonColor = kWhiteColor;

		if (m_isTitleButtonHover)
		{
			buttonColor =
				Math::Color{ 1.0f, 0.8f, 0.2f, 1.0f };
		}

		Math::Vector2 buttonDrawPos = m_titleButtonPos;
		buttonDrawPos.y += m_animOffsetY;

		KdShaderManager::Instance().m_spriteShader.DrawFont(
			buttonDrawPos,
			&buttonColor,
			Math::Vector2{ 0.5f, 0.5f },
			FontID::Future48,
			"Return To Title"
		);
	}


	// =============================
	// Pointer
	// =============================

	if (!m_fadeOut &&
		m_displayRankCount >= static_cast<int>(m_ranking.size()))
	{
		KdShaderManager::Instance().m_spriteShader.DrawTex(
			m_pointerTex,
			m_mousePosX,
			m_mousePosY,
			nullptr,
			&kWhiteColor,
			Math::Vector2{ 0.3f, 0.8f }
		);
	}

	// =============================
	// Fade
	// =============================

	if (m_fadeOut)
	{
		Math::Color fadeColor =
		{
			0.0f,
			0.0f,
			0.0f,
			m_alpha
		};

		KdShaderManager::Instance().m_spriteShader.DrawBox(
			0,
			0,
			640,
			360,
			&fadeColor
		);
	}
}

void ResultUI::ShowResult()
{
	m_ranking = ScoreManager::Instance().GetRanking();

	m_visible = true;
	m_resultInitialized = true;

	m_timer = 0.0f;
	m_displayRankCount = 0;
}

void ResultUI::GetMousePos(POINT* mousePos)
{
	GetCursorPos(mousePos);

	ScreenToClient(
		Application::Instance().GetWindowHandle(),
		mousePos
	);

	static const int ScrWidth = 1280;
	static const int ScrHeight = 720;

	mousePos->x -= ScrWidth / 2;
	mousePos->y -= ScrHeight / 2;

	mousePos->y *= -1;
}
