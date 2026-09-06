#pragma once

class AStarNode;
class AStarNodeManager;

class AStarPathfinder
{
public:

	//============================================================
	// Path
	//============================================================

	// StartからGoalまでの経路を探索
	//
	// 戻り値:
	//   成功 → StartからGoalまでのNode ID
	//   失敗 → 空
	//
	std::vector<int> FindPath(
		const AStarNodeManager& manager,
		int startID,
		int goalID) const;

private:

	//============================================================
	// Search Node
	//============================================================

	struct SearchNode
	{
		int nodeID = -1;

		float gCost = 0.0f;
		float hCost = 0.0f;
		float fCost = 0.0f;

		int parentID = -1;
	};

	//============================================================
	// Cost
	//============================================================

	float CalculateGCost(
		const AStarNode& current,
		const AStarNode& next) const;

	float CalculateHCost(
		const AStarNode& current,
		const AStarNode& goal) const;

	//============================================================
	// Open List
	//============================================================

	int FindLowestFCostNode(
		const std::vector<SearchNode>& openList) const;

	//============================================================
	// Search Data
	//============================================================

	SearchNode* FindSearchNode(
		std::vector<SearchNode>& nodes,
		int nodeID) const;

	const SearchNode* FindSearchNode(
		const std::vector<SearchNode>& nodes,
		int nodeID) const;

	//============================================================
	// Path
	//============================================================

	std::vector<int> BuildPath(
		const std::vector<SearchNode>& searchNodes,
		int startID,
		int goalID) const;
};
