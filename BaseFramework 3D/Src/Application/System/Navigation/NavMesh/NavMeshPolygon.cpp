#include "NavMeshPolygon.h"

bool NavMeshPolygon::IsValid() const
{
	// 頂点数チェック
	if (m_vertices.size() < 3)
	{
		return false;
	}

	// 誤差
	constexpr float EPSILON = 0.0001f;
	constexpr float EPSILON_SQ = EPSILON * EPSILON;

	// 重複頂点チェック
	for (size_t i = 0; i < m_vertices.size(); ++i)
	{
		for (size_t j = i + 1; j < m_vertices.size(); ++j)
		{
			const Math::Vector3 diff = m_vertices[i] - m_vertices[j];

			if (diff.LengthSquared() <= EPSILON_SQ)
			{
				return false;
			}
		}
	}

	// 時計回り方向の曲がりを検出
	bool hasNegative = false;
	// 反時計回り方向の曲がりを検出
	bool hasPositive = false;

	// 凸ポリゴンチェック
	for (size_t i = 0; i < m_vertices.size(); ++i)
	{
		// 連続する三頂点を取得
		const auto& a = m_vertices[i];

		const auto& b = m_vertices[(i + 1) % m_vertices.size()];

		const auto& c = m_vertices[(i + 2) % m_vertices.size()];

		// 外積を計算
		const float cross = (b.x - a.x) * (c.z - b.z) - (b.z - a.z) * (c.x - b.x);

		// ほぼ一直線
		if (std::abs(cross) <= EPSILON)
		{
			return false;
		}

		// 曲がる方向を記録
		if (cross < 0.0f)
		{
			hasNegative = true;
		}
		else
		{
			hasPositive = true;
		}

		// 符号が混在したら凹ポリゴン
		if (hasNegative && hasPositive)
		{
			return false;
		}
	}
	return true;
}

bool NavMeshPolygon::ContainsPoint(const Math::Vector3& point) const
{
	// 頂点数チェック
	if (m_vertices.size() < 3)
	{
		return false;
	}

	// 誤差
	constexpr float EPSILON = 0.0001f;

	const float px = point.x;
	const float pz = point.z;

	// 時計回り方向の曲がりを検出
	bool hasNegative = false;
	// 反時計回り方向の曲がりを検出
	bool hasPositive = false;

	// 各辺について符号を確認
	for (size_t i = 0; i < m_vertices.size(); ++i)
	{
		// 頂点を取得
		const auto& a = m_vertices[i];

		const auto& b = m_vertices[(i + 1) % m_vertices.size()];

		// 外積を計算
		const float cross = (b.x - a.x) * (pz - a.z) - (b.z - a.z) * (px - a.x);

		// 辺のどちら側にあるか
		if (cross < -EPSILON)
		{
			hasNegative = true;
		}
		else if (cross > EPSILON)
		{
			hasPositive = true;
		}

		// 凸ポリゴンで符号が混在したら外側
		if (hasNegative && hasPositive)
		{
			return false;
		}
	}

	// 内側か境界上
	return true;
}

Math::Vector3 NavMeshPolygon::GetClosestPoint(const Math::Vector3& point) const
{
	// ポリゴン内ならそのまま返す
	if (ContainsPoint(point))
	{
		return point;
	}

	if (m_vertices.size() < 2)
	{
		return m_center;
	}

	// 最近点の初期化
	Math::Vector3 bestPoint = m_vertices[0];

	float bestDistSq = FLT_MAX;

	const size_t vertexCount = m_vertices.size();

	for (size_t i = 0; i < vertexCount; ++i)
	{
		// ポリゴンの各辺を取得
		const Math::Vector3& a = m_vertices[i];

		const Math::Vector3& b = m_vertices[(i + 1) % vertexCount];

		// 辺のベクトル
		Math::Vector3 edge = b - a;

		edge.y = 0.0f;

		// 辺の長さを計算
		const float edgeLengthSq = edge.x * edge.x + edge.z * edge.z;

		Math::Vector3 closestOnEdge;

		// 辺と指定座標がほぼ同じ位置
		if (edgeLengthSq <= 0.000001f)
		{
			closestOnEdge = a;
		}
		else
		{
			// 辺の始点から指定座標へのベクトル
			Math::Vector3 toPoint = point - a;

			toPoint.y = 0.0f;

			// 内積から、射影係数を計算
			float t = (toPoint.x * edge.x + toPoint.z * edge.z) / edgeLengthSq;

			t = std::clamp(t, 0.0f, 1.0f);

			// 辺上の最近点を計算
			closestOnEdge = a + edge * t;
		}

		// 距離を比較
		Math::Vector3 diff = closestOnEdge - point;

		diff.y = 0.0f;

		const float distSq = diff.LengthSquared();

		if (distSq < bestDistSq)
		{
			bestDistSq = distSq;

			bestPoint = closestOnEdge;
		}
	}

	// 最近点を取得
	return bestPoint;
}

