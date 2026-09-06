#pragma once

class Player;

class Reticle : public KdGameObject
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

	std::shared_ptr<KdTexture> m_tex = nullptr;

	bool m_canInteract = false;
};