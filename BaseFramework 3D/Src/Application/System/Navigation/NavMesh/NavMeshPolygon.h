#pragma once

class NavMeshManager;

//============================================================
// NavMesh Polygon
//============================================================
//
// ・GameObjectではない
// ・SceneManagerには登録しない
// ・専用JSONに保存する
//
// 1つのNavMeshPolygon = 1つの凸ポリゴン
//
//============================================================

class NavMeshPolygon
{
	friend class NavMeshManager;

public:

	//============================================================
	// ID
	//============================================================

	int GetID() const
	{
		return m_id;
	}

	//============================================================
	// Vertex
	//============================================================

	size_t GetVertexCount() const
	{
		return m_vertices.size();
	}

	const Math::Vector3& GetVertex(size_t index) const
	{
		assert(index < m_vertices.size());

		return m_vertices[index];
	}

	const std::vector<Math::Vector3>& GetVertices() const
	{
		return m_vertices;
	}

	//============================================================
	// Vertex Set
	//============================================================
	
	// 頂点設定
	void SetVertex(
		size_t index,
		const Math::Vector3& pos)
	{
		if (index >= m_vertices.size())
		{
			return;
		}

		m_vertices[index] = pos;

		CalculateCenter();
	}

	// 頂点追加
	void AddVertex(
		const Math::Vector3& pos)
	{
		m_vertices.push_back(pos);

		CalculateCenter();
	}

	// 全頂点設定
	void SetVertices(
		const std::vector<Math::Vector3>& vertices)
	{
		m_vertices = vertices;

		CalculateCenter();
	}

	// 頂点クリア
	void ClearVertices()
	{
		m_vertices.clear();

		CalculateCenter();
	}

	//============================================================
	// Center
	//============================================================

	const Math::Vector3& GetCenter() const
	{
		return m_center;
	}

	void CalculateCenter()
	{
		if (m_vertices.empty())
		{
			m_center = Math::Vector3::Zero;
			return;
		}

		m_center = Math::Vector3::Zero;

		for (const auto& vertex : m_vertices)
		{
			m_center += vertex;
		}

		m_center /=
			static_cast<float>(m_vertices.size());
	}

	//============================================================
	// Connection
	//============================================================

	const std::vector<int>& GetConnections() const
	{
		return m_connections;
	}

	size_t GetConnectionCount() const
	{
		return m_connections.size();
	}

	bool HasConnection(int polygonID) const
	{
		return std::find(
			m_connections.begin(),
			m_connections.end(),
			polygonID)
			!= m_connections.end();
	}

	//============================================================
	// Polygon
	//============================================================

	// Polygonが有効か
	bool IsValid() const;

	// 他のPolygonと辺を共有しているか
	bool SharesEdgeWith(
		const NavMeshPolygon& other) const;

	// 2つのPolygonが共有しているEdge部分を取得する
	bool GetSharedEdge(
		const NavMeshPolygon& other,
		Math::Vector3& outStart,
		Math::Vector3& outEnd) const;

	//============================================================
	// Point
	//============================================================

	// 指定したPointがPolygon内にあるか
	bool ContainsPoint(
		const Math::Vector3& point) const;

private:

	//============================================================
	// ID
	//============================================================

	int m_id = -1;

	//============================================================
	// Polygon
	//============================================================

	std::vector<Math::Vector3> m_vertices;

	//============================================================
	// Center
	//============================================================

	Math::Vector3 m_center =
		Math::Vector3::Zero;

	//============================================================
	// Connections
	//============================================================

	std::vector<int> m_connections;

	//============================================================
	// Manager専用
	//============================================================

	void SetID(int id)
	{
		m_id = id;
	}

	void AddConnection(int polygonID)
	{
		// 自分自身には接続しない
		if (polygonID == m_id)
		{
			return;
		}

		// 二重登録防止
		if (HasConnection(polygonID))
		{
			return;
		}

		m_connections.push_back(polygonID);
	}

	void RemoveConnection(int polygonID)
	{
		m_connections.erase(
			std::remove(
				m_connections.begin(),
				m_connections.end(),
				polygonID),
			m_connections.end());
	}
};
