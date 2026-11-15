#pragma once

class Player;

class DeliveryPointEffect : public KdGameObject
{
public:
	void Init() override {}
	void Update() override;

	void SetPlayer(const std::shared_ptr<Player>& player) { m_wpPlayer = player; }

private:
	std::weak_ptr<Player> m_wpPlayer;
	std::weak_ptr<KdEffekseerObject> m_wpEffect;
};