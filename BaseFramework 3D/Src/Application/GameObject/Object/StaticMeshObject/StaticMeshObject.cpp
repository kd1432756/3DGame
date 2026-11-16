#include "StaticMeshObject.h"

#include "../../../Scene/SceneManager.h"

namespace fs = std::filesystem;

void StaticMeshObject::Init()
{
	if (!m_modelPath.empty())
	{
		ReloadModel();
	}	
}

void StaticMeshObject::DrawLit()
{
	if (!m_spModel) return;
	if (!IsInView()) return;

	KdShaderManager::Instance().m_StandardShader.DrawModel(
		*m_spModel,
		m_mWorld,
		m_color);
}

void StaticMeshObject::GenerateDepthMapFromLight()
{
	if (!m_spModel) return;
	if (!IsInShadowRange()) return;

	KdShaderManager::Instance().m_StandardShader.DrawModel(
		*m_spModel,
		m_mWorld,
		m_color);
}

void StaticMeshObject::SetModelPath(const std::string& path)
{
	if (m_modelPath == path)  return;

	m_modelPath = path;

	ReloadModel();
}

void StaticMeshObject::ReloadModel()
{
	if (m_modelPath.empty()) return;

	if (!fs::exists(m_modelPath))
	{
		return;
	}

	fs::path path(m_modelPath);
	if (path.extension() != ".gltf")
	{
		return;
	}

	if (!m_spModel)
	{
		m_spModel = std::make_shared<KdModelData>();
	}

	m_spModel = KdAssets::Instance().m_modeldatas.GetData(m_modelPath);

	CalcLocalBounds();
	RegisterCollider();
}

void StaticMeshObject::RegisterCollider()
{
	m_pCollider.reset();

	m_pCollider = std::make_unique<KdCollider>();

	UINT type = KdCollider::TypeEditor | KdCollider::TypeBump;

	if (m_isGround)
	{
		type |= KdCollider::TypeGround;
	}

	m_pCollider->RegisterCollisionShape(
		"StaticMeshObject",
		m_spModel,
		type);
}

void StaticMeshObject::SetCollisionEnabled(bool enabled)
{
	m_collisionEnabled = enabled;

	// OFFにした場合はCollider自体を破棄
	if (!m_collisionEnabled)
	{
		m_pCollider.reset();

		return;
	}

	// ONにした場合は再登録
	if (m_spModel)
	{
		RegisterCollider();
	}
}

// ===== 描画関連 =====
void StaticMeshObject::SetCullingCamera(const KdCamera& camera)
{
	// 射影行列 → ビュー空間の視錐台
	DirectX::BoundingFrustum::CreateFromMatrix(s_frustum, camera.GetProjMatrix());

	// カメラのワールド行列でワールド空間へ
	const Math::Matrix mCam = camera.GetCameraMatrix();
	s_frustum.Transform(s_frustum, mCam);

	s_camPos = mCam.Translation();
}
void StaticMeshObject::CalcLocalBounds()
{
	m_hasBounds = false;
	if (!m_spModel) return;

	const auto& nodes = m_spModel->GetOriginalNodes();
	bool first = true;

	for (int idx : m_spModel->GetDrawMeshNodeIndices())
	{
		const auto& node = nodes[idx];
		if (!node.m_spMesh) continue;

		// ノード行列を掛けて、DrawModelの描画位置と一致させる
		DirectX::BoundingBox box;
		node.m_spMesh->GetBoundingBox().Transform(box, node.m_worldTransform);

		if (first) { m_localBox = box; first = false; }
		else { DirectX::BoundingBox::CreateMerged(m_localBox, m_localBox, box); }
	}

	m_hasBounds = !first;
}
bool StaticMeshObject::IsInView() const
{
	if (!m_hasBounds) return true;   // 不明なものは描く(安全側)

	DirectX::BoundingBox worldBox;
	m_localBox.Transform(worldBox, m_mWorld);

	// BoundingBoxを大きくする
	float margin = 3.0f * SceneManager::Instance().GetSceneScale();

	worldBox.Extents.x += margin;
	worldBox.Extents.y += margin;
	worldBox.Extents.z += margin;

	return s_frustum.Intersects(worldBox);
}
bool StaticMeshObject::IsInShadowRange() const
{
	if (!m_hasBounds) return true;

	DirectX::BoundingBox worldBox;
	m_localBox.Transform(worldBox, m_mWorld);

	const Math::Vector3 center(worldBox.Center.x, worldBox.Center.y, worldBox.Center.z);
	const float radius = Math::Vector3(worldBox.Extents.x, worldBox.Extents.y, worldBox.Extents.z).Length();

	return (center - s_camPos).Length() - radius <= s_shadowDistance;
}
// ====================