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
		"PaintCan has been Spilled!",
		"Asset/Models/Props/PaintCan/SpilledPaintCan.gltf",
		false,
		"Spill"
		});

	RegisterAction({
		InteractiveState::Damaged,
		InteractiveState::Normal,
		ItemAction::Repair,
		"PaintCan",
		2.5f,
		60,
		"PaintCan Cleaned Up!",
		"",
		"Asset/Models/Props/PaintCan/PaintCan.gltf"
		});

	StaticMeshObject::Init();
}

void PaintCan::OnAction(const InteractiveAction& action, const std::shared_ptr<Player>& player)
{
	KdEffekseerManager::GetInstance().Play("interact.efkefc", m_pos, SceneManager::Instance().GetSceneScale() * 1.0f);

	if (m_state == InteractiveState::Damaged)
	{
		Math::Vector3 pos = m_pos + Math::Vector3{ 0, GetScale().y, 0 };
		Math::Vector3 dir = m_mWorld.Backward();
		auto effect = KdEffekseerManager::GetInstance().Play("paintSpill.efkefc", pos, SceneManager::Instance().GetSceneScale() * 1.0f);
		effect.lock()->SetRotation(dir);
							
		if (auto se = KdAudioManager::Instance().Play3D("Asset/Sound/Intaract/splash.wav", m_pos))
		{
			se->SetVolume(0.4f);
			se->SetCurveDistanceScaler(SceneManager::Instance().GetSceneScale());   // 足音と同様、スケールに合わせる
		}
	}
	else if (m_state == InteractiveState::Normal)
	{
		if (auto se = KdAudioManager::Instance().Play3D("Asset/Sound/Intaract/swab.wav", m_pos))
		{
			se->SetVolume(0.2f);
			se->SetCurveDistanceScaler(SceneManager::Instance().GetSceneScale());
		}
	}
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
