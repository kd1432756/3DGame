#include "Car.h"

#include "../../../../../Scene/SceneManager.h"

#include "../../../../Character/Player/Player.h"
#include "../../Item/Item.h"

void Car::Init()
{
	SetModelPath("Asset/Models/Props/Car/Car.gltf");

	m_iconPath = "Asset/Textures/Icon/Tire.png";

	RegisterAction({
		InteractiveState::Normal,
		InteractiveState::Damaged,
		ItemAction::Pierce,
		"Car",
		6.0f,
		200,
		"Car Punctured!",
		"Car was Punctured!",
		"Asset/Models/Props/Car/CarPanctured.gltf"
		});

	RegisterAction({
		InteractiveState::Damaged,
		InteractiveState::Normal,
		ItemAction::Repair,
		"Car",
		3.0f,
		200,
		"Tire Inflated!",
		"",
		"Asset/Models/Props/Car/Car.gltf",
		false,
		"Inflate Tire"
		});

	StaticMeshObject::Init();
}

void Car::Update()
{
	if (m_state == InteractiveState::Damaged)
	{
		Math::Vector3 pos = m_pos;

		if (!m_spModel) return;

		if (const KdModelData::Node* pNode = m_spModel->FindNode("PancturePoint"))
		{
			pos = (pNode->m_worldTransform * m_mWorld).Translation();
		}

		// 車の回転
		Math::Vector3 carScale, carTrans;
		Math::Quaternion carRot;
		m_mWorld.Decompose(carScale, carRot, carTrans);

		const float effectScale = SceneManager::Instance().GetSceneScale() * 1.5f;

		Math::Matrix mEffect =
			Math::Matrix::CreateScale(effectScale) *
			Math::Matrix::CreateRotationY(DirectX::XMConvertToRadians(-90.0f)) *
			Math::Matrix::CreateFromQuaternion(carRot) *
			Math::Matrix::CreateTranslation(pos);

		auto spEffect = m_wpEffect.lock();
		if (!spEffect || !spEffect->IsPlaying())
		{
			m_wpEffect = KdEffekseerManager::GetInstance().Play(
				"tireLeakLoop.efkefc", pos, effectScale, 1.0f, false);
			spEffect = m_wpEffect.lock();
		}

		if (spEffect)
		{
			spEffect->SetWorldMatrix(mEffect);
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

void Car::OnAction(const InteractiveAction& action, const std::shared_ptr<Player>& player)
{
	KdEffekseerManager::GetInstance().Play("interact.efkefc", m_pos, SceneManager::Instance().GetSceneScale() * 10.0f);

	if (m_state == InteractiveState::Damaged)
	{
		if (auto se = KdAudioManager::Instance().Play3D("Asset/Sound/Intaract/punk.wav", m_pos))
		{
			se->SetVolume(0.1f);
			se->SetCurveDistanceScaler(SceneManager::Instance().GetSceneScale());
		}
	}
	else if (m_state == InteractiveState::Normal)
	{
		if (auto se = KdAudioManager::Instance().Play3D("Asset/Sound/Intaract/pump.wav", m_pos))
		{
			se->SetVolume(0.2f);
			se->SetCurveDistanceScaler(SceneManager::Instance().GetSceneScale());
		}
	}
}

void Car::RegisterCollider()
{
	if (!m_pCollider)
	{
		m_pCollider = std::make_unique<KdCollider>();
	}

	if (m_spModel)
	{
		m_pCollider->RegisterCollisionShape(
			"Car",
			m_spModel,
			KdCollider::TypeEditor | KdCollider::TypeBump | KdCollider::TypeInteract);
	}
}
