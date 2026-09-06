#include "NavMeshManager.h"

//============================================================
// Create
//============================================================

std::shared_ptr<NavMeshPolygon>
NavMeshManager::CreatePolygon(
	const std::vector<Math::Vector3>& vertices)
{
	//--------------------------------------------------------
	// Polygon生成
	//--------------------------------------------------------

	auto polygon =
		std::make_shared<NavMeshPolygon>();

	//--------------------------------------------------------
	// ID
	//--------------------------------------------------------

	polygon->SetID(m_nextID);

	//--------------------------------------------------------
	// Vertex
	//--------------------------------------------------------

	polygon->SetVertices(vertices);

	//--------------------------------------------------------
	// Polygon妥当性チェック
	//--------------------------------------------------------

	if (!polygon->IsValid())
	{
		return nullptr;
	}

	//--------------------------------------------------------
	// 登録
	//--------------------------------------------------------

	m_polygons.push_back(polygon);

	++m_nextID;

	return polygon;
}

//============================================================
// Create with ID
//============================================================

std::shared_ptr<NavMeshPolygon>
NavMeshManager::CreatePolygon(
	int id,
	const std::vector<Math::Vector3>& vertices)
{
	//--------------------------------------------------------
	// IDチェック
	//--------------------------------------------------------

	if (id < 0)
	{
		return nullptr;
	}

	//--------------------------------------------------------
	// ID重複チェック
	//--------------------------------------------------------

	if (FindPolygon(id))
	{
		return nullptr;
	}

	//--------------------------------------------------------
	// Polygon生成
	//--------------------------------------------------------

	auto polygon =
		std::make_shared<NavMeshPolygon>();

	polygon->SetID(id);

	polygon->SetVertices(vertices);

	//--------------------------------------------------------
	// Polygon妥当性チェック
	//--------------------------------------------------------

	if (!polygon->IsValid())
	{
		return nullptr;
	}

	//--------------------------------------------------------
	// 登録
	//--------------------------------------------------------

	m_polygons.push_back(polygon);

	//--------------------------------------------------------
	// 次のIDを更新
	//--------------------------------------------------------

	if (m_nextID <= id)
	{
		m_nextID = id + 1;
	}

	return polygon;
}

//============================================================
// Delete
//============================================================

void NavMeshManager::DeletePolygon(int id)
{
	auto it =
		std::find_if(
			m_polygons.begin(),
			m_polygons.end(),
			[id](
				const std::shared_ptr<NavMeshPolygon>& polygon)
			{
				return polygon->GetID() == id;
			});

	if (it == m_polygons.end())
	{
		return;
	}

	//--------------------------------------------------------
	// 他Polygonから接続解除
	//--------------------------------------------------------

	for (auto& polygon : m_polygons)
	{
		polygon->RemoveConnection(id);
	}

	//--------------------------------------------------------
	// 削除
	//--------------------------------------------------------

	m_polygons.erase(it);
}


//============================================================
// Clear
//============================================================

void NavMeshManager::Clear()
{
	m_polygons.clear();
	m_nextID = 0;

	m_isLoaded = false;
}

//============================================================
// Set Polygon Vertex
//============================================================

void NavMeshManager::SetPolygonVertex(
	int polygonID,
	size_t vertexIndex,
	const Math::Vector3& position)
{
	//--------------------------------------------------------
	// Polygon検索
	//--------------------------------------------------------

	auto polygon =
		FindPolygon(polygonID);

	if (!polygon)
	{
		return;
	}

	//--------------------------------------------------------
	// Indexチェック
	//--------------------------------------------------------

	if (vertexIndex >= polygon->GetVertexCount())
	{
		return;
	}

	//--------------------------------------------------------
	// 元の位置を保存
	//--------------------------------------------------------

	const Math::Vector3 oldPosition =
		polygon->GetVertex(vertexIndex);

	//--------------------------------------------------------
	// 頂点変更
	//--------------------------------------------------------

	polygon->SetVertex(
		vertexIndex,
		position);

	//--------------------------------------------------------
	// Polygon妥当性チェック
	//--------------------------------------------------------

	if (!polygon->IsValid())
	{
		//----------------------------------------------------
		// 不正なら元に戻す
		//----------------------------------------------------

		polygon->SetVertex(
			vertexIndex,
			oldPosition);
	}
}

//============================================================
// Find
//============================================================

std::shared_ptr<NavMeshPolygon>
NavMeshManager::FindPolygon(int id)
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

//============================================================
// Find const
//============================================================

std::shared_ptr<const NavMeshPolygon>
NavMeshManager::FindPolygon(int id) const
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

//============================================================
// Connection
//============================================================

void NavMeshManager::ConnectPolygons(
	int fromID,
	int toID)
{
	//--------------------------------------------------------
	// 自分自身には接続しない
	//--------------------------------------------------------

	if (fromID == toID)
	{
		return;
	}

	//--------------------------------------------------------
	// Polygon検索
	//--------------------------------------------------------

	auto from =
		FindPolygon(fromID);

	auto to =
		FindPolygon(toID);

	if (!from || !to)
	{
		return;
	}

	//--------------------------------------------------------
	// 双方向接続
	//--------------------------------------------------------

	from->AddConnection(toID);
	to->AddConnection(fromID);
}

//============================================================
// Disconnect
//============================================================

void NavMeshManager::DisconnectPolygons(
	int fromID,
	int toID)
{
	//--------------------------------------------------------
	// 自分自身は対象外
	//--------------------------------------------------------

	if (fromID == toID)
	{
		return;
	}

	//--------------------------------------------------------
	// Polygon検索
	//--------------------------------------------------------

	auto from =
		FindPolygon(fromID);

	auto to =
		FindPolygon(toID);

	if (!from || !to)
	{
		return;
	}

	//--------------------------------------------------------
	// 双方向接続解除
	//--------------------------------------------------------

	from->RemoveConnection(toID);
	to->RemoveConnection(fromID);
}

//============================================================
// Find Polygon From Point
//============================================================

std::shared_ptr<NavMeshPolygon>
NavMeshManager::FindPolygonFromPoint(
	const Math::Vector3& point)
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

//============================================================
// Find Polygon From Point const
//============================================================

std::shared_ptr<const NavMeshPolygon>
NavMeshManager::FindPolygonFromPoint(
	const Math::Vector3& point) const
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

