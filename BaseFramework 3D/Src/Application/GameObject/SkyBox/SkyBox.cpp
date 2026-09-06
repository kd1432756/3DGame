#include "SkyBox.h"

void SkyBox::Init()
{
	if (!m_spModel)
	{
		m_spModel = std::make_shared<KdModelData>();
		m_spModel->Load("Asset/Models/SkyBox/SkyBox.gltf");
	}

	SetScale(10.0f);
}

void SkyBox::Update()
{}

void SkyBox::DrawUnLit()
{
	if (!m_spModel) return;

	KdShaderManager::Instance().m_StandardShader.DrawModel(*m_spModel, m_mWorld);
}
