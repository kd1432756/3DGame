#include "NavMeshPolygon.h"

//============================================================
// Polygon Valid
//============================================================
//
// 以下をチェックする。
//
// ・頂点数が3以上
// ・重複頂点がない
// ・3点が一直線ではない
// ・凸Polygonである
//
// NavMeshはXZ平面を使用する。
//============================================================

bool NavMeshPolygon::IsValid() const
{
	//--------------------------------------------------------
	// 頂点数
	//--------------------------------------------------------

	if (m_vertices.size() < 3)
	{
		return false;
	}

	constexpr float EPSILON = 0.0001f;
	constexpr float EPSILON_SQ =
		EPSILON * EPSILON;

	//--------------------------------------------------------
	// 重複頂点チェック
	//--------------------------------------------------------

	for (size_t i = 0;
		i < m_vertices.size();
		++i)
	{
		for (size_t j = i + 1;
			j < m_vertices.size();
			++j)
		{
			const Math::Vector3 diff =
				m_vertices[i] - m_vertices[j];

			if (diff.LengthSquared() <= EPSILON_SQ)
			{
				return false;
			}
		}
	}

	//--------------------------------------------------------
	// 凸Polygonチェック
	//--------------------------------------------------------

	bool hasNegative = false;
	bool hasPositive = false;

	for (size_t i = 0;
		i < m_vertices.size();
		++i)
	{
		const auto& a =
			m_vertices[i];

		const auto& b =
			m_vertices[
				(i + 1) % m_vertices.size()
			];

		const auto& c =
			m_vertices[
				(i + 2) % m_vertices.size()
			];

		const float cross =
			(b.x - a.x) * (c.z - b.z)
			-
			(b.z - a.z) * (c.x - b.x);

		//----------------------------------------------------
		// ほぼ一直線
		//----------------------------------------------------

		if (std::abs(cross) <= EPSILON)
		{
			return false;
		}

		if (cross < 0.0f)
		{
			hasNegative = true;
		}
		else
		{
			hasPositive = true;
		}

		//----------------------------------------------------
		// 符号が混在したら凹Polygon
		//----------------------------------------------------

		if (hasNegative && hasPositive)
		{
			return false;
		}
	}

	return true;
}


//============================================================
// PointがPolygon内にあるか
//============================================================
//
// XZ平面上で判定する。
// NavMeshがXZ平面を歩行面としている前提。
//
//============================================================

bool NavMeshPolygon::ContainsPoint(
	const Math::Vector3& point) const
{
	if (m_vertices.size() < 3)
	{
		return false;
	}

	constexpr float EPSILON = 0.0001f;

	const float px = point.x;
	const float pz = point.z;

	bool hasNegative = false;
	bool hasPositive = false;

	//--------------------------------------------------------
	// 各辺について符号を確認
	//--------------------------------------------------------

	for (size_t i = 0;
		i < m_vertices.size();
		++i)
	{
		const auto& a =
			m_vertices[i];

		const auto& b =
			m_vertices[
				(i + 1) % m_vertices.size()
			];

		const float cross =
			(b.x - a.x) * (pz - a.z)
			-
			(b.z - a.z) * (px - a.x);

		if (cross < -EPSILON)
		{
			hasNegative = true;
		}
		else if (cross > EPSILON)
		{
			hasPositive = true;
		}

		//----------------------------------------------------
		// 凸Polygonで符号が混在したら外側
		//----------------------------------------------------

		if (hasNegative && hasPositive)
		{
			return false;
		}
	}

	return true;
}

//============================================================
// GetClosestPoint
//============================================================
//
// XZ平面上で、指定Pointからこのポリゴンへの最近傍点を求める。
// ポリゴン内ならそのまま、外なら一番近い辺上の点を返す。
//
//============================================================

Math::Vector3 NavMeshPolygon::GetClosestPoint(
	const Math::Vector3& point) const
{
	//--------------------------------------------------------
	// ポリゴン内ならそのまま返す
	//--------------------------------------------------------

	if (ContainsPoint(point))
	{
		return point;
	}

	//--------------------------------------------------------
	// 頂点が2つ未満なら計算不可
	//--------------------------------------------------------

	if (m_vertices.size() < 2)
	{
		return m_center;
	}

	//--------------------------------------------------------
	// 各辺への最近点を総当たり
	//--------------------------------------------------------

	Math::Vector3 bestPoint = m_vertices[0];

	float bestDistSq = FLT_MAX;

	const size_t vertexCount = m_vertices.size();

	for (size_t i = 0; i < vertexCount; ++i)
	{
		const Math::Vector3& a = m_vertices[i];

		const Math::Vector3& b =
			m_vertices[(i + 1) % vertexCount];

		//------------------------------------------------
		// 辺 a → b への射影（XZ平面）
		//------------------------------------------------

		Math::Vector3 edge = b - a;

		edge.y = 0.0f;

		const float edgeLengthSq =
			edge.x * edge.x + edge.z * edge.z;

		Math::Vector3 closestOnEdge;

		if (edgeLengthSq <= 0.000001f)
		{
			closestOnEdge = a;
		}
		else
		{
			Math::Vector3 toPoint = point - a;

			toPoint.y = 0.0f;

			float t =
				(toPoint.x * edge.x + toPoint.z * edge.z) /
				edgeLengthSq;

			t = std::clamp(t, 0.0f, 1.0f);

			closestOnEdge = a + edge * t;
		}

		//------------------------------------------------
		// XZ距離で比較
		//------------------------------------------------

		Math::Vector3 diff = closestOnEdge - point;

		diff.y = 0.0f;

		const float distSq = diff.LengthSquared();

		if (distSq < bestDistSq)
		{
			bestDistSq = distSq;

			bestPoint = closestOnEdge;
		}
	}

	return bestPoint;
}

