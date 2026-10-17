#pragma once

class TitleChaser : public KdGameObject
{
public:
	void Init() override;
	void Update() override;
	void DrawLit() override;
	void GenerateDepthMapFromLight() override;

	// 設定用
	void SetModel(const std::string& path);
	void SetCenter(const Math::Vector3& center) { m_center = center; }
	void SetRadius(float r) { m_radius = r; }
	void SetAngleSpeed(float s) { m_angleSpeed = s; }  // 負なら逆回り
	void SetStartAngle(float a) { m_angle = a; }
	void SetYawOffset(float rad) { m_yawOffset = rad; }

private:
	std::shared_ptr<KdModelWork>	m_spModel = nullptr;
	std::shared_ptr<KdAnimator>		m_spAnimator = nullptr;

	Math::Vector3 m_center = Math::Vector3::Zero;
	float m_radius = 3.0f;
	float m_angle = 0.0f;   // ラジアン
	float m_angleSpeed = 0.02f;
	float m_yawOffset = 0.0f;   // モデルごとの向き補正（ラジアン）
};