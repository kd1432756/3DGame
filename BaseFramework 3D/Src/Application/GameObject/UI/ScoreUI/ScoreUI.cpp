#include "ScoreUI.h"

#include "../../Character/Player/Player.h"

void ScoreUI::Init()
{}

void ScoreUI::Update()
{
	auto player = LockPlayer();

	if (!player)
	{
		return;
	}

	// デバッグ
	if (GetAsyncKeyState('4') & 0x8000)
	{
		player->AddScore(100);
	}

	const int targetScore = player->GetScore();

	if (m_displayScore == targetScore)
	{
		return;
	}

	const int difference =
		std::abs(targetScore - m_displayScore);

	const int speed =
		(std::max)(1, difference / 10);

	if (m_displayScore < targetScore)
	{
		m_displayScore =
			(std::min)(m_displayScore + speed, targetScore);
	}
	else
	{
		m_displayScore =
			(std::max)(m_displayScore - speed, targetScore);
	}
}

void ScoreUI::DrawSprite()
{
	if (!LockPlayer())
	{
		return;
	}

	const std::string scoreText = std::to_string(m_displayScore);

	const float DigitWidth = 48.0f;

	const size_t digitCount = scoreText.length();

	const float ScoreTextX = digitCount * DigitWidth;

	// -----------------------------
	// Score : の表示
	// -----------------------------

	DrawLabel(
		Math::Vector2{ ScoreX - ScoreTextX, ScoreY },
		kWhiteColor,
		FontID::FutureNormal64,
		"Score : ",
		Math::Vector2{ 1.0f, 0.5f });

	// -----------------------------
	// 数字の表示（1桁ずつ固定幅で描いて桁ズレを防ぐ）
	// -----------------------------

	for (size_t i = 0; i < digitCount; ++i)
	{
		const float x =
			ScoreX -
			(static_cast<float>(digitCount - i) * DigitWidth);

		const char digit[] =
		{
			scoreText[i],
			'\0'
		};

		DrawLabel(
			Math::Vector2{ x + DigitWidth, ScoreY },
			kWhiteColor,
			FontID::FutureNormal64,
			digit,
			Math::Vector2{ 1.0f, 0.5f });
	}
}
