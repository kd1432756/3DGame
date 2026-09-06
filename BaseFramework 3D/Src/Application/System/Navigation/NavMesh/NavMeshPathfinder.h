#pragma once

#include "NavMeshManager.h"

//============================================================
// NavMesh Pathfinder
//============================================================
//
// NavMesh上でA*による経路探索を行う。
//
// A*の探索単位:
//
//     Polygon + Entry Polygon
//
// Polygonだけではなく、どのPolygonから入ってきたかを
// Stateとして保持する。
//
// A*:
//     Polygon Pathを決定
//
// Funnel:
//     Polygon Pathから実際の移動Pointを生成
//
// 座標系:
//     XZ平面をNavMeshの移動平面として扱う。
//     Yは経路コスト・Funnel判定には使用しない。
//
//============================================================

class NavMeshPathfinder
{
public:

	//============================================================
	// Polygon Path
	//============================================================

	std::vector<int> FindPath(
		const NavMeshManager& navMeshManager,
		const Math::Vector3& start,
		const Math::Vector3& goal) const;

	//============================================================
	// Point Path
	//============================================================

	std::vector<Math::Vector3> FindPathPoints(
		const NavMeshManager& navMeshManager,
		const Math::Vector3& start,
		const Math::Vector3& goal) const;

private:

	//============================================================
	// Search State
	//============================================================

	struct State
	{
		int polygonID = -1;

		int entryPolygonID = -1;

		bool operator==(
			const State& other) const
		{
			return
				polygonID ==
				other.polygonID &&
				entryPolygonID ==
				other.entryPolygonID;
		}
	};

	//============================================================
	// State Hash
	//============================================================

	struct StateHash
	{
		size_t operator()(
			const State& state) const
		{
			const size_t h1 =
				std::hash<int>{}(
					state.polygonID);

			const size_t h2 =
				std::hash<int>{}(
					state.entryPolygonID);

			return
				h1 ^
				(
					h2 +
					static_cast<size_t>(0x9e3779b9) +
					(h1 << 6) +
					(h1 >> 2)
					);
		}
	};

	//============================================================
	// A* Node
	//============================================================

	struct Node
	{
		State state;

		//--------------------------------------------------------
		// このPolygonへ入ったPoint
		//
		// Start:
		//     start
		//
		// その他:
		//     前のPortal上の最近点
		//--------------------------------------------------------

		Math::Vector3 entryPoint;

		float gCost = 0.0f;

		float hCost = 0.0f;

		State parentState;

		bool hasParent = false;

		float GetFCost() const
		{
			return gCost + hCost;
		}
	};

	//============================================================
	// Portal
	//============================================================

	struct Portal
	{
		Math::Vector3 left;

		Math::Vector3 right;
	};

	//============================================================
	// Portal
	//============================================================

	bool CreatePortal(
		const NavMeshPolygon& from,
		const NavMeshPolygon& to,
		Portal& outPortal) const;

	//============================================================
	// Closest Point
	//============================================================

	Math::Vector3 GetClosestPointOnPortal(
		const Math::Vector3& point,
		const Portal& portal) const;

	//============================================================
	// Heuristic
	//============================================================

	float CalculateHeuristic(
		const Math::Vector3& currentPoint,
		const Math::Vector3& goalPoint) const;

	//============================================================
	// XZ Distance
	//============================================================

	float CalculateDistanceXZ(
		const Math::Vector3& a,
		const Math::Vector3& b) const;

	//============================================================
	// Path Reconstruction
	//============================================================

	std::vector<int> ReconstructPath(
		const std::unordered_map<
		State,
		Node,
		StateHash>& nodes,
		const State& goalState) const;
};
