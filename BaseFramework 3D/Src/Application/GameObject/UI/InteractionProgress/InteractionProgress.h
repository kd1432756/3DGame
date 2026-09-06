#pragma once

class Player;

class InteractionProgress : public KdGameObject
{
public:

	void Init() override;
	void DrawSprite() override;
	void Update() override;

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

	bool m_visible = false;

	float m_progress = 0.0f;

};