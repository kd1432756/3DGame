#include "TitleUI.h"

#include "../../../Scene/SceneManager.h"
#include "../../../Scene/CameraManager/CameraManager.h"

void TitleUI::Init()
{
	m_pointerTex = std::make_shared<KdTexture>();
	m_pointerTex->Load("Asset/Textures/Cursor/Pointer.png");

	m_buttons[BtnStart] = { Math::Vector2{ 0.0f,  -80.0f }, Math::Vector2{ 500.0f, 80.0f }, "Play Game",  FontID::FutureNormal64 };
	m_buttons[BtnHowTo] = { Math::Vector2{ 0.0f, -180.0f }, Math::Vector2{ 500.0f, 60.0f }, "How To Play", FontID::FutureNormal64 };
	m_buttons[BtnExit] = { Math::Vector2{ 0.0f, -280.0f }, Math::Vector2{ 500.0f, 60.0f }, "Quit Game",   FontID::FutureNormal64 };

	auto load = [](const char* name)
		{
			auto tex = std::make_shared<KdTexture>();
			tex->Load(std::string("Asset/Textures/UI/Key/") + name);
			return tex;
		};

	m_controls =
	{
		{ { load("keyboard_w.png"), load("keyboard_a.png"), load("keyboard_s.png"), load("keyboard_d.png") },	"移動" },
		{ { load("mouse_outline.png") },																		"視点" },
		{ { load("keyboard_1.png"), load("keyboard_2.png"), load("keyboard_3.png") },							"アイテム選択" },
		{ { load("keyboard_e.png") },																			"拾う" },
		{ { load("keyboard_q.png") },																			"捨てる" },
		{ { load("mouse_left_outline.png") },																	"いたずら・修復・スキルチェック" },
		{ { load("mouse_right_outline.png") },																	"中断" },
		{ { load("keyboard_w.png"), load("keyboard_a.png"), load("keyboard_s.png"), load("keyboard_d.png") },	"スキルチェック（方向入力）" },
	};

	if (m_bgm = KdAudioManager::Instance().Play("Asset/Sound/BGM/TitleBGM.wav", true))
	{
		m_bgm->SetVolume(0.3f);
	}
}

