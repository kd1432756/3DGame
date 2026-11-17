#include "GrassField.h"

#include "../../GameObject/Object/StaticMeshObject/Ground/Ground.h"

#include "../../Scene/SceneManager.h"

#include "../../GameObject/Character/Player/Player.h"

void GrassField::Generate(float density, float obstacleRadius)
{
	m_model = std::make_shared<KdModelData>();
	m_model->Load("Asset/Models/Grass/Grass.gltf");

	m_instances.clear();

	const auto& objList = SceneManager::Instance().GetObjList();

	// 1. 候補点を集める
	std::vector<Math::Vector3> points;
	for (auto& obj : objList)
	{
		if (auto ground = std::dynamic_pointer_cast<Ground>(obj))
		{
			ground->CollectGrassPoints(points, density);
		}
	}

	// 2. 障害物チェック
	KdCollider::SphereInfo sphere;
	sphere.m_sphere.Radius = obstacleRadius;
	sphere.m_type = KdCollider::TypeBump;

	const float baseScale = 0.2f;
	const float sceneScale = SceneManager::Instance().GetSceneScale();

	for (const auto& p : points)
	{
		// 地面に埋まらないよう、半径ぶん持ち上げる
		sphere.m_sphere.Center = p + Math::Vector3(0, obstacleRadius + 0.05f, 0);

		std::list<KdCollider::CollisionResult> hit;
		for (auto& obj : objList)
		{
			if (!obj) { continue; }
			obj->Intersects(sphere, &hit);
			if (!hit.empty()) { break; }
		}
		if (!hit.empty()) { continue; }

		// 3. ランダムな大きさと向きを設定
		const float scale = baseScale *
			KdRandom::GetFloat(0.8f, 1.2f) *
			sceneScale;

		const float yaw = KdRandom::GetFloat(0.0f, DirectX::XM_2PI);

		m_instances.push_back(
			Math::Matrix::CreateScale(scale) *
			Math::Matrix::CreateRotationY(yaw) *
			Math::Matrix::CreateTranslation(p)
		);
	}
}

void GrassField::DrawLit()
{
	if (!m_model) { return; }

	bool useCull = false;
	Math::Vector3 center;
	if (auto player = m_wpPlayer.lock())
	{
		center = player->GetPos();
		useCull = true;
	}

	const float distSq = m_drawDistance * m_drawDistance;

	auto& sm = KdShaderManager::Instance();
	sm.ChangeRasterizerState(KdRasterizerState::CullNone);

	for (const auto& m : m_instances)
	{
		if (useCull)
		{
			const Math::Vector3 p = m.Translation();
			const float dx = p.x - center.x;
			const float dz = p.z - center.z;
			if (dx * dx + dz * dz > distSq) { continue; }
		}

		sm.m_StandardShader.SetWindEffectEnable(true);
		sm.m_StandardShader.DrawModel(*m_model, m);
	}

	sm.UndoRasterizerState();
}