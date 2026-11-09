#include "ResultUI.h"

#include "../ResultFadeUI/ResultFadeUI.h"

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
	// ポインタ画像の読み込みなど(基底クラス)
	MenuUI::Init();

	m_visible = false;
	m_resultInitialized = false;

	m_phase = Phase::ScoreCount;
	m_phaseTimer = 0.0f;

	m_finalScore = 0;
	m_displayScore = 0;
	m_rank = ScoreManager::Rank::C;

	m_titleButton = { Math::Vector2{ 0.0f, -270.0f }, Math::Vector2{ 470.0f, 60.0f }, "Return To Title", FontID::Future48 };

	m_animTime = 0.0f;
	m_animOffsetY = 0.0f;
}

void ResultUI::OnFadeOutFinished()
{
	SceneManager::Instance().SetNextScene(
		SceneManager::SceneType::Title
	);
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

	// Fade中（暗転は別オブジェクトの ResultFadeUI が描く）
	if (IsFadingOut())
	{
		UpdateFade();

		if (m_spFadeUI)
		{
			m_spFadeUI->SetAlpha(GetFadeAlpha());
		}

		return;
	}

	// 演出中もマウス状態を追っておく
	// (押しっぱなしのままボタンが出たとき、誤クリック扱いにしないため)
	UpdateMouse();

	m_phaseTimer += DeltaTime();

	switch (m_phase)
	{
	case Phase::ScoreCount:
	{
		const float t = std::clamp(m_phaseTimer / kCountTime, 0.0f, 1.0f);

		// EaseOut: 最初は速く、最後にゆっくり止まる
		m_displayScore = static_cast<int>(m_finalScore * EaseOutCubic(t));

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
	UpdateHover(&m_titleButton, 1);

	// クリック
	if (m_titleButton.hover && IsClickTriggered())
	{
		StartFadeOut();

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

	// =============================
	// 背景
	// =============================

	DrawFullScreenBox(Math::Color{ 0.0f, 0.0f, 0.0f, 0.8f });

	// =============================
	// RESULT
	// =============================

	DrawLabel(Math::Vector2{ 0, 260 }, kWhiteColor, FontID::Future128, "RESULT");

	// =============================
	// スコア
	// =============================

	DrawLabel(Math::Vector2{ 0, 150 }, kWhiteColor, FontID::Future48, "SCORE");

	const std::string scoreText = std::to_string(m_displayScore);

	DrawLabel(Math::Vector2{ 0, 80 }, kWhiteColor, FontID::Future128, scoreText.c_str());

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
		const float ease = EaseOutCubic(t);

		Math::Color rankColor = RankColor(m_rank);
		rankColor.w *= ease;

		const float rankY = -80.0f + (1.0f - ease) * 60.0f;

		DrawLabel(Math::Vector2{ 0, 0 }, kWhiteColor, FontID::Future48, "RANK");

		DrawLabel(Math::Vector2{ 0, rankY }, rankColor, FontID::Future128,
			ScoreManager::RankToString(m_rank));

		if (m_phase == Phase::Done)
		{
			DrawLabel(Math::Vector2{ 0, -170 }, rankColor, FontID::Future32,
				RankMessage(m_rank));
		}
	}

	// =============================
	// Titleに戻るボタン
	// =============================

	if (m_phase == Phase::Done)
	{
		// ホバーで黄色、上下にフワフワ揺らす
		DrawButtons(&m_titleButton, 1,
			kWhiteColor,
			Math::Color{ 1.0f, 0.8f, 0.2f, 1.0f },
			m_animOffsetY);
	}

	// =============================
	// Pointer
	// =============================

	if (!IsFadingOut() && m_phase == Phase::Done)
	{
		DrawPointer();
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

		// フェードアウト時はこのBGMを小さくしていく
		SetFadeBgm(m_resultBgm, 0.3f);
	}
}

void ResultUI::ChangePhase(Phase phase)
{
	m_phase = phase;
	m_phaseTimer = 0.0f;
}
