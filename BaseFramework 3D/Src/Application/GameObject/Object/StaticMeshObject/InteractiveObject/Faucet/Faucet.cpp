#include "Faucet.h"

#include "../../../../../Scene/SceneManager.h"

#include "../../../../Character/Player/Player.h"
#include "../../Item/Item.h"

void Faucet::Init()
{
	SetModelPath("Asset/Models/Props/Faucet/Faucet.gltf");

	RegisterAction({
		InteractiveState::Normal,
		InteractiveState::Damaged,
		ItemAction::Wrench,
		"Faucet",
		1.5f,
		200,
		"Faucet Loosened!",
		"Your Faucet has been Loosened!",
		""
		});

	RegisterAction({
		InteractiveState::Damaged,
		InteractiveState::Normal,
		ItemAction::Repair,
		"Faucet",
		3.0f,
		80,
		"Faucet Tightened!",
		"",
		""
		});

	SetCollisionEnabled(false);

	StaticMeshObject::Init();
}

void Faucet::Update()
{
	if (m_state == InteractiveState::Damaged)
	{
		auto spEffect = m_wpEffect.lock();
		if (!spEffect)
		{
			Math::Vector3 pos = m_pos;

			const KdModelData::Node* pNode =
				m_spModel->FindNode("WaterSpawnPoint");
			if (pNode)
			{
				Math::Matrix mat = pNode->m_worldTransform * m_mWorld;
				pos = mat.Translation();
			}

			m_wpEffect = KdEffekseerManager::GetInstance().Play(
				"faucetLoop.efkefc", pos,
				SceneManager::Instance().GetSceneScale() * 1.5f,
				1.0f, false);
		}
	}
	else
	{
		if (auto spEffect = m_wpEffect.lock())
		{
			spEffect->StopEffect();
		}
		m_wpEffect.reset();
	}
}

void Faucet::OnAction(const InteractiveAction& action, const std::shared_ptr<Player>& player)
{
	KdEffekseerManager::GetInstance().Play("interact.efkefc", m_pos, SceneManager::Instance().GetSceneScale() * 3.0f);
}

void Faucet::RegisterCollider()
{
	if (!m_pCollider)
	{
		m_pCollider = std::make_unique<KdCollider>();
	}

	m_pCollider->RegisterCollisionShape(
		"Faucet",
		m_spModel,
		KdCollider::TypeEditor | KdCollider::TypeBump | KdCollider::TypeInteract);
}
