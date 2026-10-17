#include "TitleProp.h"

void TitleProp::Init()
{
	m_spModel = std::make_shared<KdModelData>();
}

void TitleProp::DrawLit()
{
	if (!m_spModel) return;
	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_spModel, m_mWorld);
}

void TitleProp::GenerateDepthMapFromLight()
{
	if (!m_spModel) return;
	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_spModel, m_mWorld);
}

void TitleProp::SetModel(const std::string& path)
{
	if (!m_spModel) return;
	m_spModel->Load(path);
}

void TitleProp::SetTransform(const Math::Vector3& pos,
	const Math::Vector3& rotDeg,
	const Math::Vector3& scale)
{
	m_mWorld = Math::Matrix::CreateScale(scale)
		* Math::Matrix::CreateFromYawPitchRoll(
			DirectX::XMConvertToRadians(rotDeg.y),
			DirectX::XMConvertToRadians(rotDeg.x),
			DirectX::XMConvertToRadians(rotDeg.z))
		* Math::Matrix::CreateTranslation(pos);
}
