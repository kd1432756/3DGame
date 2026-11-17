#pragma once

class TitleProp : public KdGameObject
{
public:
	void Init() override;
	void DrawLit() override;
	void GenerateDepthMapFromLight() override;

	void SetModel(const std::string& path);

	void SetTransform(const Math::Vector3& pos,
		const Math::Vector3& rotDeg = Math::Vector3::Zero,
		const Math::Vector3& scale = Math::Vector3::One);

private:
	std::shared_ptr<KdModelData> m_spModel;
};