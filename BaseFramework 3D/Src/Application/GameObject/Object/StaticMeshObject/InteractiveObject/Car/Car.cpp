#include "Car.h"

#include "../../../../../Scene/SceneManager.h"

#include "../../../../Character/Player/Player.h"
#include "../../Item/Item.h"

void Car::Init()
{
	SetModelPath("Asset/Models/Props/Car/Car.gltf");

	RegisterAction({
		InteractiveState::Normal,
		InteractiveState::Damaged,
		ItemAction::Pirece,
		"Car",
		1.5f,
		200,
		"Car Panctured!",
		"Your Car was Panctured!",
		"Asset/Models/Props/Car/CarPanctured.gltf"
		});

	RegisterAction({
		InteractiveState::Damaged,
		InteractiveState::Normal,
		ItemAction::Repair,
		"Car",
		3.0f,
		80,
		"Tire Inflated!",
		"",
		"Asset/Models/Props/Car/Car.gltf",
		false,
		"Inflate"
		});

	StaticMeshObject::Init();
}

void Car::Update()
{
	if (m_state == InteractiveState::Damaged)
	{
		Math::Vector3 pos = m_pos;
		if (const KdModelData::Node* pNode = m_spModel->FindNode("PancturePoint"))
		{
			pos = (pNode->m_worldTransform * m_mWorld).Translation();
		}

		// 車の回転だけを取り出す(拡大・移動は除く)
		Math::Vector3 carScale, carTrans;
		Math::Quaternion carRot;
		m_mWorld.Decompose(carScale, carRot, carTrans);

		const float effectScale = SceneManager::Instance().GetSceneScale() * 1.5f;

		// 拡大 → 車から見て右へ90度 → 車の向き → 位置
		Math::Matrix mEffect =
			Math::Matrix::CreateScale(effectScale) *
			Math::Matrix::CreateRotationY(DirectX::XMConvertToRadians(-90.0f)) *
			Math::Matrix::CreateFromQuaternion(carRot) *
			Math::Matrix::CreateTranslation(pos);

		// 再生していなければ再生する
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
}

void Car::RegisterCollider()
{
	if (!m_pCollider)
	{
		m_pCollider = std::make_unique<KdCollider>();
	}

	m_pCollider->RegisterCollisionShape(
		"Car",
		m_spModel,
		KdCollider::TypeEditor | KdCollider::TypeBump | KdCollider::TypeInteract);
}
