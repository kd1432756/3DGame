#include "DeliveryPointEffect.h"
#include "../../../Scene/SceneManager.h"
#include "../../../System/Location/LocationPointManager.h"
#include "../../Character/Player/Player.h"

void DeliveryPointEffect::Update()
{
	auto player = m_wpPlayer.lock();
	auto point = LocationPointManager::Instance().FindByTag(LocationTag::DeliveryPoint);

	// 危険物を持っている間だけ表示
	const bool show = player && point && player->CountDangerousItems() > 0;

	if (!show)
	{
		if (auto e = m_wpEffect.lock()) { e->StopEffect(); }
		m_wpEffect.reset();
		return;
	}

	const float scale = SceneManager::Instance().GetSceneScale() * 2.0f;
	const Math::Vector3 pos = point->GetPos();

	auto spEffect = m_wpEffect.lock();
	if (!spEffect || !spEffect->IsPlaying())
	{
		m_wpEffect = KdEffekseerManager::GetInstance().Play(
			"deliveryPoint.efkefc", pos, scale, 1.0f, false);
		spEffect = m_wpEffect.lock();
	}

	if (spEffect)
	{
		spEffect->SetWorldMatrix(Math::Matrix::CreateScale(scale) * Math::Matrix::CreateTranslation(pos));
	}
}