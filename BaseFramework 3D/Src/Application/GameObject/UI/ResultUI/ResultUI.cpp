#include "ResultUI.h"

#include "ResultFadeUI.h"

#include "../../../Scene/GameScene/GameScene.h"

#include "../../../Scene/SceneManager.h"

#include "../../../System/Score/ScoreManager.h"

#include "../../Character/CharacterBase.h"
#include "../../Character/Player/Player.h"

#include "../ConfettiUI/ConfettiUI.h"

void ResultUI::Init()
{
	m_updateGroup = UpdateGroup::Always;

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

	for (auto& t : m_rowAnimTime) t = 0.0f;
	m_prevDisplayRankCount = 0;

	m_confettiWaitTimer = 0.0f;
	m_confettiStarted = false;

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
			// m_alphaが0.0から1.0に向かうの連動させて音量を下げる（初期音量が0.3fの場合）
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

	// ランキング表示
	if (m_displayRankCount < static_cast<int>(m_ranking.size()))
	{
		m_timer += 1.0f / 60.0f;

		if (m_timer >= 0.5f)
		{
			m_timer = 0.0f;
			m_displayRankCount++;

			if (auto se = KdAudioManager::Instance().Play("Asset/Sound/SE/Count.wav"))
			{
				se->SetVolume(0.2f);
			}
		}

		for (int i = 0; i < m_displayRankCount && i < 4; ++i)
		{
			m_rowAnimTime[i] += 1.0f / 60.0f;
		}

		return;
	}

	for (int i = 0; i < static_cast<int>(m_ranking.size()) && i < 4; ++i)
	{
		m_rowAnimTime[i] += 1.0f / 60.0f;
	}

	if (!m_confettiStarted)
	{
		m_confettiWaitTimer += 1.0f / 60.0f;

		if (m_confettiWaitTimer >= 0.7f)
		{
			m_confettiStarted = true;
		}
	}

	// 全ての結果が表示された瞬間に一度だけ紙吹雪とBGMを再生
	if (!m_confettiStarted)
	{
		m_confettiWaitTimer += 1.0f / 60.0f;

		// 1位のスライド(0.4秒)＋余韻(0.3秒)
		if (m_confettiWaitTimer >= 0.7f)
		{
			m_confettiStarted = true;

			if (!m_ranking.empty() && m_ranking[0])
			{
				bool isPlayerFirst = std::dynamic_pointer_cast<Player>(m_ranking[0]) != nullptr;

				if (isPlayerFirst)
				{
					// 1位（プレイヤー勝利）用のBGM ＆ 紙吹雪
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
		}
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
		0.0f,
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
		std::string nameText = chara->GetDisplayNameOrName();

		// 名前の色（Player と NPC で変える）
		const bool isPlayer =
			std::dynamic_pointer_cast<Player>(chara) != nullptr;

		const Math::Color nameColor = isPlayer
			? Math::Color{ 0.4f, 0.9f, 1.0f, 1.0f }   // Player：水色
		: kWhiteColor;                            // NPC：白

		// 順位の色（順位ごとに変える）
		Math::Color rankColor = kWhiteColor;

		switch (rankingIndex)
		{
		case 0: rankColor = { 1.0f, 0.84f, 0.0f, 1.0f };  break;  // 1位：金
		case 1: rankColor = { 0.78f, 0.78f, 0.82f, 1.0f }; break;  // 2位：銀
		case 2: rankColor = { 0.85f, 0.55f, 0.25f, 1.0f }; break;  // 3位：銅
		default: break;                                            // 4位：白
		}

		// スコア
		std::string scoreText =
			std::to_string(chara->GetScore()) + " pt";

		const float RankingBottomY = -160.0f;
		const float RankingHeight = 100.0f;

		const float y =
			RankingBottomY +
			(static_cast<int>(m_ranking.size()) - 1 - rankingIndex)
			* RankingHeight;

		const float kSlideDuration = 0.4f;    // スライドにかかる秒数
		const float kSlideDistance = 700.0f;  // 画面外からの距離

		float t = m_rowAnimTime[i] / kSlideDuration;
		t = std::clamp(t, 0.0f, 1.0f);

		// EaseOutCubic：最初は速く、最後にゆっくり止まる
		const float ease = 1.0f - powf(1.0f - t, 3.0f);

		const float offsetX = -kSlideDistance * (1.0f - ease);

		// フェードイン（スライドと同時に濃くなる）
		const float alpha = ease;

		// ---- 行背景 ----
		Math::Color rowColor = isPlayer
			? Math::Color{ 0.4f, 0.9f, 1.0f, 0.25f }   // Player：水色の半透明
		: Math::Color{ 1.0f, 1.0f, 1.0f, 0.08f };  // NPC：うっすら白

		// 色にアルファを反映（元の色は変えず、コピーして使う）
		Math::Color rowDrawColor = rowColor;   rowDrawColor.w *= alpha;
		Math::Color rankDrawColor = rankColor;  rankDrawColor.w *= alpha;
		Math::Color nameDrawColor = nameColor;  nameDrawColor.w *= alpha;

		// ---- 行背景 ----
		KdShaderManager::Instance().m_spriteShader.DrawBox(
			static_cast<int>(0 + offsetX),
			static_cast<int>(y),
			400,
			40,
			&rowDrawColor,
			true
		);

		// ---- 順位 ----
		KdShaderManager::Instance().m_spriteShader.DrawFont(
			Math::Vector2{ -360 + offsetX, y },
			&rankDrawColor,
			Math::Vector2{ 0.0f, 0.5f },
			0.0f,
			FontID::Future64,
			rankText.c_str()
		);

		// ---- 名前 ----
		KdShaderManager::Instance().m_spriteShader.DrawFont(
			Math::Vector2{ -180 + offsetX, y },
			&nameDrawColor,
			Math::Vector2{ 0.0f, 0.5f },
			0.0f,
			FontID::Future48,
			nameText.c_str()
		);

		// ---- スコア ----
		KdShaderManager::Instance().m_spriteShader.DrawFont(
			Math::Vector2{ 360 + offsetX, y },
			&nameDrawColor,
			Math::Vector2{ 1.0f, 0.5f },
			0.0f,
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
			0.0f,
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
}

void ResultUI::ShowResult()
{
	m_ranking = ScoreManager::Instance().GetRanking();

	m_visible = true;
	m_resultInitialized = true;

	m_timer = 0.0f;
	m_displayRankCount = 0;

	for (auto& t : m_rowAnimTime) t = 0.0f;
	m_prevDisplayRankCount = 0;

	m_confettiWaitTimer = 0.0f;
	m_confettiStarted = false;
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
