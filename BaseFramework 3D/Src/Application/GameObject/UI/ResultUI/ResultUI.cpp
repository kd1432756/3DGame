#include "ResultUI.h"

#include "ResultFadeUI.h"

#include "../../../Scene/GameScene/GameScene.h"

#include "../../../Scene/SceneManager.h"

#include "../../Character/Player/Player.h"

#include "../ConfettiUI/ConfettiUI.h"

namespace
{
	Math::Color RankColor(ScoreManager::Rank rank)
	{
		switch (rank)
		{
		case ScoreManager::Rank::S: return { 1.0f, 0.84f, 0.0f, 1.0f };	// 金
		case ScoreManager::Rank::A: return { 0.4f, 0.9f, 1.0f, 1.0f };	// 水色
		case ScoreManager::Rank::B: return { 0.4f, 0.9f, 0.4f, 1.0f };	// 緑
		default:                    return { 0.8f, 0.8f, 0.8f, 1.0f };	// 灰
		}
	}

	const char* RankMessage(ScoreManager::Rank rank)
	{
		switch (rank)
		{
		case ScoreManager::Rank::S: return "Legendary Fixer!";
		case ScoreManager::Rank::A: return "Great Job!";
		case ScoreManager::Rank::B: return "Not Bad";
		default:                    return "Keep Trying";
		}
	}
}

