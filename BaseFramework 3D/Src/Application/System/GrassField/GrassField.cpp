#include "GrassField.h"

#include "../../GameObject/Object/StaticMeshObject/Ground/Ground.h"

#include "../../Scene/SceneManager.h"

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

	// 2. 障害物チェック(Sphere)
	KdCollider::SphereInfo sphere;
	sphere.m_sphere.Radius = obstacleRadius;
	sphere.m_type = KdCollider::TypeBump;

	std::mt19937 rng(12345);
	std::uniform_real_distribution<float> yaw(0.0f, DirectX::XM_2PI);
	const float baseScale = 0.2f;
	std::uniform_real_distribution<float> scale(0.8f, 1.2f);

	for (const auto& p : points)
	{
		// 地面に埋まらないよう、半径ぶん持ち上げる
		sphere.m_sphere.Center = p + Math::Vector3(0, obstacleRadius + 0.05f, 0);

		std::list<KdCollider::CollisionResult> hit;
		for (auto& obj : objList)
		{
			if (!obj) { continue; }
			obj->Intersects(sphere, &hit);
			if (!hit.empty()) { break; }	// 1つ当たれば十分
		}
		if (!hit.empty()) { continue; }

		// 3. 行列を作る(ランダムな向きと大きさ)
		float s = scale(rng) * baseScale * SceneManager::Instance().GetSceneScale();

		m_instances.push_back(
			Math::Matrix::CreateScale(s) *
			Math::Matrix::CreateRotationY(yaw(rng)) *
			Math::Matrix::CreateTranslation(p));
	}
}

void GrassField::DrawLit()
{
	if (!m_model) { return; }

	auto& sm = KdShaderManager::Instance();
	sm.ChangeRasterizerState(KdRasterizerState::CullNone);

	for (const auto& m : m_instances)
	{
		sm.m_StandardShader.SetWindEffectEnable(true);
		sm.m_StandardShader.DrawModel(*m_model, m);
	}

	sm.UndoRasterizerState();
}