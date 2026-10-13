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
		"Your Flower has been Cut!",
		"Asset/Models/Props/Flower/FlowerCut.gltf"
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
		"Asset/Models/Props/Flower/FlowerTape.gltf"
		});

	SetCollisionEnabled(false);

	StaticMeshObject::Init();
}

void Flower::OnAction(const InteractiveAction& action, const std::shared_ptr<Player>& player)
{
	KdEffekseerManager::GetInstance().Play("interact.efkefc", m_pos, SceneManager::Instance().GetSceneScale() * 3.0f);
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