//============================================================
// 2つのPolygonが辺を共有しているか
//============================================================
//
// XZ平面上で辺同士が同一直線上にあり、一定以上重なっている
//
//============================================================

bool NavMeshPolygon::SharesEdgeWith(
	const NavMeshPolygon& other) const
{
	Math::Vector3 start;
	Math::Vector3 end;

	return GetSharedEdge(
		other,
		start,
		end);
}

/*bool NavMeshPolygon::SharesEdgeWith(
	const NavMeshPolygon& other) const
{
	if (m_vertices.size() < 3 ||
		other.GetVertexCount() < 3)
	{
		return false;
	}

	constexpr float EPSILON = 0.01f;

	//--------------------------------------------------------
	// Edge overlap
	//--------------------------------------------------------

	auto IsOverlappingEdge =
		[EPSILON](
			const Math::Vector3& a1,
			const Math::Vector3& a2,
			const Math::Vector3& b1,
			const Math::Vector3& b2)
		{
			//------------------------------------------------
			// 方向ベクトル
			//------------------------------------------------

			const float ax =
				a2.x - a1.x;

			const float az =
				a2.z - a1.z;

			const float bx =
				b2.x - b1.x;

			const float bz =
				b2.z - b1.z;

			//------------------------------------------------
			// 短すぎる辺
			//------------------------------------------------

			const float aLengthSq =
				ax * ax + az * az;

			const float bLengthSq =
				bx * bx + bz * bz;

			if (aLengthSq <= EPSILON * EPSILON ||
				bLengthSq <= EPSILON * EPSILON)
			{
				return false;
			}

			//------------------------------------------------
			// 平行チェック
			//------------------------------------------------

			const float cross =
				ax * bz -
				az * bx;

			if (std::abs(cross) >
				EPSILON *
				std::sqrt(aLengthSq * bLengthSq))
			{
				return false;
			}

			//------------------------------------------------
			// 同一直線上かチェック
			//------------------------------------------------

			const float cross1 =
				ax * (b1.z - a1.z) -
				az * (b1.x - a1.x);

			const float cross2 =
				ax * (b2.z - a1.z) -
				az * (b2.x - a1.x);

			if (std::abs(cross1) >
				EPSILON * std::sqrt(aLengthSq))
			{
				return false;
			}

			if (std::abs(cross2) >
				EPSILON * std::sqrt(aLengthSq))
			{
				return false;
			}

			//------------------------------------------------
			// Aの辺方向に投影
			//------------------------------------------------

			const float aLength =
				std::sqrt(aLengthSq);

			const float dx =
				ax / aLength;

			const float dz =
				az / aLength;

			const float aStart = 0.0f;
			const float aEnd = aLength;

			const float bStart =
				(b1.x - a1.x) * dx +
				(b1.z - a1.z) * dz;

			const float bEnd =
				(b2.x - a1.x) * dx +
				(b2.z - a1.z) * dz;

			const float bMin =
				std::min(bStart, bEnd);

			const float bMax =
				std::max(bStart, bEnd);

			//------------------------------------------------
			// 区間の重なり
			//------------------------------------------------

			const float overlapMin =
				std::max(aStart, bMin);

			const float overlapMax =
				std::min(aEnd, bMax);

			//------------------------------------------------
			// 一定以上重なっている
			//------------------------------------------------

			return
				(overlapMax - overlapMin)
	> EPSILON;
		};

	//--------------------------------------------------------
	// 全Edge比較
	//--------------------------------------------------------

	for (size_t i = 0;
		i < m_vertices.size();
		++i)
	{
		const auto& a1 =
			m_vertices[i];

		const auto& a2 =
			m_vertices[
				(i + 1) % m_vertices.size()
			];

		for (size_t j = 0;
			j < other.GetVertexCount();
			++j)
		{
			const auto& b1 =
				other.GetVertex(j);

			const auto& b2 =
				other.GetVertex(
					(j + 1) %
					other.GetVertexCount());

			if (IsOverlappingEdge(
				a1,
				a2,
				b1,
				b2))
			{
				return true;
			}
		}
	}

	return false;
}*/

