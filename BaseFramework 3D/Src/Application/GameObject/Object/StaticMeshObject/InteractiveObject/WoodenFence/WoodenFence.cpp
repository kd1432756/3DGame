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
		"Your Wodd Fence has been Broken!",
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
	KdEffekseerManager::GetInstance().Play("interact.efkefc", m_pos, SceneManager::Instance().GetSceneScale() * 5.0f);

	if (m_state == InteractiveState::Damaged)
	{
		Math::Vector3 pos = m_pos + Math::Vector3{0, GetScale().y, 0};
		Math::Vector3 dir = m_mWorld.Backward();
		auto effect = KdEffekseerManager::GetInstance().Play("fenceBreak.efkefc", pos, SceneManager::Instance().GetSceneScale() * 5.0f);
		effect.lock()->SetRotation(dir);
	}
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
