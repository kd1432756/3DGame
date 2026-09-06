#include "Ground.h"

void Ground::Init()
{
	StaticMeshObject::Init();
}

void Ground::RegisterCollider()
{
	if (!m_pCollider)
	{
		m_pCollider = std::make_unique<KdCollider>();
	}

	m_pCollider->RegisterCollisionShape(
		"Ground",
		m_spModel,
		KdCollider::TypeEditor | KdCollider::TypeGround);
}
