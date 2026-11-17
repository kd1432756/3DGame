#include "InteractionProgress.h"

#include "../../Character/Player/Player.h"

void InteractionProgress::Update()
{
	auto player = LockPlayer();

	if (player)
	{
		m_visible = player->IsCounting();
		m_progress = player->GetUseProgress();
	}
}

void InteractionProgress::DrawSprite()
{
	if (!m_visible) return;

	// バー
	DrawGauge(0, -40, 90, 6, m_progress,
		Math::Color{ 0.0f, 0.0f, 0.0f, 0.5f },
		Math::Color{ 0.0f, 1.0f, 0.0f, 1.0f });
}
