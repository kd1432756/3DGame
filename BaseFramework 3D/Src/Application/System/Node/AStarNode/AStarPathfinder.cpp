#include "AStarPathfinder.h"
#include "AStarNodeManager.h"

std::vector<int> AStarPathfinder::FindPath(
	const AStarNodeManager& manager,
	int startID,
	int goalID) const
{
	//------------------------------------------------------------
	// Start / Goal
	//------------------------------------------------------------

	auto startNode =
		manager.FindNode(startID);

	auto goalNode =
		manager.FindNode(goalID);

	if (!startNode || !goalNode)
	{
		return {};
	}

	//------------------------------------------------------------
	// StartとGoalが同じ
	//------------------------------------------------------------

	if (startID == goalID)
	{
		return { startID };
	}

	//------------------------------------------------------------
	// Open / Closed
	//------------------------------------------------------------

	std::vector<SearchNode> openList;
	std::vector<SearchNode> closedList;

	//------------------------------------------------------------
	// Start Node
	//------------------------------------------------------------

	SearchNode startSearch;

	startSearch.nodeID = startID;
	startSearch.gCost = 0.0f;
	startSearch.hCost =
		CalculateHCost(
			*startNode,
			*goalNode);

	startSearch.fCost =
		startSearch.gCost +
		startSearch.hCost;

	startSearch.parentID = -1;

	openList.push_back(startSearch);

	//------------------------------------------------------------
	// A*
	//------------------------------------------------------------

	while (!openList.empty())
	{
		//--------------------------------------------------------
		// F Costが一番小さいNodeを取得
		//--------------------------------------------------------

		int currentIndex =
			FindLowestFCostNode(openList);

		if (currentIndex < 0)
		{
			break;
		}

		SearchNode current =
			openList[currentIndex];

		//--------------------------------------------------------
		// Openから削除
		//--------------------------------------------------------

		openList.erase(
			openList.begin() + currentIndex);

		//--------------------------------------------------------
		// Goal到達
		//--------------------------------------------------------

		if (current.nodeID == goalID)
		{
			closedList.push_back(current);

			return BuildPath(
				closedList,
				startID,
				goalID);
		}

		//--------------------------------------------------------
		// Closedへ
		//--------------------------------------------------------

		closedList.push_back(current);

		//--------------------------------------------------------
		// Current Node
		//--------------------------------------------------------

		auto currentNode =
			manager.FindNode(
				current.nodeID);

		if (!currentNode)
		{
			continue;
		}

		//--------------------------------------------------------
		// Connection
		//--------------------------------------------------------

		for (int nextID :
		currentNode->GetConnections())
		{
			//----------------------------------------------------
			// 接続先Node
			//----------------------------------------------------

			auto nextNode =
				manager.FindNode(nextID);

			if (!nextNode)
			{
				continue;
			}

			//----------------------------------------------------
			// Closedなら無視
			//----------------------------------------------------

			if (FindSearchNode(
				closedList,
				nextID))
			{
				continue;
			}

			//----------------------------------------------------
			// G Cost
			//----------------------------------------------------

			float newGCost =
				current.gCost +
				CalculateGCost(
					*currentNode,
					*nextNode);

			//----------------------------------------------------
			// Openに存在するか
			//----------------------------------------------------

			SearchNode* openNode =
				FindSearchNode(
					openList,
					nextID);

			//----------------------------------------------------
			// Openに存在しない
			//----------------------------------------------------

			if (!openNode)
			{
				SearchNode newNode;

				newNode.nodeID =
					nextID;

				newNode.gCost =
					newGCost;

				newNode.hCost =
					CalculateHCost(
						*nextNode,
						*goalNode);

				newNode.fCost =
					newNode.gCost +
					newNode.hCost;

				newNode.parentID =
					current.nodeID;

				openList.push_back(
					newNode);

				continue;
			}

			//----------------------------------------------------
			// 既存より良い経路なら更新
			//----------------------------------------------------

			if (newGCost < openNode->gCost)
			{
				openNode->gCost =
					newGCost;

				openNode->fCost =
					openNode->gCost +
					openNode->hCost;

				openNode->parentID =
					current.nodeID;
			}
		}
	}

	//------------------------------------------------------------
	// Pathが見つからなかった
	//------------------------------------------------------------

	return {};
}

float AStarPathfinder::CalculateGCost(
	const AStarNode& current,
	const AStarNode& next) const
{
	return (
		next.GetPos() -
		current.GetPos()
		).Length();
}

float AStarPathfinder::CalculateHCost(
	const AStarNode& current,
	const AStarNode& goal) const
{
	return (
		goal.GetPos() -
		current.GetPos()
		).Length();
}

int AStarPathfinder::FindLowestFCostNode(
	const std::vector<SearchNode>& openList) const
{
	if (openList.empty())
	{
		return -1;
	}

	int bestIndex = 0;

	for (int i = 1;
		i < static_cast<int>(openList.size());
		++i)
	{
		const SearchNode& current =
			openList[i];

		const SearchNode& best =
			openList[bestIndex];

		//--------------------------------------------------------
		// F Cost
		//--------------------------------------------------------

		if (current.fCost < best.fCost)
		{
			bestIndex = i;
			continue;
		}

		//--------------------------------------------------------
		// Fが同じならHが小さい方
		//--------------------------------------------------------

		if (current.fCost == best.fCost &&
			current.hCost < best.hCost)
		{
			bestIndex = i;
		}
	}

	return bestIndex;
}

AStarPathfinder::SearchNode*
AStarPathfinder::FindSearchNode(
	std::vector<SearchNode>& nodes,
	int nodeID) const
{
	for (auto& node : nodes)
	{
		if (node.nodeID == nodeID)
		{
			return &node;
		}
	}

	return nullptr;
}

const AStarPathfinder::SearchNode*
AStarPathfinder::FindSearchNode(
	const std::vector<SearchNode>& nodes,
	int nodeID) const
{
	for (const auto& node : nodes)
	{
		if (node.nodeID == nodeID)
		{
			return &node;
		}
	}

	return nullptr;
}

std::vector<int> AStarPathfinder::BuildPath(
	const std::vector<SearchNode>& searchNodes,
	int startID,
	int goalID) const
{
	std::vector<int> path;

	int currentID = goalID;

	//------------------------------------------------------------
	// Goal → Parent → Parent...
	//------------------------------------------------------------

	while (currentID != -1)
	{
		path.push_back(currentID);

		//--------------------------------------------------------
		// Start到達
		//--------------------------------------------------------

		if (currentID == startID)
		{
			break;
		}

		//--------------------------------------------------------
		// SearchNode
		//--------------------------------------------------------

		const SearchNode* node =
			FindSearchNode(
				searchNodes,
				currentID);

		if (!node)
		{
			// Parent情報が壊れている
			return {};
		}

		currentID =
			node->parentID;
	}

	//------------------------------------------------------------
	// Startまで到達していなければ失敗
	//------------------------------------------------------------

	if (path.empty() ||
		path.back() != startID)
	{
		return {};
	}

	//------------------------------------------------------------
	// Goal → Start
	// ↓
	// Start → Goal
	//------------------------------------------------------------

	std::reverse(
		path.begin(),
		path.end());

	return path;
}
