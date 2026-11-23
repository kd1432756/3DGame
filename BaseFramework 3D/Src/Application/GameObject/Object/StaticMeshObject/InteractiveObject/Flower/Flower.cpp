#include "Flower.h"

#include "../../../../../Scene/SceneManager.h"

#include "../../../../Character/Player/Player.h"
#include "../../Item/Item.h"

void Flower::Init()
{
	SetModelPath("Asset/Models/Props/Flower/Flower.gltf");

	m_iconPath = "Asset/Textures/Icon/Flower.png";

	RegisterAction({
		InteractiveState::Normal,
		InteractiveState::Damaged,
		ItemAction::Cut,
		"Flower",
		6.0f,
		120,
		"Flower Cut!",
		"Flower has been Cut!",
		"Asset/Models/Props/Flower/FlowerCut.gltf"
		});

	RegisterAction({
		InteractiveState::Damaged,
		InteractiveState::Repaired,
		ItemAction::Repair,
		"Flower",
		3.0f,
		120,
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

	if (m_state == InteractiveState::Damaged)
	{
		Math::Vector3 pos;
		if (const KdModelData::Node* pNode = m_spModel->FindNode("FlowerCutPoint"))
		{
			pos = (pNode->m_worldTransform * m_mWorld).Translation();
			KdEffekseerManager::GetInstance().Play("flowerCut.efkefc", pos, SceneManager::Instance().GetSceneScale() * 1.0f);
		}
						   
		if (auto se = KdAudioManager::Instance().Play3D("Asset/Sound/Intaract/scissors.wav", m_pos))
		{
			se->SetVolume(0.3f);
			se->SetCurveDistanceScaler(SceneManager::Instance().GetSceneScale());
		}
	}
	else if (m_state == InteractiveState::Repaired)
	{
		if (auto se = KdAudioManager::Instance().Play3D("Asset/Sound/Intaract/tape.wav", m_pos))
		{
			se->SetVolume(0.2f);
			se->SetCurveDistanceScaler(SceneManager::Instance().GetSceneScale());
		}
	}
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
