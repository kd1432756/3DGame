#include "StaticMeshObject.h"

namespace fs = std::filesystem;

void StaticMeshObject::Init()
{
	if (!m_modelPath.empty())
	{
		ReloadModel();
	}	
}

void StaticMeshObject::DrawLit()
{
	if (!m_spModel) return;

	KdShaderManager::Instance().m_StandardShader.DrawModel(
		*m_spModel,
		m_mWorld,
		m_color);
}

void StaticMeshObject::GenerateDepthMapFromLight()
{
	if (!m_spModel) return;

	KdShaderManager::Instance().m_StandardShader.DrawModel(
		*m_spModel,
		m_mWorld,
		m_color);
}

void StaticMeshObject::SetModelPath(const std::string& path)
{
	if (m_modelPath == path)
		return;

	m_modelPath = path;

	ReloadModel();
}

void StaticMeshObject::ReloadModel()
{
	if (m_modelPath.empty()) return;

	if (!fs::exists(m_modelPath))
	{
		return;
	}

	fs::path path(m_modelPath);
	if (path.extension() != ".gltf")
	{
		return;
	}

	if (!m_spModel)
	{
		m_spModel = std::make_shared<KdModelData>();
	}

//	m_spModel->Load(m_modelPath);

	m_spModel =
		KdAssets::Instance().m_modeldatas.GetData(m_modelPath);

	RegisterCollider();
}

void StaticMeshObject::RegisterCollider()
{
	if (!m_pCollider)
	{
		m_pCollider = std::make_unique<KdCollider>();
	}

	m_pCollider->RegisterCollisionShape(
		"StaticMeshObject",
		m_spModel,
		KdCollider::TypeEditor | KdCollider::TypeBump);
}

void StaticMeshObject::SetCollisionEnabled(bool enabled)
{
	m_collisionEnabled = enabled;

	// OFFにした場合はCollider自体を破棄
	if (!m_collisionEnabled)
	{
		m_pCollider.reset();

		return;
	}

	// ONにした場合は再登録
	if (m_spModel)
	{
		RegisterCollider();
	}
}