bool NavMeshPolygon::SharesEdgeWith(const NavMeshPolygon& other) const
{
	Math::Vector3 start;
	Math::Vector3 end;

	return GetSharedEdge(other, start, end);
}

bool NavMeshPolygon::GetSharedEdge(
	const NavMeshPolygon& other,
	Math::Vector3& outStart,
	Math::Vector3& outEnd) const
{
	if (m_vertices.size() < 3 || other.GetVertexCount() < 3)
	{
		return false;
	}

	// 誤差
	constexpr float EPSILON = 0.01f;
	constexpr float EPSILON_SQ = EPSILON * EPSILON;

	// 初期化
	outStart = Math::Vector3::Zero;
	outEnd = Math::Vector3::Zero;

	for (size_t i = 0; i < m_vertices.size(); ++i)
	{
		// ポリゴンの辺を取得
		const Math::Vector3& a1 = m_vertices[i];

		const Math::Vector3& a2 = m_vertices[(i + 1) % m_vertices.size()];

		// A側の辺の方向と長さを計算
		const float ax = a2.x - a1.x;

		const float az = a2.z - a1.z;

		const float aLengthSq = ax * ax + az * az;

		if (aLengthSq <= EPSILON_SQ)
		{
			continue;
		}

		// 辺の長さを求める
		const float aLength = std::sqrt(aLengthSq);

		// 単位ベクトルを求める
		const float dx = ax / aLength;

		const float dz = az / aLength;

		for (size_t j = 0; j < other.GetVertexCount(); ++j)
		{
			// ポリゴンの辺を取得
			const Math::Vector3& b1 = other.GetVertex(j);

			const Math::Vector3& b2 = other.GetVertex((j + 1) % other.GetVertexCount());

			// B側の辺の方向と長さを計算
			const float bx = b2.x - b1.x;

			const float bz = b2.z - b1.z;

			const float bLengthSq = bx * bx + bz * bz;

			if (bLengthSq <= EPSILON_SQ)
			{
				continue;
			}

			// 2辺の外積を計算
			const float cross = ax * bz - az * bx;

			// 平衡か
			if (std::abs(cross) > EPSILON * std::sqrt(aLengthSq * bLengthSq))
			{
				continue;
			}

			// B側の始点とAの外積
			const float cross1 = ax * (b1.z - a1.z) - az * (b1.x - a1.x);

			// B側の終点とAの外積
			const float cross2 = ax * (b2.z - a1.z) - az * (b2.x - a1.x);

			// 同一直線上かチェック
			if (std::abs(cross1) > EPSILON * aLength)
			{
				continue;
			}
			if (std::abs(cross2) > EPSILON * aLength)
			{
				continue;
			}

			// A側の始点を0
			const float aStart = 0.0f;

			// A側の終点を長さ
			const float aEnd = aLength;

			// 内積で投影
			const float bStart = (b1.x - a1.x) * dx + (b1.z - a1.z) * dz;

			const float bEnd = (b2.x - a1.x) * dx + (b2.z - a1.z) * dz;

			// B側で投影範囲を計算
			const float bMin = std::min(bStart, bEnd);

			const float bMax = std::max(bStart, bEnd);

			// 共有している範囲
			const float overlapMin = std::max(aStart, bMin);

			const float overlapMax = std::min(aEnd, bMax);

			// 共有部分がない
			if (overlapMax - overlapMin <= EPSILON)
			{
				continue;
			}

			// 共有部分の始点
			outStart = a1 + Math::Vector3(dx, 0.0f, dz) * overlapMin;

			// 共有部分の終点
			outEnd = a1 + Math::Vector3(dx, 0.0f, dz) * overlapMax;

			// Y座標を補間
			const float tStart = overlapMin / aLength;

			const float tEnd = overlapMax / aLength;

			outStart.y = a1.y + (a2.y - a1.y) * tStart;

			outEnd.y = a1.y + (a2.y - a1.y) * tEnd;

			return true;
		}
	}

	return false;
}