#pragma once

class RoadNode
{
public:

	Math::Vector3 GetPos() const
	{
		return m_pos;
	}

	void SetPos(const Math::Vector3& pos)
	{
		m_pos = pos;
	}

	void AddConnection(
		const std::shared_ptr<RoadNode>& node)
	{
		m_connections.push_back(node);
	}

	const std::vector<std::weak_ptr<RoadNode>>&
		GetConnections() const
	{
		return m_connections;
	}

private:

	Math::Vector3 m_pos;

	std::vector<std::weak_ptr<RoadNode>>
		m_connections;
};