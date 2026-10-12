#include "WoodenFence.h"

#include "../../../../../Scene/SceneManager.h"

#include "../../../../Character/Player/Player.h"
#include "../../Item/Item.h"

void WoodenFence::Init()
{
	SetModelPath("Asset/Models/Props/WoodFence/WoodFence_Repaired.gltf");

	RegisterAction({
		InteractiveState::Normal,
		InteractiveState::Damaged,
		ItemAction::Break,
		"WoodFence",
		3.5f,
		300,
		"Wooden Fence Break!",
		"Your Wodd Fence was Broken!",
		"Asset/Models/Props/WoodFence/WoodFence_Broken.gltf"
		});

	RegisterAction({
		InteractiveState::Damaged,
		InteractiveState::Normal,
		ItemAction::Repair,
		"WoodFence",
		3.0f,
		210,
		"Wood Fence Repair!",
		"",
		"Asset/Models/Props/WoodFence/WoodFence_Repaired.gltf"
		});

	StaticMeshObject::Init();
}

void WoodenFence::OnAction(const InteractiveAction& action, const std::shared_ptr<Player>& player)
{

}

void WoodenFence::RegisterCollider()
{
	if (!m_pCollider)
	{
		m_pCollider = std::make_unique<KdCollider>();
	}

	m_pCollider->RegisterCollisionShape(
		"WoodenFence",
		m_spModel,
		KdCollider::TypeEditor | KdCollider::TypeBump | KdCollider::TypeInteract);
}
