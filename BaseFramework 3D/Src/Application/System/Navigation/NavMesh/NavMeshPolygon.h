#pragma once

class NavMeshManager;

class NavMeshPolygon
{
	// NavMeshManagerのみprivateメンバーへアクセス可能
	friend class NavMeshManager;

public:

	// ID
	int GetID() const
	{
		return m_id;
	}

	// 頂点数の取得
	size_t GetVertexCount() const
	{
		return m_vertices.size();
	}

	// 頂点の取得
	const Math::Vector3& GetVertex(size_t index) const
	{
		assert(index < m_vertices.size());

		return m_vertices[index];
	}

	// 頂点群の取得
	const std::vector<Math::Vector3>& GetVertices() const
	{
		return m_vertices;
	}

	// 頂点設定
	void SetVertex(size_t index, const Math::Vector3& pos)
	{
		if (index >= m_vertices.size())
		{
			return;
		}

		m_vertices[index] = pos;

		CalculateCenter();
	}

	// 頂点追加
	void AddVertex(const Math::Vector3& pos)
	{
		m_vertices.push_back(pos);

		CalculateCenter();
	}

	// 全頂点設定
	void SetVertices(const std::vector<Math::Vector3>& vertices)
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

	// ポリゴンの中心座標取得
	const Math::Vector3& GetCenter() const
	{
		return m_center;
	}

	// ポリゴンの中心座標計算
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

		m_center /= static_cast<float>(m_vertices.size());
	}

	// 接続ポリゴン取得
	const std::vector<int>& GetConnections() const
	{
		return m_connections;
	}

	// 接続ポリゴン数取得
	size_t GetConnectionCount() const
	{
		return m_connections.size();
	}

	// 接続しているか
	bool HasConnection(int polygonID) const
	{
		return std::find(
			m_connections.begin(),
			m_connections.end(),
			polygonID)
			!= m_connections.end();
	}


	// ポリゴンが有効か
	bool IsValid() const;

	// 他のポリゴンと辺を共有しているか
	bool SharesEdgeWith(const NavMeshPolygon& other) const;

	// 2つのポリゴンが共有している辺を取得
	bool GetSharedEdge(
		const NavMeshPolygon& other,
		Math::Vector3& outStart,
		Math::Vector3& outEnd) const;

	// 指定した座標がポリゴン内にあるか
	bool ContainsPoint(const Math::Vector3& point) const;

	// 指定した座標からもっとも近いポリゴン
	Math::Vector3 GetClosestPoint(const Math::Vector3& point) const;

private:

	int m_id = -1;

	std::vector<Math::Vector3> m_vertices;

	Math::Vector3 m_center =	 Math::Vector3::Zero;

	std::vector<int> m_connections;

	// id取得
	void SetID(int id)
	{
		m_id = id;
	}

	// 接続先id取得
	void AddConnection(int polygonID)
	{
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

	// id削除
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