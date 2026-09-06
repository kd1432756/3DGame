#pragma once

class Player;

class ScoreUI : public KdGameObject
{
public:

	void Init() override;
	void Update() override;
	void DrawSprite() override;

	ObjectCategory GetCategory() const override
	{
		return ObjectCategory::UI;
	}

	void SetPlayer(const std::shared_ptr<Player>& player)
	{
		m_wpPlayer = player;
	}

private:

	std::weak_ptr<Player> m_wpPlayer;

	int m_displayScore = 0;

	static constexpr float ScoreX = 600.0f;
	static constexpr float ScoreY = 300.0f;
};
