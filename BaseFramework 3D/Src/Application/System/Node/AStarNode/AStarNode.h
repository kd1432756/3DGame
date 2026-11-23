#pragma once

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