void ResultUI::Init()
{
	m_updateGroup = UpdateGroup::Always;

	m_visible = false;
	m_resultInitialized = false;

	m_phase = Phase::ScoreCount;
	m_phaseTimer = 0.0f;

	m_finalScore = 0;
	m_displayScore = 0;
	m_rank = ScoreManager::Rank::C;

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

	// 一度だけ結果を取得
	if (!m_resultInitialized)
	{
		ShowResult();

		m_spFadeUI = std::make_shared<ResultFadeUI>();
		m_spFadeUI->Init();
		SceneManager::Instance().AddObject(m_spFadeUI);

		return;
	}

	// Fade中
	if (m_fadeOut)
	{
		m_alpha += 0.02f;

		if (m_resultBgm)
		{
			float volume = 0.3f * (1.0f - m_alpha);
			if (volume < 0.0f) volume = 0.0f;
			m_resultBgm->SetVolume(volume);
		}

		if (m_alpha >= 1.0f)
		{
			m_alpha = 1.0f;

			SceneManager::Instance().SetNextScene(
				SceneManager::SceneType::Title
			);
		}

		if (m_spFadeUI)
		{
			m_spFadeUI->SetAlpha(m_alpha);
		}

		return;
	}

	const float dt = 1.0f / 60.0f;

	m_phaseTimer += dt;

	switch (m_phase)
	{
	case Phase::ScoreCount:
	{
		float t = std::clamp(m_phaseTimer / kCountTime, 0.0f, 1.0f);

		// EaseOut: 最初は速く、最後にゆっくり止まる
		const float ease = 1.0f - std::pow(1.0f - t, 3.0f);

		m_displayScore = static_cast<int>(m_finalScore * ease);

		if (t >= 1.0f)
		{
			m_displayScore = m_finalScore;
			ChangePhase(Phase::RankWait);
		}

		break;
	}

	case Phase::RankWait:

		if (m_phaseTimer >= kRankWaitTime)
		{
			ChangePhase(Phase::RankReveal);

			if (auto se = KdAudioManager::Instance().Play("Asset/Sound/SE/Count.wav"))
			{
				se->SetVolume(0.2f);
			}
		}

		break;

	case Phase::RankReveal:

		if (m_phaseTimer >= kRankRevealTime)
		{
			ChangePhase(Phase::Done);
			StartCelebration();
		}

		break;

	case Phase::Done:
		break;
	}

	if (m_phase != Phase::Done)
	{
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
		m_mousePosX >= left &&
		m_mousePosX <= right &&
		m_mousePosY <= top &&
		m_mousePosY >= bottom;

	// クリック
	if (m_isTitleButtonHover &&
		(GetAsyncKeyState(VK_LBUTTON) & 0x8000))
	{
		m_fadeOut = true;

		if (auto confetti = m_wpConfetti.lock())
		{
			confetti->Stop();
		}
	}
}

void ResultUI::DrawSprite()
{
	if (!m_visible)
	{
		return;
	}

	auto& sprite = KdShaderManager::Instance().m_spriteShader;

	// =============================
	// 背景
	// =============================

	Math::Color bgColor = { 0.0f, 0.0f, 0.0f, 0.8f };

	sprite.DrawBox(0, 0, 640, 360, &bgColor);

	// =============================
	// RESULT
	// =============================

	sprite.DrawFont(
		Math::Vector2{ 0, 260 },
		&kWhiteColor,
		Math::Vector2{ 0.5f, 0.5f },
		0.0f,
		FontID::Future128,
		"RESULT"
	);

	// =============================
	// スコア
	// =============================

	sprite.DrawFont(
		Math::Vector2{ 0, 150 },
		&kWhiteColor,
		Math::Vector2{ 0.5f, 0.5f },
		0.0f,
		FontID::Future48,
		"SCORE"
	);

	const std::string scoreText = std::to_string(m_displayScore);

	sprite.DrawFont(
		Math::Vector2{ 0, 80 },
		&kWhiteColor,
		Math::Vector2{ 0.5f, 0.5f },
		0.0f,
		FontID::Future128,
		scoreText.c_str()
	);

	// =============================
	// ランク
	// =============================

	if (m_phase == Phase::RankReveal || m_phase == Phase::Done)
	{
		float t = 1.0f;

		if (m_phase == Phase::RankReveal)
		{
			t = std::clamp(m_phaseTimer / kRankRevealTime, 0.0f, 1.0f);
		}

		// EaseOut: 上から落ちて、フェードインする
		const float ease = 1.0f - std::pow(1.0f - t, 3.0f);

		Math::Color rankColor = RankColor(m_rank);
		rankColor.w *= ease;

		const float rankY = -80.0f + (1.0f - ease) * 60.0f;

		sprite.DrawFont(
			Math::Vector2{ 0, 0 },
			&kWhiteColor,
			Math::Vector2{ 0.5f, 0.5f },
			0.0f,
			FontID::Future48,
			"RANK"
		);

		sprite.DrawFont(
			Math::Vector2{ 0, rankY },
			&rankColor,
			Math::Vector2{ 0.5f, 0.5f },
			0.0f,
			FontID::Future128,
			ScoreManager::RankToString(m_rank)
		);

		if (m_phase == Phase::Done)
		{
			sprite.DrawFont(
				Math::Vector2{ 0, -170 },
				&rankColor,
				Math::Vector2{ 0.5f, 0.5f },
				0.0f,
				FontID::Future32,
				RankMessage(m_rank)
			);
		}
	}

	// =============================
	// Titleに戻るボタン
	// =============================

	if (m_phase == Phase::Done)
	{
		Math::Color buttonColor = kWhiteColor;

		if (m_isTitleButtonHover)
		{
			buttonColor = Math::Color{ 1.0f, 0.8f, 0.2f, 1.0f };
		}

		Math::Vector2 buttonDrawPos = m_titleButtonPos;
		buttonDrawPos.y += m_animOffsetY;

		sprite.DrawFont(
			buttonDrawPos,
			&buttonColor,
			Math::Vector2{ 0.5f, 0.5f },
			0.0f,
			FontID::Future48,
			"Return To Title"
		);
	}

	// =============================
	// Pointer
	// =============================

	if (!m_fadeOut && m_phase == Phase::Done)
	{
		sprite.DrawTex(
			m_pointerTex,
			m_mousePosX,
			m_mousePosY,
			nullptr,
			&kWhiteColor,
			Math::Vector2{ 0.3f, 0.8f }
		);
	}
}

void ResultUI::ShowResult()
{
	auto player = SceneManager::Instance().FindObject<Player>();

	m_finalScore = player ? player->GetScore() : 0;
	m_displayScore = 0;

	m_rank = ScoreManager::Instance().CalcRank(m_finalScore);

	m_visible = true;
	m_resultInitialized = true;

	ChangePhase(Phase::ScoreCount);
}

void ResultUI::StartCelebration()
{
	const bool celebrate =
		m_rank == ScoreManager::Rank::S ||
		m_rank == ScoreManager::Rank::A;

	if (celebrate)
	{
		m_resultBgm = KdAudioManager::Instance().Play("Asset/Sound/BGM/WinBGM.wav", true);

		if (auto confetti = m_wpConfetti.lock())
		{
			confetti->Start();
		}

		if (auto se = KdAudioManager::Instance().Play("Asset/Sound/SE/PartyPopper.wav"))
		{
			se->SetVolume(0.2f);
		}
	}
	else
	{
		m_resultBgm = KdAudioManager::Instance().Play("Asset/Sound/BGM/ResultBGM.wav", true);
	}

	if (m_resultBgm)
	{
		m_resultBgm->SetVolume(0.3f);
	}
}

void ResultUI::ChangePhase(Phase phase)
{
	m_phase = phase;
	m_phaseTimer = 0.0f;
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