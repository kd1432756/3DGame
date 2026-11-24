#include "GameStateUI.h"

#include "../../../Scene/GameScene/GameScene.h"

void GameStateUI::Init()
{
	m_timer = 0.0f;
	m_count = 3;
	m_displayState = DisplayState::WaitClick;
}

void GameStateUI::Update()
{
	auto gameScene = m_wpGameScene.lock();

	if (!gameScene)
	{
		return;
	}

	m_timer += DeltaTime();

	switch (m_displayState)
	{
	case DisplayState::WaitClick:

		if (GetAsyncKeyState(VK_LBUTTON) & 0x0001)
		{
			m_displayState = DisplayState::CountDown;
			m_timer = 0.0f;
			m_count = 3;
		}

		break;
	case DisplayState::CountDown:

		if (m_timer >= 1.0f)
		{
			m_timer = 0.0f;

			m_count--;

			if (m_count > 0)
			{
				if (auto se = KdAudioManager::Instance().Play("Asset/Sound/SE/Count.wav"))
				{
					se->SetVolume(0.2f);
				}
			}

			if (m_count <= 0)
			{
				m_displayState = DisplayState::Start;

				// ゲーム開始
				gameScene->SetGameUpdate(true);
				gameScene->SetGameState(GameScene::GameState::Playing);

				if (auto se = KdAudioManager::Instance().Play("Asset/Sound/SE/Start.wav"))
				{
					se->SetVolume(0.2f);
				}

				m_bgm = KdAudioManager::Instance().Play("Asset/Sound/BGM/GameBGM.wav", true);
				if (m_bgm)
				{
					m_bgm->SetVolume(0.12f);
				}
			}
		}

		break;

	case DisplayState::Start:

		if (m_timer >= 1.0f)
		{
			m_timer = 0.0f;
			m_displayState = DisplayState::None;
		}

		break;

	case DisplayState::Finish:

		if (m_bgm)
		{
			float volume = 0.2f * (1.0f - (m_timer / 2.0f));
			if (volume < 0.0f) volume = 0.0f;
			m_bgm->SetVolume(volume);
		}

		if (m_timer >= 2.0f)
		{
			m_timer = 0.0f;

			gameScene->SetGameState(
				GameScene::GameState::Result
			);

			m_displayState = DisplayState::None;
		}

		break;

	case DisplayState::None:

		break;
	}
}

void GameStateUI::DrawSprite()
{
	std::string text;
	FontID font = FontID::Future128;
	Math::Vector2 pos = {};

	switch (m_displayState)
	{
	case DisplayState::WaitClick:

		text = "Click to Start";
		font = FontID::Future48;
		pos = { 0,-60 };

		break;

	case DisplayState::CountDown:

		text = std::to_string(m_count);

		break;

	case DisplayState::Start:

		text = "START!";

		break;

	case DisplayState::Finish:

		text = "FINISH!";

		break;

	case DisplayState::None:

		return;
	}

	// 画面を少し暗く
	DrawFullScreenBox(Math::Color{ 0.0f, 0.0f, 0.0f, 0.5f });

	DrawLabel(pos, kWhiteColor, font, text.c_str());
}
