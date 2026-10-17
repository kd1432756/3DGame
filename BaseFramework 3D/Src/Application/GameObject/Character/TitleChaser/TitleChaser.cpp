#include "TitleChaser.h"

void TitleChaser::Init()
{
	m_spModel = std::make_shared<KdModelWork>();

	m_spAnimator = std::make_shared<KdAnimator>();
}

void TitleChaser::Update()
{
	m_angle += m_angleSpeed;

	Math::Vector3 pos;
	pos.x = m_center.x + cosf(m_angle) * m_radius;
	pos.y = m_center.y;
	pos.z = m_center.z + sinf(m_angle) * m_radius;

	// 進行方向（接線方向）を向かせる
	float yaw = -m_angle + (m_angleSpeed > 0 ? -DirectX::XM_PIDIV2 : DirectX::XM_PIDIV2) + m_yawOffset;

	m_mWorld = Math::Matrix::CreateRotationX(DirectX::XMConvertToRadians(90))
		* Math::Matrix::CreateRotationY(yaw)
		* Math::Matrix::CreateTranslation(pos);

	{
		if (!m_spAnimator)return;
		if (!m_spModel)return;

		m_spAnimator->AdvanceTime(m_spModel->WorkNodes());
		m_spModel->CalcNodeMatrices();
	}
}

void TitleChaser::DrawLit()
{
	if (!m_spModel)return;
	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_spModel, m_mWorld);
}

void TitleChaser::GenerateDepthMapFromLight()
{
	if (!m_spModel)return;
	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_spModel, m_mWorld);
}

void TitleChaser::SetModel(const std::string& path)
{
	if (!m_spModel)return;
	m_spModel->SetModelData(path);
	m_spAnimator->SetAnimation(m_spModel->GetAnimation("Armature|Sprint"));
	SetYawOffset(DirectX::XMConvertToRadians(-90.0f));
}
