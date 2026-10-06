#include "WoodenFence.h"

#include "../../../../../Scene/SceneManager.h"

#include "../../../../Character/Player/Player.h"
#include "../../Item/Item.h"

void WoodenFence::Init()
{
	SetModelPath("Asset/Models/Props/WoodenFence/WoodenFence.gltf");

	RegisterAction({
		InteractiveState::Normal,
		InteractiveState::Damaged,
		ItemAction::Break,
		"WoodenFence",
		3.5f,
		300,
		"Wooden Fence Break!",
		"Your Wodden Fence was Broken!",
		"Asset/Models/Props/WoodenFence/WoodenFenceBroken.gltf"
		});

	// ↓は仮
	RegisterAction({
		InteractiveState::Damaged,
		InteractiveState::Repaired,
		ItemAction::Repair,
		"WoodenFence",
		3.0f,
		210,
		"Wooden Fence Repair!",
		"",
		"Asset/Models/Props/Flower/FlowerTape.gltf"
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
