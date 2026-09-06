#include "VisionLight.h"

VisionLight::VisionLight()
{
	Set2DObject(false);

	CreateVertices();
}

//============================================================
// 視界設定
//============================================================
void VisionLight::SetVision(float distance, float angleDeg)
{
	m_distance = std::max(0.0f, distance);

	// 0～360度に制限
	m_angleDeg = std::clamp(angleDeg, 0.0f, 360.0f);

	CreateVertices();
}


//============================================================
// 視界距離変更
//============================================================
void VisionLight::SetDistance(float distance)
{
	m_distance = std::max(0.0f, distance);

	CreateVertices();
}


//============================================================
// 視野角変更
//============================================================
void VisionLight::SetAngle(float angleDeg)
{
	m_angleDeg = std::clamp(angleDeg, 0.0f, 360.0f);

	CreateVertices();
}


//============================================================
// 分割数変更
//============================================================
void VisionLight::SetDivision(UINT division)
{
	// 少なすぎると扇形にならないので最低4
	m_division = std::max(4u, division);

	CreateVertices();
}


//============================================================
// 色変更
//============================================================
void VisionLight::SetVisionColor(const Math::Color& color)
{
	m_color = color;
}


//============================================================
// 扇形の頂点作成
//============================================================
//
// KdStandardShader::DrawPolygon() は
//
//     D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP
//
// で描画している。
//
// そのため、
//
//     中心
//     外周1
//     中心
//     外周2
//     中心
//     外周3
//
// のように「中心を交互に挟む」ことで、
// 扇形を三角形の集合として作る。
//
// ローカル +Z が敵の正面。
// XZ平面上に作成する。
//============================================================
void VisionLight::CreateVertices()
{
	m_vertices.clear();

	// 距離が0なら描画する必要なし
	if (m_distance <= 0.0f)
	{
		return;
	}

	// 角度をラジアンへ変換
	const float angleRad =
		DirectX::XMConvertToRadians(m_angleDeg);

	const float halfAngle = angleRad * 0.5f;

	// 中心点
	KdPolygon::Vertex center;

	center.pos = {
		0.0f,
		m_height,
		0.0f
	};

	center.UV = { 0.5f, 0.5f };

	center.color = 0xFFFFFFFF;

	center.normal = Math::Vector3::Up;
	center.tangent = Math::Vector3::Right;

	// 扇形の各三角形を作る
	//
	// TRIANGLESTRIP用なので
	//
	// center
	// outer0
	// center
	// outer1
	// center
	// outer2
	//
	// とする
	//
	m_vertices.reserve(m_division * 2 + 2);

	for (UINT i = 0; i <= m_division; ++i)
	{
		float rate =
			static_cast<float>(i) /
			static_cast<float>(m_division);

		// -半角 ～ +半角
		float angle =
			-halfAngle +
			angleRad * rate;

		// +Z方向を正面とする
		Math::Vector3 pos;

		pos.x = std::sin(angle) * m_distance;
		pos.y = m_height;
		pos.z = std::cos(angle) * m_distance;

		KdPolygon::Vertex outer;

		outer.pos = pos;
		outer.UV = { 0.5f, 0.5f };

		outer.color = 0xFFAAAAAA;

		outer.normal = Math::Vector3::Up;
		outer.tangent = Math::Vector3::Right;

		// 中心
		m_vertices.push_back(center);

		// 外周
		m_vertices.push_back(outer);
	}
}