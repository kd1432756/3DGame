#include "TitleUI.h"

#include "../../../Scene/SceneManager.h"
#include "../../../Scene/CameraManager/CameraManager.h"

void TitleUI::Init()
{
	MenuUI::Init();

	m_titleLogoTex = LoadTexture("Asset/Textures/Title_Logo.png");

	m_buttons[BtnStart] = { Math::Vector2{ 0.0f,  -80.0f }, Math::Vector2{ 420.0f, 80.0f }, "Play Game",  FontID::FutureNormal64 };
	m_buttons[BtnHowTo] = { Math::Vector2{ 0.0f, -180.0f }, Math::Vector2{ 480.0f, 60.0f }, "How To Play", FontID::FutureNormal64 };
	m_buttons[BtnExit] = { Math::Vector2{ 0.0f, -280.0f }, Math::Vector2{ 400.0f, 60.0f }, "Quit Game",   FontID::FutureNormal64 };

	auto load = [](const char* name)
		{
			return LoadTexture(std::string("Asset/Textures/UI/Key/") + name);
		};

	// 操作説明
	m_controls =
	{
		{ { load("keyboard_w.png"), load("keyboard_a.png"), load("keyboard_s.png"), load("keyboard_d.png") }, "移動" },
		{ { load("mouse_outline.png") }, "視点" },
		{ { load("keyboard_1.png"), load("keyboard_2.png"), load("keyboard_3.png") }, "アイテム選択" },
		{ { load("keyboard_e.png") }, "アイテムを拾う" },
		{ { load("keyboard_g.png") }, "アイテムを捨てる" },
		{ { load("keyboard_f.png") },	 "妨害" },
		{ { load("keyboard_r.png") },	 "警察に通報" },
		{ { load("mouse_left_outline.png") }, "修復・スキルチェック" },
		{ { load("mouse_right_outline.png") }, "中断" },
		{ { load("keyboard_w.png"), load("keyboard_a.png"), load("keyboard_s.png"), load("keyboard_d.png") }, "スキルチェック（方向入力）" },
	};

	m_bgm = KdAudioManager::Instance().Play("Asset/Sound/BGM/TitleBGM.wav", true);
	if (m_bgm)
	{
		m_bgm->SetVolume(0.3f);

		SetFadeBgm(m_bgm, 0.3f);
	}
}

void TitleUI::OnFadeOutFinished()
{
	// ロードを始める
	SceneManager::Instance().SetNextScene(SceneManager::SceneType::Loading);
}

void TitleUI::Update()
{
	if (IsFadingOut())
	{
		UpdateFade();
		return;
	}

	m_animTime += 0.05f;
	m_scaleOffset = sinf(m_animTime * 0.5f) * 5.0f;

	UpdateMouse();

	if (m_showHowTo)
	{
		if (IsClickTriggered())
		{
			m_howToPage++;
			if (m_howToPage >= 2)
			{
				m_howToPage = 0;
				m_showHowTo = false;
			}
		}
		return;
	}

	UpdateHover(m_buttons);

	const bool trigger = IsClickTriggered();

	if (trigger && m_buttons[BtnStart].hover)
	{
		SceneManager::Instance().SetStage(SceneManager::StageType::Stage1);
		StartFadeOut();
		return;
	}

	if (trigger && m_buttons[BtnHowTo].hover)
	{
		m_showHowTo = true;
		return;
	}

	if (trigger && m_buttons[BtnExit].hover)
	{
		Application::Instance().End();
		return;
	}
}

void TitleUI::DrawSprite()
{
	auto& sprite = KdShaderManager::Instance().m_spriteShader;

	// ロゴ
	const Math::Vector2 titlePos = { 0, 160 };
	const Math::Vector2 logoTexSize = { 1534, 1024 };
	const float logoScale = 0.35f * (1 + m_scaleOffset * 0.005f);
	const Math::Vector2 logoSize = logoTexSize * logoScale;

	sprite.DrawTex(
		m_titleLogoTex,
		titlePos.x, titlePos.y,
		logoSize.x, logoSize.y,
		nullptr, &kWhiteColor, Math::Vector2{ 0.5f, 0.5f });

	// ボタン
	DrawButtons(m_buttons);

	if (m_showHowTo)
	{
		// 背景を暗く
		DrawFullScreenBox(Math::Color{ 0.0f, 0.0f, 0.0f, 0.75f });

		// 遊び方
		Math::Color panelColor = { 0.15f, 0.15f, 0.2f, 0.95f };
		sprite.DrawBox(0, 0, 520, 300, &panelColor);

		DrawLabel(Math::Vector2{ 0.0f, 250.0f },
			Math::Color{ 1.0f, 0.8f, 0.2f, 1.0f },
			FontID::Gothic64,
			(m_howToPage == 0) ? "遊び方" : "操作方法");

		if (m_howToPage == 0)
		{
			// ---- 1ページ目：ルール ----
			static const char* rules[] =
			{
				"朝から夜までの約5分間、街の壊れた物を直してスコアを稼ごう！",
				"1. 中央エリアでアイテムを拾おう（最大3個まで持てる）",
				"2. 壊された物を見つけて、合うアイテムで修復しよう",
				"3. 壊している最中のご近所さんは、妨害して足止めできる",
				"4. 修復の成功でスコア獲得！ スキルチェックの成績で変わる",
				"5. 壊されたばかりの物を直すところを警察に見られると、",
				"    疑われて交番へ連行。危険なアイテムも没収！",
				"6. 通報で警察を呼んで、犯人を捕まえてもらおう",
				"※ 修復中はミニゲーム（スキルチェック）が発生"
			};

			float y = 160.0f;
			for (const char* line : rules)
			{
				DrawLabel(Math::Vector2{ -480.0f, y }, kWhiteColor,
					FontID::Gothic32, line, Math::Vector2{ 0.0f, 0.5f });
				y -= 42.0f;
			}
		}
		else
		{
			// ---- 2ページ目：操作方法 ----
			const float iconSize = 38.0f;
			const float iconStep = 42.0f;
			float y = 160.0f;

			for (const auto& info : m_controls)
			{
				float x = -440.0f;
				for (const auto& icon : info.icons)
				{
					sprite.DrawTex(
						icon,
						static_cast<int>(x), static_cast<int>(y),
						static_cast<int>(iconSize), static_cast<int>(iconSize),
						nullptr, &kWhiteColor, Math::Vector2{ 0.0f, 0.5f });
					x += iconStep;
				}

				DrawLabel(Math::Vector2{ -210.0f, y }, kWhiteColor,
					FontID::Gothic32, info.text.c_str(), Math::Vector2{ 0.0f, 0.5f });

				y -= 42.0f;
			}
		}

		DrawLabel(Math::Vector2{ 0.0f, -270.0f }, kWhiteColor,
			FontID::Gothic32,
			(m_howToPage == 0) ? "クリックで次へ" : "クリックで閉じる");
	}

	DrawPointer();

	// フェードアウト
	DrawFadeOverlay();
}
