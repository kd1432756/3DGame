#pragma once

class Player;

class GrassField : public KdGameObject
{
public:
	// 全オブジェクトが揃った後に呼ぶ
	void Generate(float density, float obstacleRadius);

	void DrawLit() override;

	void SetPlayer(const std::shared_ptr<Player>& player) { m_wpPlayer = player; }

	// この距離より遠い草は描画しない()
	void SetDrawDistance(float distance) { m_drawDistance = distance; }

private:
	void Init() override {}

	std::shared_ptr<KdModelData> m_model = nullptr;
	std::vector<Math::Matrix> m_instances;	// 草1本ごとのワールド行列

	std::weak_ptr<Player> m_wpPlayer;
	float m_drawDistance = 10.0f;
};