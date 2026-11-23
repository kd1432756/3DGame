#include "AStarNodeManager.h"

std::shared_ptr<AStarNode> AStarNodeManager::CreateNode(const Math::Vector3& pos)
{
	auto node = std::make_shared<AStarNode>();

	node->SetID(m_nextID++);
	node->SetPos(pos);

	m_nodes.push_back(node);

	return node;
}

std::shared_ptr<AStarNode> AStarNodeManager::CreateNode(
	int id, const Math::Vector3& pos)
{
	if (FindNode(id))
	{
		return nullptr;
	}

	auto node = std::make_shared<AStarNode>();

	node->SetID(id);
	node->SetPos(pos);

	m_nodes.push_back(node);

	m_nextID = std::max(m_nextID, id + 1);

	return node;
}

void AStarNodeManager::DeleteNode(int id)
{
	auto node = FindNode(id);

	if (!node)
	{
		return;
	}

	//--------------------------------------------------------
	// 他Nodeから、このNodeへの接続を削除
	//--------------------------------------------------------

	for (auto& other : m_nodes)
	{
		if (!other)
		{
			continue;
		}

		other->RemoveConnection(id);
	}

	//--------------------------------------------------------
	// Node自身を削除
	//--------------------------------------------------------

	m_nodes.erase(
		std::remove(
			m_nodes.begin(),
			m_nodes.end(),
			node),
		m_nodes.end());
}

void AStarNodeManager::Clear()
{
	m_nodes.clear();
	m_nextID = 0;

	m_isLoaded = false;
}

std::shared_ptr<AStarNode> AStarNodeManager::FindNode(int id)
{
	for (auto& node : m_nodes)
	{
		if (!node)
		{
			continue;
		}

		if (node->GetID() == id)
		{
			return node;
		}
	}

	return nullptr;
}

std::shared_ptr<const AStarNode> AStarNodeManager::FindNode(int id) const
{
	for (auto& node : m_nodes)
	{
		if (!node)
		{
			continue;
		}

		if (node->GetID() == id)
		{
			return node;
		}
	}

	return nullptr;
}

void AStarNodeManager::ConnectNodes(
	int fromID, int toID)
{
	auto from = FindNode(fromID);
	auto to = FindNode(toID);

	if (!from || !to)
	{
		return;
	}

	if (fromID == toID)
	{
		return;
	}

	//--------------------------------------------------------
	// 今回は双方向接続
	//--------------------------------------------------------

	from->AddConnection(toID);
	to->AddConnection(fromID);
}

void AStarNodeManager::DisconnectNodes(
	int fromID, int toID)
{
	auto from = FindNode(fromID);
	auto to = FindNode(toID);

	if (!from || !to)
	{
		return;
	}

	//--------------------------------------------------------
	// 双方向接続を解除
	//--------------------------------------------------------

	from->RemoveConnection(toID);
	to->RemoveConnection(fromID);
}
