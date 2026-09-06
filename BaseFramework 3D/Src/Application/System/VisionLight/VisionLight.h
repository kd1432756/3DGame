#pragma once

// 敵の視界を表現する扇形ポリゴン
class VisionLight : public KdPolygon
{
public:

	VisionLight();
	~VisionLight() override {}

	// 視界の設定
	// distance : 視界距離
	// angleDeg : 視野角（度）
	void SetVision(float distance, float angleDeg);

	// 視界距離だけ変更
	void SetDistance(float distance);

	// 視野角だけ変更
	void SetAngle(float angleDeg);

	// 分割数
	void SetDivision(UINT division);

	// 視界の色
	void SetVisionColor(const Math::Color& color);

	// 視界の情報取得
	float GetDistance() const { return m_distance; }
	float GetAngle() const { return m_angleDeg; }

	Math::Color GetVisionColor() const{ return m_color; }

private:

	// 扇形の頂点を作成
	void CreateVertices();

	float m_distance = 10.0f;

	// 度数法
	float m_angleDeg = 60.0f;

	// 扇形の分割数
	UINT m_division = 32;

	// 視界の色
	Math::Color m_color = Math::Color(1.0f, 1.0f, 0.0f, 0.35f);

	// 地面から少し浮かせる
	float m_height = 0.03f;
};