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
	Math::Vector3 pos = m_pos + Math::Vector3{0, GetScale().y * 1.2f, 0};
	KdEffekseerManager::GetInstance().Play("Test.efkefc", pos, 0.05f);
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
