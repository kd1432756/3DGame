#include "Ground.h"

void Ground::Init()
{
	StaticMeshObject::Init();
}

void Ground::RegisterCollider()
{
	m_pCollider.reset();
	if (!m_spModel) return;

	m_pCollider = std::make_unique<KdCollider>();

	m_pCollider->RegisterCollisionShape(
		"Ground",
		m_spModel,
		KdCollider::TypeEditor | KdCollider::TypeGround);
}

// Ground.cpp
void Ground::CollectGrassPoints(std::vector<Math::Vector3>& out, float density) const
{
	if (!IsGrass() || !m_spModel) { return; }

	// Groundごとに固定シード(毎回同じ配置になる)
	const Math::Vector3 t = m_mWorld.Translation();
	std::mt19937 rng(
		static_cast<unsigned>(static_cast<int>(t.x * 10.0f)) * 73856093u ^
		static_cast<unsigned>(static_cast<int>(t.z * 10.0f)) * 19349663u);
	std::uniform_real_distribution<float> dist(0.0f, 1.0f);

	const auto& nodes = m_spModel->GetOriginalNodes();

	for (int nodeIdx : m_spModel->GetMeshNodeIndices())
	{
		const auto& node = nodes[nodeIdx];
		if (!node.m_spMesh) { continue; }

		const Math::Matrix mat = node.m_worldTransform * m_mWorld;
		const auto& pos = node.m_spMesh->GetVertexPositions();

		for (const auto& face : node.m_spMesh->GetFaces())
		{
			const Math::Vector3 a = Math::Vector3::Transform(pos[face.Idx[0]], mat);
			const Math::Vector3 b = Math::Vector3::Transform(pos[face.Idx[1]], mat);
			const Math::Vector3 c = Math::Vector3::Transform(pos[face.Idx[2]], mat);

			const Math::Vector3 cross = (b - a).Cross(c - a);
			const float len = cross.Length();
			if (len <= 0.000001f) { continue; }

			// 上向きの面だけ対象(側面・底面は除外)
			if (cross.y / len < 0.7f) { continue; }

			// 面積 × 密度 = 期待本数。小数部は確率で切り上げ
			const float expected = len * 0.5f * density;
			int count = static_cast<int>(expected);
			if (dist(rng) < expected - count) { ++count; }

			for (int i = 0; i < count; ++i)
			{
				float u = dist(rng);
				float v = dist(rng);
				if (u + v > 1.0f) { u = 1.0f - u; v = 1.0f - v; }	// 折り返し

				out.push_back(a + (b - a) * u + (c - a) * v);
			}
		}
	}
}