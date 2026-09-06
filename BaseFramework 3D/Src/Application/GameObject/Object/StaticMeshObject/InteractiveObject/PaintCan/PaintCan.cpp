#include "PaintCan.h"

#include "../../../../../Scene/SceneManager.h"

#include "../../../../Character/Player/Player.h"
#include "../../Item/Item.h"

void PaintCan::Init()
{
	SetModelPath("Asset/Models/Props/PaintCan/PaintCan.gltf");

	StaticMeshObject::Init();
}

void PaintCan::DrawBright()
{
	if (!IsFocused())
	{
		return;
	}

	if (!m_spModel) return;

	Math::Color color{ 0.1,0.1,0.1,1 };
	KdShaderManager::Instance().m_StandardShader.DrawModel(
		*m_spModel,
		m_mWorld,
		color);
}


void PaintCan::RegisterCollider()
{
	if (!m_pCollider)
	{
		m_pCollider = std::make_unique<KdCollider>();
	}

	m_pCollider->RegisterCollisionShape(
		"PaintCan",
		m_spModel,
		KdCollider::TypeEditor | KdCollider::TypeBump | KdCollider::TypeInteract);
}
