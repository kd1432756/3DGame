#include "ReportUI.h"

#include "../../Character/Player/Player.h"

void ReportUI::Init()
{
	m_tex.Load("Asset/Textures/UI/ReportIcon.png");
	m_texKeyR.Load("Asset/Textures/UI/Key/keyboard_r.png");
}

void ReportUI::Update()
{
	auto player = LockPlayer();
	if (!player) return;

	m_remaining = player->GetReportRemaining();
	m_ratio = player->GetReportCooldownRatio();

	const bool cooling = m_remaining > 0.0f;

	// クールダウン明け → フラッシュ開始
	if (m_prevCooling && !cooling)
	{
		m_flashTimer = kFlashTime;
	}
	m_prevCooling = cooling;

	if (m_flashTimer > 0.0f)
	{
		m_flashTimer = (std::max)(0.0f, m_flashTimer - DeltaTime());
	}
}

void ReportUI::DrawSprite()
{
	auto& sprite = KdShaderManager::Instance().m_spriteShader;

	const bool cooling = m_remaining > 0.0f;
	const int  radius = kIconSize / 2;
	const Math::Vector2 centerPivot = { 0.5f, 0.5f };

	//============================================================
	// キー
	//============================================================

	const int keyOffsetY = 75;
	const int keyPosY = kPosY + keyOffsetY;

	Math::Color keyColor = kWhiteColor;
	if (cooling)
	{
		keyColor = { 0.3f, 0.3f, 0.3f, 1.0f };
	}

	sprite.DrawTex(&m_texKeyR, kPosX, keyPosY, kKeySize, kKeySize,
		nullptr, &keyColor, centerPivot);

	//============================================================
	// アイコン（画像そのものが円）
	//============================================================
	if (cooling)
	{
		// 暗い全体
		Math::Color dim = { 0.35f, 0.35f, 0.35f, 1.0f };
		sprite.DrawTex(&m_tex, kPosX, kPosY, kIconSize, kIconSize,
			nullptr, &dim, centerPivot);

		// 明るい部分（下から満ちていく）
		const float fill = 1.0f - m_ratio;   // 0→1
		const int   fillH = static_cast<int>(kIconSize * fill);

		if (fillH > 0)
		{
			const int texW = static_cast<int>(m_tex.GetInfo().Width);
			const int texH = static_cast<int>(m_tex.GetInfo().Height);
			const int srcH = static_cast<int>(texH * fill);

			Math::Rectangle src(0, texH - srcH, texW, srcH);

			Math::Color bright = { 1.0f, 1.0f, 1.0f, 1.0f };

			sprite.DrawTex(&m_tex,
				kPosX, kPosY - radius,
				kIconSize, fillH,
				&src, &bright, Math::Vector2{ 0.5f, 0.0f });
		}
	}
	else
	{
		Math::Color white = { 1.0f, 1.0f, 1.0f, 1.0f };
		sprite.DrawTex(&m_tex, kPosX, kPosY, kIconSize, kIconSize,
			nullptr, &white, centerPivot);
	}

	//============================================================
	// 外枠（半径をずらして重ね描きして太くする）
	//============================================================
	const Math::Color ring = cooling
		? Math::Color{ 0.6f, 0.6f, 0.6f, 1.0f }
	: Math::Color{ 0.8f, 0.2f, 0.0f, 1.0f };

	for (int i = 0; i < kRingWidth; ++i)
	{
		sprite.DrawCircle(kPosX, kPosY, radius + i, &ring, false);
	}

	//============================================================
	// 使用可能になった瞬間のフラッシュ
	//============================================================
	if (m_flashTimer > 0.0f)
	{
		const float t = 1.0f - (m_flashTimer / kFlashTime);   // 0→1
		const int   r = radius + kRingWidth + static_cast<int>(t * 24.0f);

		Math::Color flash = { 0.6f, 0.85f, 1.0f, 1.0f - t };
		for (int i = 0; i < 2; ++i)
		{
			sprite.DrawCircle(kPosX, kPosY, r + i, &flash, false);
		}
	}
}
