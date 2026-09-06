#include "RoadNodeManager.h"

#include "RoadNode.h"

void RoadNodeManager::Init()
{
	m_nodes.clear();

	CreateNodes(0.2f);
	CreateConnections();
}

void RoadNodeManager::CreateNodes(float scale)
{
	const Math::Vector3 basePositions[] =
	{
		{ -48.0f, 1.0f, -44.0f },
		{ -16.0f, 1.0f, -44.0f },
		{  48.0f, 1.0f, -44.0f },

		{ -48.0f, 1.0f,   0.0f },
		{ -16.0f, 1.0f,   0.0f },
		{  28.0f, 1.0f,   0.0f },
		{  48.0f, 1.0f,   0.0f },

		{ -48.0f, 1.0f,  44.0f },
		{  28.0f, 1.0f,  44.0f },
		{  48.0f, 1.0f,  44.0f }
	};

	m_nodes.resize(10);

	for (int i = 0; i < 10; ++i)
	{
		m_nodes[i] = std::make_shared<RoadNode>();

		m_nodes[i]->SetPos(
			basePositions[i] * scale
		);
	}
}

void RoadNodeManager::CreateConnections()
{
	ConnectNodes(0, 1);
	ConnectNodes(1, 2);
	ConnectNodes(0, 3);
	ConnectNodes(1, 4);
	ConnectNodes(2, 6);
	ConnectNodes(3, 4);
	ConnectNodes(4, 5);
	ConnectNodes(5, 6);
	ConnectNodes(3, 7);
	ConnectNodes(5, 8);
	ConnectNodes(6, 9);
	ConnectNodes(7, 8);
	ConnectNodes(8, 9);
}

void RoadNodeManager::ConnectNodes(
	int indexA,
	int indexB)
{
	m_nodes[indexA]->AddConnection(m_nodes[indexB]);
	m_nodes[indexB]->AddConnection(m_nodes[indexA]);
}