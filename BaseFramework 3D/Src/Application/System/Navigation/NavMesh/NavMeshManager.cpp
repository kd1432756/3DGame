#include "NavMeshManager.h"

std::shared_ptr<NavMeshPolygon> NavMeshManager::CreatePolygon(const std::vector<Math::Vector3>& vertices)
{
	// ポリゴン生成
	auto polygon = std::make_shared<NavMeshPolygon>();

	// ID設定
	polygon->SetID(m_nextID);

	// 頂点設定
	polygon->SetVertices(vertices);

	// ポリゴン妥当性チェック
	if (!polygon->IsValid())
	{
		return nullptr;
	}

	// 登録
	m_polygons.push_back(polygon);

	++m_nextID;

	return polygon;
}

std::shared_ptr<NavMeshPolygon> NavMeshManager::CreatePolygon(int id, const std::vector<Math::Vector3>& vertices)
{
	if (id < 0)
	{
		return nullptr;
	}

	// ID重複チェック
	if (FindPolygon(id))
	{
		return nullptr;
	}

	// ポリゴン生成
	auto polygon = std::make_shared<NavMeshPolygon>();

	// 指定したidで生成
	polygon->SetID(id);

	polygon->SetVertices(vertices);

	// ポリゴン妥当性チェック
	if (!polygon->IsValid())
	{
		return nullptr;
	}

	// 登録
	m_polygons.push_back(polygon);

	if (m_nextID <= id)
	{
		m_nextID = id + 1;
	}

	return polygon;
}

void NavMeshManager::DeletePolygon(int id)
{
	// 指定したIDのポリゴンを検索
	auto it = 
		std::find_if(m_polygons.begin(), m_polygons.end(), [id](const std::shared_ptr<NavMeshPolygon>& polygon)
		{
			return polygon->GetID() == id;
		});

	if (it == m_polygons.end())
	{
		return;
	}

	// 他ポリゴンから接続解除
	for (auto& polygon : m_polygons)
	{
		polygon->RemoveConnection(id);
	}

	// 削除
	m_polygons.erase(it);
}

void NavMeshManager::Clear()
{
	m_polygons.clear();
	m_nextID = 0;

	m_isLoaded = false;
}

void NavMeshManager::SetPolygonVertex(int polygonID, size_t vertexIndex, const Math::Vector3& position)
{
	// ポリゴン検索
	auto polygon = FindPolygon(polygonID);

	if (!polygon)
	{
		return;
	}

	// 頂点Indexチェック
	if (vertexIndex >= polygon->GetVertexCount())
	{
		return;
	}

	// 元の位置を保存
	const Math::Vector3 oldPosition = polygon->GetVertex(vertexIndex);

	// 頂点変更
	polygon->SetVertex(vertexIndex, position);

	// ポリゴン妥当性チェック
	if (!polygon->IsValid())
	{
		// 不正なら元に戻す
		polygon->SetVertex(vertexIndex, oldPosition);
	}
}

std::shared_ptr<NavMeshPolygon> NavMeshManager::FindPolygon(int id)
{
	for (auto& polygon : m_polygons)
	{
		if (polygon->GetID() == id)
		{
			return polygon;
		}
	}

	return nullptr;
}

std::shared_ptr<const NavMeshPolygon> NavMeshManager::FindPolygon(int id) const
{
	for (const auto& polygon : m_polygons)
	{
		if (polygon->GetID() == id)
		{
			return polygon;
		}
	}

	return nullptr;
}

void NavMeshManager::ConnectPolygons(int fromID, int toID)
{
	// 自分自身には接続しない
	if (fromID == toID)
	{
		return;
	}

	// ポリゴン検索
	auto from = FindPolygon(fromID);
	auto to = FindPolygon(toID);

	if (!from || !to)
	{
		return;
	}

	// 双方向接続
	from->AddConnection(toID);
	to->AddConnection(fromID);
}

void NavMeshManager::DisconnectPolygons(int fromID, int toID)
{
	// 自分自身は対象外
	if (fromID == toID)
	{
		return;
	}

	// ポリゴン検索
	auto from = FindPolygon(fromID);
	auto to = FindPolygon(toID);

	if (!from || !to)
	{
		return;
	}

	// 双方向接続解除
	from->RemoveConnection(toID);
	to->RemoveConnection(fromID);
}

std::shared_ptr<NavMeshPolygon> NavMeshManager::FindPolygonFromPoint(const Math::Vector3& point)
{
	for (auto& polygon : m_polygons)
	{
		if (polygon->ContainsPoint(point))
		{
			return polygon;
		}
	}

	return nullptr;
}

std::shared_ptr<const NavMeshPolygon> NavMeshManager::FindPolygonFromPoint(	const Math::Vector3& point) const
{
	for (const auto& polygon : m_polygons)
	{
		if (polygon->ContainsPoint(point))
		{
			return polygon;
		}
	}

	return nullptr;
}

Math::Vector3 NavMeshManager::FindNearestPoint(const Math::Vector3& point, float maxSearchRadius) const
{
	// 初期値設定
	Math::Vector3 bestPoint = point;

	float bestDistSq = FLT_MAX;

	bool found = false;

	// 全ポリゴン調べる
	for (const auto& polygon : m_polygons)
	{
		if (!polygon)
		{
			continue;
		}

		// ポリゴン上の最近点を取得
		Math::Vector3 closest = polygon->GetClosestPoint(point);

		// 指定座標との距離を計算
		Math::Vector3 diff = closest - point;

		diff.y = 0.0f;

		const float distSq = diff.LengthSquared();

		// 最も近い点を更新
		if (distSq < bestDistSq)
		{
			bestDistSq = distSq;

			bestPoint = closest;

			found = true;
		}
	}

	if (!found)
	{
		return point;
	}

	// 検索範囲のチェック
	if (maxSearchRadius > 0.0f && bestDistSq > maxSearchRadius * maxSearchRadius)
	{
		return point;
	}

	return bestPoint;
}