void TitleUI::Update()
{
	if (m_start)
	{
		m_alpha += 0.02f;

		// フェードに合わせたBGMの音量調整
		if (m_bgm)
		{
			// m_alphaが1.0に向かうにつれて音量を0に近づける（初期音量が0.2fの場合）
			float volume = 0.2f * (1.0f - m_alpha);
			if (volume < 0.0f) volume = 0.0f;
			m_bgm->SetVolume(volume);
		}

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

	// クリックした瞬間だけtrueにする
	bool click = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
	bool trigger = click && !m_prevClick;
	m_prevClick = click;

	// 遊び方パネル表示中：クリックかESCで閉じる
	if (m_showHowTo)
	{
		if (trigger)
		{
			m_howToPage++;
			if (m_howToPage >= 2)   // 2ページ目の次で閉じる
			{
				m_howToPage = 0;
				m_showHowTo = false;
			}
		}
		if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
		{
			m_howToPage = 0;
			m_showHowTo = false;
		}
		return;
	}

	// 矩形ヒット判定
	auto hitTest = [&](const Math::Vector2& pos, const Math::Vector2& size)
		{
			return  m_mousePosX >= pos.x - size.x * 0.5f &&
				m_mousePosX <= pos.x + size.x * 0.5f &&
				m_mousePosY >= pos.y - size.y * 0.5f &&
				m_mousePosY <= pos.y + size.y * 0.5f;
		};

	for (auto& btn : m_buttons)
	{
		btn.hover = hitTest(btn.pos, btn.size);
	}

	if (trigger && m_buttons[BtnStart].hover)
	{
		m_start = true;
		SceneManager::Instance().SetStage(SceneManager::StageType::Stage1);
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

	Math::Vector2 drawPos = m_startPos;
	drawPos.y += m_animOffsetY;

	// ボタン描画（ホバー時は黄色）
	for (int i = 0; i < BtnMax; i++)
	{
		const Button& btn = m_buttons[i];

		Math::Color color = btn.hover
			? Math::Color{ 1.0f, 0.8f, 0.2f, 1.0f }
		: kWhiteColor;

		Math::Vector2 drawPos = btn.pos;

		// 上下に揺らす
		//drawPos.y += m_animOffsetY;		

		KdShaderManager::Instance().m_spriteShader.DrawFont(
			drawPos, &color, Math::Vector2{ 0.5f, 0.5f },
			0.0f, btn.font, btn.text);
	}

	// 遊び方パネル
	if (m_showHowTo)
	{
		// 背景を暗くする
		Math::Color dimColor = { 0.0f, 0.0f, 0.0f, 0.75f };
		KdShaderManager::Instance().m_spriteShader.DrawBox(0, 0, 640, 360, &dimColor);

		// パネル本体
		Math::Color panelColor = { 0.15f, 0.15f, 0.2f, 0.95f };
		KdShaderManager::Instance().m_spriteShader.DrawBox(0, 0, 520, 300, &panelColor);

		Math::Color headColor = { 1.0f, 0.8f, 0.2f, 1.0f };
		KdShaderManager::Instance().m_spriteShader.DrawFont(
			Math::Vector2{ 0.0f, 250.0f }, &headColor,
			Math::Vector2{ 0.5f, 0.5f }, 0.0f, FontID::Gothic64,
			(m_howToPage == 0) ? "遊び方" : "操作方法");

		if (m_howToPage == 0)
		{
			// ---- 1ページ目：ルール ----
			static const char* rules[] =
			{
				"朝から夜までの約5分間、ご近所にいたずらしてスコアを競おう！",
				"1. 中央エリアでアイテムを拾おう（最大3個まで持てる）",
				"2. 他人の家でアイテムを使っていたずらしよう",
				"3. 自分の家がいたずらされたら、アイテムで修復しよう",
				"4. いたずら・修復の成功でスコアゲット",
				"5. 警察に見つかると交番へ連行。アイテムも没収！",
				"※ いたずら中はミニゲーム（スキルチェック）が発生",
				"※ 相手はNPC3人。みんなでいたずらし合うぞ"
			};

			float y = 150.0f;
			for (const char* line : rules)
			{
				KdShaderManager::Instance().m_spriteShader.DrawFont(
					Math::Vector2{ -480.0f, y }, &kWhiteColor,
					Math::Vector2{ 0.0f, 0.5f }, 0.0f, FontID::Gothic32, line);
				y -= 50.0f;
			}
		}
		else
		{
			// ---- 2ページ目：操作方法（アイコン＋説明）----
			const float iconSize = 44.0f;
			const float iconStep = 50.0f;
			float y = 150.0f;

			for (const auto& info : m_controls)
			{
				float x = -440.0f;
				for (const auto& icon : info.icons)
				{
					KdShaderManager::Instance().m_spriteShader.DrawTex(
						icon,
						static_cast<int>(x), static_cast<int>(y),
						static_cast<int>(iconSize), static_cast<int>(iconSize),
						nullptr, &kWhiteColor, Math::Vector2{ 0.0f, 0.5f });
					x += iconStep;
				}

				KdShaderManager::Instance().m_spriteShader.DrawFont(
					Math::Vector2{ -210.0f, y }, &kWhiteColor,
					Math::Vector2{ 0.0f, 0.5f }, 0.0f, FontID::Gothic32,
					info.text.c_str());

				y -= 50.0f;
			}
		}

		KdShaderManager::Instance().m_spriteShader.DrawFont(
			Math::Vector2{ 0.0f, -270.0f }, &kWhiteColor,
			Math::Vector2{ 0.5f, 0.5f }, 0.0f, FontID::Gothic32,
			(m_howToPage == 0) ? "クリックで次へ" : "クリックで閉じる");
	}

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
