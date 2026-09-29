#include "Flower.h"

#include "../../../../../Scene/SceneManager.h"

#include "../../../../Character/Player/Player.h"
#include "../../Item/Item.h"

void Flower::Init()
{
	SetModelPath("Asset/Models/Props/Flower/Flower.gltf");

	RegisterAction({
		InteractiveState::Normal,
		InteractiveState::Damaged,
		ItemAction::Cut,
		"Flower",
		1.5f,
		200,
		"Flower Cut!",
		"Your Flower was Cut!",
		"Asset/Models/Props/Flower/FlowerCut.gltf",
		true
		});

	RegisterAction({
		InteractiveState::Damaged,
		InteractiveState::Repaired,
		ItemAction::Repair,
		"Flower",
		3.0f,
		80,
		"Flower Repair!",
		"",
		"Asset/Models/Props/Flower/FlowerTape.gltf",
		true
		});

	SetCollisionEnabled(false);

	StaticMeshObject::Init();
}

void Flower::RegisterCollider()
{
	if (!m_pCollider)
	{
		m_pCollider = std::make_unique<KdCollider>();
	}

	m_pCollider->RegisterCollisionShape(
		"Flower",
		m_spModel,
		KdCollider::TypeEditor | KdCollider::TypeBump | KdCollider::TypeInteract);
}
