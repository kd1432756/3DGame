#include "ScoreUI.h"

#include "../../Character/Player/Player.h"

void ScoreUI::Update()
{
	auto player = LockPlayer();
	if (!player)
	{
		return;
	}

	const int targetScore = player->GetScore();

	if (GetAsyncKeyState('4') & 0x8000)
	{
		player->AddScore(100);
	}

	if (m_displayScore == targetScore)
	{
		return;
	}

	const int difference = std::abs(targetScore - m_displayScore);

	const int speed = (std::max)(1, difference / 10);

	if (m_displayScore < targetScore)
	{
		m_displayScore = (std::min)(m_displayScore + speed, targetScore);
	}
	else
	{
		m_displayScore = (std::max)(m_displayScore - speed, targetScore);
	}
}

void ScoreUI::DrawSprite()
{
	if (!LockPlayer())
	{
		return;
	}

	const std::string scoreText = std::to_string(m_displayScore);

	const size_t digitCount = scoreText.length();

	const float ScoreTextX = digitCount * kDigitWidth;

	// Score : 
	DrawLabel(
		Math::Vector2{ kScoreX - ScoreTextX, kScoreY },
		kWhiteColor,
		FontID::FutureNormal64,
		"Score : ",
		Math::Vector2{ 1.0f, 0.5f });

	// 数字
	for (size_t i = 0; i < digitCount; ++i)
	{
		const float x = kScoreX - (static_cast<float>(digitCount - i) * kDigitWidth);

		const char digit[] =
		{
			scoreText[i],
			'\0'
		};

		DrawLabel(
			Math::Vector2{ x + kDigitWidth, kScoreY },
			kWhiteColor,
			FontID::FutureNormal64,
			digit,
			Math::Vector2{ 1.0f, 0.5f });
	}
}
