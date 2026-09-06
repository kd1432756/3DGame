#include "Tree.h"

void Tree::Init()
{
	SetModelPath("Asset/Models/Props/Tree/Tree.gltf");

	StaticMeshObject::Init();
}

void Tree::RegisterCollider()
{
	if (!m_pCollider)
	{
		m_pCollider = std::make_unique<KdCollider>();
	}

	m_pCollider->RegisterCollisionShape(
		"Tree",
		m_spModel,
		KdCollider::TypeEditor | KdCollider::TypeBump | KdCollider::TypeBush);
}
