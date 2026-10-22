#include "GameStateUI.h"

#include "../../../Scene/GameScene/GameScene.h"

void GameStateUI::Init()
{
	m_updateGroup = UpdateGroup::Always;

	m_timer = 0.0f;
	m_count = 3;
	m_displayState = DisplayState::CountDown;

/*	if (auto se = KdAudioManager::Instance().Play("Asset/Sound/SE/Count.wav"))
	{
		se->SetVolume(0.2f);
	}*/
}

void GameStateUI::Update()
{
	auto gameScene = m_wpGameScene.lock();

	if (!gameScene)
	{
		return;
	}

	m_timer += 1.0f / 60.0f /*Application::Instance().GetDeltaTime()*/;

	switch (m_displayState)
	{
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
				// START! 表示へ
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

		// START!を1秒表示
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

	switch (m_displayState)
	{
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

	Math::Color color = { 0.0f,0.0f,0.0f,0.5f };
	KdShaderManager::Instance().m_spriteShader.DrawBox(
		0, 0, 640, 360, &color);

	KdShaderManager::Instance().m_spriteShader.DrawFont(
		Math::Vector2{ 0, 0 },
		&kWhiteColor,
		Math::Vector2{ 0.5f, 0.5f },
		0.0f,
		FontID::Future128,
		text.c_str());
}
