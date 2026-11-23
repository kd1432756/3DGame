#include "Window.h"

#include "../../../../../Scene/SceneManager.h"

#include "../../../../Character/Player/Player.h"
#include "../../Item/Item.h"

void Window::Init()
{
	SetModelPath("Asset/Models/Props/House/Window.gltf");

	m_iconPath = "Asset/Textures/Icon/Window.png";

	RegisterAction({
		InteractiveState::Normal,
		InteractiveState::Damaged,
		ItemAction::Break,
		"Window",
		8.0f,
		140,
		"Window Break!",
		"Window has been Broken!",
		"Asset/Models/Props/House/WindowBroken.gltf"
		});

	RegisterAction({
		InteractiveState::Damaged,
		InteractiveState::Repaired,
		ItemAction::Repair,
		"Window",
		1.8f,
		140,
		"Window Repair!",
		"",
		"Asset/Models/Props/House/WindowRepaired.gltf"
		});

	StaticMeshObject::Init();
}

void Window::OnAction(const InteractiveAction& action, const std::shared_ptr<Player>& player)
{
	KdEffekseerManager::GetInstance().Play("interact.efkefc", m_pos, SceneManager::Instance().GetSceneScale() * 3.0f);

	if (m_state == InteractiveState::Damaged)
	{
		Math::Vector3 dir = m_mWorld.Backward();
		auto effect = KdEffekseerManager::GetInstance().Play("windowBreak.efkefc", m_pos, SceneManager::Instance().GetSceneScale() * 3.0f);
		effect.lock()->SetRotation(dir);

		if (auto se = KdAudioManager::Instance().Play3D("Asset/Sound/Intaract/glass.wav", m_pos))
		{
			se->SetVolume(0.2f);
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

void Window::RegisterCollider()
{
	if (!m_pCollider)
	{
		m_pCollider = std::make_unique<KdCollider>();
	}

	m_pCollider->RegisterCollisionShape(
		"Window",
		m_spModel,
		KdCollider::TypeEditor | KdCollider::TypeBump | KdCollider::TypeInteract);
}
