#pragma once

class Player;

class InteractionPrompt : public KdGameObject
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

	std::string m_prompt = "";

	bool m_visible = false;

	KdTexture m_texKeyE;
	bool m_showKeyIcon = false;
};