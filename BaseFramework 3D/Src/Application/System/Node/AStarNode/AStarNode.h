#pragma once

//============================================================
// A* Node
//============================================================
//
// ・GameObjectではない
// ・SceneManagerには登録しない
// ・専用JSONに保存する
//
// Node同士の接続は「Node ID」で管理する
//
//============================================================

class AStarNode
{
public:

	//--------------------------------------------------------
	// ID
	//--------------------------------------------------------

	int GetID() const
	{
		return m_id;
	}

	void SetID(int id)
	{
		m_id = id;
	}

	//--------------------------------------------------------
	// Position
	//--------------------------------------------------------

	const Math::Vector3& GetPos() const
	{
		return m_pos;
	}

	void SetPos(const Math::Vector3& pos)
	{
		m_pos = pos;
	}

	//--------------------------------------------------------
	// Connection
	//--------------------------------------------------------
	//
	// 接続先NodeのIDを保持する
	//
	// 例:
	//
	// Node 0
	//   -> 1
	//   -> 5
	//
	// m_connections = { 1, 5 }
	//
	//--------------------------------------------------------

	const std::vector<int>& GetConnections() const
	{
		return m_connections;
	}

	void SetConnections(const std::vector<int>& connections)
	{
		m_connections = connections;
	}

	void AddConnection(int nodeID)
	{
		// 自分自身には接続しない
		if (nodeID == m_id)
		{
			return;
		}

		// 二重登録防止
		if (HasConnection(nodeID))
		{
			return;
		}

		m_connections.push_back(nodeID);
	}

	void RemoveConnection(int nodeID)
	{
		m_connections.erase(
			std::remove(
				m_connections.begin(),
				m_connections.end(),
				nodeID),
			m_connections.end());
	}

	bool HasConnection(int nodeID) const
	{
		return std::find(
			m_connections.begin(),
			m_connections.end(),
			nodeID)
			!= m_connections.end();
	}

	void ClearConnections()
	{
		m_connections.clear();
	}

	//--------------------------------------------------------
	// Connection数
	//--------------------------------------------------------

	size_t GetConnectionCount() const
	{
		return m_connections.size();
	}

private:

	//--------------------------------------------------------
	// Node ID
	//--------------------------------------------------------

	int m_id = -1;

	//--------------------------------------------------------
	// World Position
	//--------------------------------------------------------

	Math::Vector3 m_pos = Math::Vector3::Zero;

	//--------------------------------------------------------
	// 接続先Node ID
	//--------------------------------------------------------

	std::vector<int> m_connections;
};
