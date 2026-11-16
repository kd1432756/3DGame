#pragma once

class Player;

class GrassField : public KdGameObject
{
public:
	void Generate(float density, float obstacleRadius);

	void DrawLit() override;

	void SetPlayer(const std::shared_ptr<Player>& player) { m_wpPlayer = player; }

	// この距離より遠い草は描画しない
	void SetDrawDistance(float distance) { m_drawDistance = distance; }

private:
	void Init() override {}

	std::shared_ptr<KdModelData> m_model = nullptr;
	std::vector<Math::Matrix> m_instances;

	std::weak_ptr<Player> m_wpPlayer;
	float m_drawDistance = 15.0f;
};