//============================================================
// Shared Edge取得
//============================================================
//
// 2つのPolygonが共有しているEdge部分を取得する。
//
// ・XZ平面で判定
// ・完全一致でなくてもOK
// ・部分的な重なりにも対応
//
// outStart / outEnd:
//    実際に共有しているEdgeの両端
//
//============================================================

bool NavMeshPolygon::GetSharedEdge(
	const NavMeshPolygon& other,
	Math::Vector3& outStart,
	Math::Vector3& outEnd) const
{
	if (m_vertices.size() < 3 ||
		other.GetVertexCount() < 3)
	{
		return false;
	}

	constexpr float EPSILON = 0.01f;
	constexpr float EPSILON_SQ =
		EPSILON * EPSILON;

	//--------------------------------------------------------
	// 出力初期化
	//--------------------------------------------------------

	outStart = Math::Vector3::Zero;
	outEnd = Math::Vector3::Zero;

	//--------------------------------------------------------
	// 全Edge比較
	//--------------------------------------------------------

	for (size_t i = 0;
		i < m_vertices.size();
		++i)
	{
		const Math::Vector3& a1 =
			m_vertices[i];

		const Math::Vector3& a2 =
			m_vertices[
				(i + 1) % m_vertices.size()
			];

		//----------------------------------------------------
		// A Edge方向
		//----------------------------------------------------

		const float ax =
			a2.x - a1.x;

		const float az =
			a2.z - a1.z;

		const float aLengthSq =
			ax * ax +
			az * az;

		if (aLengthSq <= EPSILON_SQ)
		{
			continue;
		}

		const float aLength =
			std::sqrt(aLengthSq);

		const float dx =
			ax / aLength;

		const float dz =
			az / aLength;

		//----------------------------------------------------
		// A Edgeの法線方向
		//
		// 同一直線上か判定するために使用
		//----------------------------------------------------

		for (size_t j = 0;
			j < other.GetVertexCount();
			++j)
		{
			const Math::Vector3& b1 =
				other.GetVertex(j);

			const Math::Vector3& b2 =
				other.GetVertex(
					(j + 1) %
					other.GetVertexCount());

			//------------------------------------------------
			// B Edge方向
			//------------------------------------------------

			const float bx =
				b2.x - b1.x;

			const float bz =
				b2.z - b1.z;

			const float bLengthSq =
				bx * bx +
				bz * bz;

			if (bLengthSq <= EPSILON_SQ)
			{
				continue;
			}

			//------------------------------------------------
			// 平行チェック
			//------------------------------------------------

			const float cross =
				ax * bz -
				az * bx;

			if (std::abs(cross) >
				EPSILON *
				std::sqrt(
					aLengthSq *
					bLengthSq))
			{
				continue;
			}

			//------------------------------------------------
			// 同一直線上かチェック
			//------------------------------------------------

			const float cross1 =
				ax * (b1.z - a1.z) -
				az * (b1.x - a1.x);

			const float cross2 =
				ax * (b2.z - a1.z) -
				az * (b2.x - a1.x);

			if (std::abs(cross1) >
				EPSILON * aLength)
			{
				continue;
			}

			if (std::abs(cross2) >
				EPSILON * aLength)
			{
				continue;
			}

			//------------------------------------------------
			// A Edge上へBの端点を投影
			//------------------------------------------------

			const float aStart =
				0.0f;

			const float aEnd =
				aLength;

			const float bStart =
				(b1.x - a1.x) * dx +
				(b1.z - a1.z) * dz;

			const float bEnd =
				(b2.x - a1.x) * dx +
				(b2.z - a1.z) * dz;

			//------------------------------------------------
			// B Edgeの投影範囲
			//------------------------------------------------

			const float bMin =
				std::min(
					bStart,
					bEnd);

			const float bMax =
				std::max(
					bStart,
					bEnd);

			//------------------------------------------------
			// 重なり範囲
			//------------------------------------------------

			const float overlapMin =
				std::max(
					aStart,
					bMin);

			const float overlapMax =
				std::min(
					aEnd,
					bMax);

			//------------------------------------------------
			// 共有部分がない
			//------------------------------------------------

			if (overlapMax - overlapMin <=
				EPSILON)
			{
				continue;
			}

			//------------------------------------------------
			// A Edge上の共有点を取得
			//------------------------------------------------

			outStart =
				a1 +
				Math::Vector3(
					dx,
					0.0f,
					dz) *
				overlapMin;

			outEnd =
				a1 +
				Math::Vector3(
					dx,
					0.0f,
					dz) *
				overlapMax;

			//------------------------------------------------
			// Y座標
			//------------------------------------------------
			//
			// A側のEdge上の座標を使用する。
			//
			// NavMeshでは同一の接続面を想定しているため、
			// 基本的には問題ない。
			//
			//------------------------------------------------

			const float tStart =
				overlapMin /
				aLength;

			const float tEnd =
				overlapMax /
				aLength;

			outStart.y =
				a1.y +
				(a2.y - a1.y) *
				tStart;

			outEnd.y =
				a1.y +
				(a2.y - a1.y) *
				tEnd;

			return true;
		}
	}

	return false;
}
