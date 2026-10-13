#include "PaintCan.h"

#include "../../../../../Scene/SceneManager.h"

#include "../../../../Character/Player/Player.h"
#include "../../Item/Item.h"

void PaintCan::Init()
{
	SetModelPath("Asset/Models/Props/PaintCan/PaintCan.gltf");

	RegisterAction({
		InteractiveState::Normal,
		InteractiveState::Damaged,
		ItemAction::None,
		"PaintCan",
		1.5f,
		100,
		"PaintCan Spilled!",
		"Your PaintCan has been Spilled!",
		"Asset/Models/Props/PaintCan/SpilledPaintCan.gltf",
		false,
		"Spill"
		});

	RegisterAction({
		InteractiveState::Damaged,
		InteractiveState::Normal,
		ItemAction::Repair,
		"PaintCan",
		2.0f,
		60,
		"PaintCan Cleaned Up!",
		"",
		"Asset/Models/Props/PaintCan/PaintCan.gltf"
		});

	StaticMeshObject::Init();
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
