#pragma once

class GrassField : public KdGameObject
{
public:
	// 全オブジェクトが揃った後に呼ぶ
	void Generate(float density, float obstacleRadius);

	void DrawLit() override;

private:
	void Init() override {}

	std::shared_ptr<KdModelData> m_model = nullptr;
	std::vector<Math::Matrix> m_instances;	// 草1本ごとのワールド行列
};