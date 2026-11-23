#include "BonusUI.h"

#include "../../Character/Player/Player.h"

void BonusUI::Init()
{
	m_keyTex = LoadTexture("Asset/Textures/UI/Key/keyboard_e.png");
	m_animTime = 0.0f;

	m_prevBonusCount = 0;
	m_popTimer = 0.0f;
	m_ringTimer = 0.0f;
}

void BonusUI::Update()
{
	m_animTime += DeltaTime();

	auto player = LockPlayer();
	if (!player) return;

	const int count = player->GetRepairBonusCount();

	// カウントが増えた瞬間だけポップとリングを出す
	if (count > m_prevBonusCount)
	{
		m_popTimer = kPopTime;
		m_ringTimer = kRingTime;
	}
	m_prevBonusCount = count;

	if (m_popTimer > 0.0f)
	{
		m_popTimer -= DeltaTime();
		if (m_popTimer < 0.0f) m_popTimer = 0.0f;
	}

	if (m_ringTimer > 0.0f)
	{
		m_ringTimer -= DeltaTime();
		if (m_ringTimer < 0.0f) m_ringTimer = 0.0f;
	}
}

void BonusUI::DrawSprite()
{
	auto player = LockPlayer();

	if (!player) return;

	if (player->GetRepairBonusCount() > 0)
	{
		const float remain = player->GetRepairBonusTime();

		const bool low = remain <= 10.0f;

		Math::Color labelColor = low
			? Math::Color{ 1.0f, 0.4f, 0.3f, 1.0f }
		: Math::Color{ 1.0f, 0.85f, 0.2f, 1.0f };

		// ポップ
		float popOffsetY = 0.0f;
		if (m_popTimer > 0.0f)
		{
			const float t = 1.0f - (m_popTimer / kPopTime);
			popOffsetY = kPopHeight * sinf(t * DirectX::XM_PI);
			labelColor = Math::Color{ 1.0f, 1.0f, 1.0f, 1.0f };
		}

		auto& sprite = KdShaderManager::Instance().m_spriteShader;

		// score
		DrawLabel(Math::Vector2{ kBonusX, kBonusY + kScoreOffsetY + popOffsetY },
			labelColor, FontID::Future16, "score");

		// 倍率
		const char* markText = "×";
		char rateText[32];
		sprintf_s(rateText, "%.1f",
			player->GetRepairBonusRate());

		const float markW = sprite.CalcFontSize(
			KdFontManager::Instance().CreateFontTexture(
				static_cast<int>(FontID::Gothic32), markText, false)).x;
		const float rateW = sprite.CalcFontSize(
			KdFontManager::Instance().CreateFontTexture(
				static_cast<int>(FontID::Future32), rateText, false)).x;

		const float totalW = markW + kRateGap + rateW;
		const float left = kBonusX - totalW * 0.5f;
		const float rateY = kBonusY + popOffsetY;

		DrawLabel(Math::Vector2{ left, rateY },
			labelColor,
			FontID::Gothic32, markText,
			Math::Vector2{ 0.0f, 0.5f });

		DrawLabel(Math::Vector2{ left + markW + kRateGap, rateY },
			labelColor,
			FontID::Future32, rateText,
			Math::Vector2{ 0.0f, 0.5f });

		// 残り時間
		DrawGauge(
			static_cast<int>(kBonusX),
			static_cast<int>(kBonusY + kBarOffsetY),
			kBarHalfW, kBarHalfH,
			player->GetRepairBonusTimeRatio(),
			Math::Color{ 0.0f, 0.0f, 0.0f, 0.5f },
			labelColor);

		// リング
		if (m_ringTimer > 0.0f)
		{
			const float t = 1.0f - (m_ringTimer / kRingTime);
			const float radius = kRingStartRadius + (kRingEndRadius - kRingStartRadius) * t;
			const Math::Color ringColor{ 1.0f, 0.85f, 0.2f, 1.0f - t };

			sprite.DrawCircle(
				(int)kBonusX, (int)(kBonusY + popOffsetY),
				(int)radius,
				&ringColor,
				false);
		}
	}

	// Eキーと「Turn in」
	if (player->CanDeliver() && !player->IsUsing() && !player->IsArrested())
	{
		const float a = 0.75f + 0.25f * std::sin(m_animTime * 6.0f);

		DrawKeyAndText(
			m_keyTex.get(),
			Math::Vector2{ kPromptX, kPromptY },
			"Turn in",
			Math::Color{ 1.0f, 1.0f, 1.0f, a },
			FontID::Future24);
	}
}