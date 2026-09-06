#include "NavMeshPathfinder.h"

//============================================================
// FindPath
//============================================================
//
// A*によってPolygon IDの経路を求める。
//
// State:
//     Polygon + Entry Polygon
//
// Node:
//     State
//     Entry Point
//     G Cost
//     H Cost
//
//============================================================

std::vector<int>
NavMeshPathfinder::FindPath(
	const NavMeshManager& navMeshManager,
	const Math::Vector3& start,
	const Math::Vector3& goal) const
{
	//--------------------------------------------------------
	// Start Polygon
	//--------------------------------------------------------

	auto startPolygon =
		navMeshManager.FindPolygonFromPoint(start);

	if (!startPolygon)
	{
		return {};
	}

	//--------------------------------------------------------
	// Goal Polygon
	//--------------------------------------------------------

	auto goalPolygon =
		navMeshManager.FindPolygonFromPoint(goal);

	if (!goalPolygon)
	{
		return {};
	}

	const int startPolygonID =
		startPolygon->GetID();

	const int goalPolygonID =
		goalPolygon->GetID();

	//--------------------------------------------------------
	// Start == Goal
	//--------------------------------------------------------

	if (startPolygonID == goalPolygonID)
	{
		return
		{
			startPolygonID
		};
	}

	//--------------------------------------------------------
	// Nodes
	//--------------------------------------------------------

	std::unordered_map<
		State,
		Node,
		StateHash> nodes;

	//--------------------------------------------------------
	// Open
	//--------------------------------------------------------

	std::vector<State> openList;

	//--------------------------------------------------------
	// Closed
	//--------------------------------------------------------

	std::unordered_set<
		State,
		StateHash> closedSet;

	//--------------------------------------------------------
	// Start State
	//--------------------------------------------------------

	State startState;

	startState.polygonID =
		startPolygonID;

	startState.entryPolygonID =
		-1;

	Node startNode;

	startNode.state =
		startState;

	startNode.entryPoint =
		start;

	startNode.gCost =
		0.0f;

	startNode.hCost =
		CalculateHeuristic(
			start,
			goal);

	startNode.hasParent =
		false;

	nodes.emplace(
		startState,
		startNode);

	openList.push_back(
		startState);

	//--------------------------------------------------------
	// A*
	//--------------------------------------------------------

	int iteration = 0;

	while (!openList.empty())
	{
		++iteration;

		//----------------------------------------------------
		// F Cost最小
		//----------------------------------------------------

		size_t bestIndex = 0;

		for (size_t i = 1;
			i < openList.size();
			++i)
		{
			const Node& candidate =
				nodes.at(openList[i]);

			const Node& best =
				nodes.at(openList[bestIndex]);

			if (candidate.GetFCost() <
				best.GetFCost())
			{
				bestIndex = i;
			}
			else if (
				candidate.GetFCost() ==
				best.GetFCost())
			{
				if (candidate.hCost <
					best.hCost)
				{
					bestIndex = i;
				}
			}
		}

		//----------------------------------------------------
		// Current
		//----------------------------------------------------

		const State currentState =
			openList[bestIndex];

		const Node currentNode =
			nodes.at(currentState);

		//----------------------------------------------------
		// Open Remove
		//----------------------------------------------------

		openList.erase(
			openList.begin() +
			bestIndex);

		closedSet.insert(
			currentState);

		//----------------------------------------------------
		// Goal
		//----------------------------------------------------

		if (currentState.polygonID ==
			goalPolygonID)
		{
			const auto path =
				ReconstructPath(
					nodes,
					currentState);

			return path;
		}

		//----------------------------------------------------
		// Current Polygon
		//----------------------------------------------------

		auto currentPolygon =
			navMeshManager.FindPolygon(
				currentState.polygonID);

		if (!currentPolygon)
		{
			continue;
		}

		//----------------------------------------------------
		// Neighbor
		//----------------------------------------------------

		for (int neighborID :
		currentPolygon->GetConnections())
		{
			//------------------------------------------------
			// Neighbor Polygon
			//------------------------------------------------

			auto neighborPolygon =
				navMeshManager.FindPolygon(
					neighborID);

			if (!neighborPolygon)
			{
				continue;
			}

			//------------------------------------------------
			// Portal
			//------------------------------------------------

			Portal portal;

			if (!CreatePortal(
				*currentPolygon,
				*neighborPolygon,
				portal))
			{
				continue;
			}

			//------------------------------------------------
			// Closest Point
			//------------------------------------------------

			const Math::Vector3 portalPoint =
				GetClosestPointOnPortal(
					currentNode.entryPoint,
					portal);

			//------------------------------------------------
			// Move Cost
			//------------------------------------------------

			const float moveCost =
				CalculateDistanceXZ(
					currentNode.entryPoint,
					portalPoint);

			const float newGCost =
				currentNode.gCost +
				moveCost;

			//------------------------------------------------
			// Neighbor State
			//------------------------------------------------

			State neighborState;

			neighborState.polygonID =
				neighborID;

			neighborState.entryPolygonID =
				currentState.polygonID;

			//------------------------------------------------
			// Closed
			//------------------------------------------------

			if (closedSet.find(
				neighborState)
				!=
				closedSet.end())
			{
				continue;
			}

			//------------------------------------------------
			// Existing Node
			//------------------------------------------------

			auto it =
				nodes.find(
					neighborState);

			if (it == nodes.end())
			{
				Node neighborNode;

				neighborNode.state =
					neighborState;

				neighborNode.entryPoint =
					portalPoint;

				neighborNode.gCost =
					newGCost;

				neighborNode.hCost =
					CalculateHeuristic(
						portalPoint,
						goal);

				neighborNode.parentState =
					currentState;

				neighborNode.hasParent =
					true;

				nodes.emplace(
					neighborState,
					neighborNode);

				openList.push_back(
					neighborState);
			}
			else
			{
				Node& neighborNode =
					it->second;

				if (newGCost <
					neighborNode.gCost)
				{
					neighborNode.entryPoint =
						portalPoint;

					neighborNode.gCost =
						newGCost;

					neighborNode.hCost =
						CalculateHeuristic(
							portalPoint,
							goal);

					neighborNode.parentState =
						currentState;
				}
			}
		}
	}

	return {};
}

//============================================================
// FindPathPoints
//============================================================
//
// Polygon Path
//     ↓
// Portal List
//     ↓
// Portal Normalize
//     ↓
// Funnel
//     ↓
// Point Path
//
//============================================================

std::vector<Math::Vector3>
NavMeshPathfinder::FindPathPoints(
	const NavMeshManager& navMeshManager,
	const Math::Vector3& start,
	const Math::Vector3& goal) const
{
	//--------------------------------------------------------
	// Polygon Path
	//--------------------------------------------------------

	const std::vector<int> polygonPath =
		FindPath(
			navMeshManager,
			start,
			goal);

	if (polygonPath.empty())
	{
		return {};
	}

	//--------------------------------------------------------
	// Same Polygon
	//--------------------------------------------------------

	if (polygonPath.size() == 1)
	{
		return
		{
			start,
			goal
		};
	}

	//--------------------------------------------------------
	// Portal List
	//
	// [0]         Start
	// [1..N-1]    Shared Edge
	// [N]         Goal
	//
	//--------------------------------------------------------

	std::vector<Portal> portals;

	portals.reserve(
		polygonPath.size() + 1);

	//--------------------------------------------------------
	// Start Portal
	//--------------------------------------------------------

	{
		Portal portal;

		portal.left =
			start;

		portal.right =
			start;

		portals.push_back(
			portal);
	}

	//--------------------------------------------------------
	// Shared Portals
	//--------------------------------------------------------

	for (size_t i = 0;
		i + 1 < polygonPath.size();
		++i)
	{
		auto fromPolygon =
			navMeshManager.FindPolygon(
				polygonPath[i]);

		auto toPolygon =
			navMeshManager.FindPolygon(
				polygonPath[i + 1]);

		if (!fromPolygon ||
			!toPolygon)
		{
			return {};
		}

		Portal portal;

		if (!CreatePortal(
			*fromPolygon,
			*toPolygon,
			portal))
		{
			return {};
		}

		//----------------------------------------------------
		// Travel Direction
		//
		// Polygon A → Polygon B
		//
		//----------------------------------------------------

		Math::Vector3 travelDirection =
			toPolygon->GetCenter() -
			fromPolygon->GetCenter();

		//----------------------------------------------------
		// XZ Length
		//----------------------------------------------------

		const float travelLengthSq =
			travelDirection.x *
			travelDirection.x +
			travelDirection.z *
			travelDirection.z;

		if (travelLengthSq <= 0.000001f)
		{
			travelDirection =
				goal -
				start;
		}

		//----------------------------------------------------
		// Portal Edge
		//----------------------------------------------------

		const Math::Vector3 portalEdge =
			portal.right -
			portal.left;

		//----------------------------------------------------
		// XZ Cross
		//
		// travel × edge
		//
		// cross > 0
		//     edge.right 側が Left 側になる
		//
		// その場合、現在の left/right を swap。
		//
		//----------------------------------------------------

		const float cross =
			travelDirection.x *
			portalEdge.z
			-
			travelDirection.z *
			portalEdge.x;

		//----------------------------------------------------
		// Normalize Left / Right
		//----------------------------------------------------

		if (cross > 0.0f)
		{
			std::swap(
				portal.left,
				portal.right);
		}

		portals.push_back(
			portal);
	}

	//--------------------------------------------------------
	// Goal Portal
	//--------------------------------------------------------

	{
		Portal portal;

		portal.left =
			goal;

		portal.right =
			goal;

		portals.push_back(
			portal);
	}

	//--------------------------------------------------------
	// Utility
	//--------------------------------------------------------

	auto TriArea2 =
		[](
			const Math::Vector3& a,
			const Math::Vector3& b,
			const Math::Vector3& c)
		{
			const float abx =
				b.x - a.x;

			const float abz =
				b.z - a.z;

			const float acx =
				c.x - a.x;

			const float acz =
				c.z - a.z;

			return
				acx * abz -
				abx * acz;
		};


	auto IsSamePoint =
		[](
			const Math::Vector3& a,
			const Math::Vector3& b)
		{
			constexpr float EPSILON =
				0.001f;

			const float dx =
				a.x - b.x;

			const float dy =
				a.y - b.y;

			const float dz =
				a.z - b.z;

			return
				dx * dx +
				dy * dy +
				dz * dz
				<=
				EPSILON * EPSILON;
		};

	//--------------------------------------------------------
	// Funnel Initial State
	//--------------------------------------------------------

	Math::Vector3 apex =
		portals[0].left;

	Math::Vector3 funnelLeft =
		portals[0].left;

	Math::Vector3 funnelRight =
		portals[0].right;

	size_t apexIndex =
		0;

	size_t leftIndex =
		0;

	size_t rightIndex =
		0;

	std::vector<Math::Vector3> path;

	path.push_back(
		start);

	//--------------------------------------------------------
	// Funnel
	//--------------------------------------------------------

	size_t i = 1;

	while (i < portals.size())
	{
		const Math::Vector3 newLeft =
			portals[i].left;

		const Math::Vector3 newRight =
			portals[i].right;

		//----------------------------------------------------
		//====================================================
		// Update Right
		//====================================================
		//----------------------------------------------------

		const float rightArea =
			TriArea2(
				apex,
				funnelRight,
				newRight);

		if (rightArea <= 0.0f)
		{
			const float rightCross =
				TriArea2(
					apex,
					funnelLeft,
					newRight);

			//------------------------------------------------
			// Right can tighten.
			//------------------------------------------------

			if (IsSamePoint(
				apex,
				funnelRight)
				||
				rightCross > 0.0f)
			{
				funnelRight =
					newRight;

				rightIndex =
					i;
			}
			else
			{
				//------------------------------------------------
				// Right crossed Left.
				//
				// Left becomes corner.
				//------------------------------------------------

				if (!IsSamePoint(
					path.back(),
					funnelLeft))
				{
					path.push_back(
						funnelLeft);
				}

				apex =
					funnelLeft;

				apexIndex =
					leftIndex;

				funnelLeft =
					apex;

				funnelRight =
					apex;

				leftIndex =
					apexIndex;

				rightIndex =
					apexIndex;

				//------------------------------------------------
				// Restart from Apex Portal
				//------------------------------------------------

				i =
					apexIndex;

				++i;

				continue;
			}
		}

		//----------------------------------------------------
		//====================================================
		// Update Left
		//====================================================
		//----------------------------------------------------

		const float leftArea =
			TriArea2(
				apex,
				funnelLeft,
				newLeft);

		if (leftArea >= 0.0f)
		{
			const float leftCross =
				TriArea2(
					apex,
					funnelRight,
					newLeft);

			//------------------------------------------------
			// Left can tighten.
			//------------------------------------------------

			if (IsSamePoint(
				apex,
				funnelLeft)
				||
				leftCross < 0.0f)
			{
				funnelLeft =
					newLeft;

				leftIndex =
					i;
			}
			else
			{
				//------------------------------------------------
				// Left crossed Right.
				//
				// Right becomes corner.
				//------------------------------------------------

				if (!IsSamePoint(
					path.back(),
					funnelRight))
				{
					path.push_back(
						funnelRight);
				}

				apex =
					funnelRight;

				apexIndex =
					rightIndex;

				funnelLeft =
					apex;

				funnelRight =
					apex;

				leftIndex =
					apexIndex;

				rightIndex =
					apexIndex;

				//------------------------------------------------
				// Restart from Apex Portal
				//------------------------------------------------

				i =
					apexIndex;

				++i;

				continue;
			}
		}

		//----------------------------------------------------
		// Next Portal
		//----------------------------------------------------

		++i;
	}

	//--------------------------------------------------------
	// Goal
	//--------------------------------------------------------

	if (path.empty() ||
		!IsSamePoint(
			path.back(),
			goal))
	{
		path.push_back(
			goal);
	}

	return path;
}

//============================================================
// CreatePortal
//============================================================
//
// from → to
//
// Shared Edgeをそのまま取得する。
// Left / Right の向きはここでは決めない。
//
//============================================================

bool
NavMeshPathfinder::CreatePortal(
	const NavMeshPolygon& from,
	const NavMeshPolygon& to,
	Portal& outPortal) const
{
	Math::Vector3 edgeStart;
	Math::Vector3 edgeEnd;

	//--------------------------------------------------------
	// Shared Edge
	//--------------------------------------------------------

	if (!from.GetSharedEdge(
		to,
		edgeStart,
		edgeEnd))
	{
		return false;
	}

	//--------------------------------------------------------
	// Do not determine Left / Right here.
	//--------------------------------------------------------

	outPortal.left =
		edgeStart;

	outPortal.right =
		edgeEnd;

	return true;
}

//============================================================
// GetClosestPointOnPortal
//============================================================
//
// XZ平面上でPointからPortalへの最近点を求める。
//
// YはPortalの線形補間によって保持する。
//
//============================================================

Math::Vector3
NavMeshPathfinder::GetClosestPointOnPortal(
	const Math::Vector3& point,
	const Portal& portal) const
{
	const Math::Vector3 edge =
		portal.right -
		portal.left;

	//--------------------------------------------------------
	// XZ Length
	//--------------------------------------------------------

	const float edgeLengthSq =
		edge.x * edge.x +
		edge.z * edge.z;

	//--------------------------------------------------------
	// Degenerate Portal
	//--------------------------------------------------------

	if (edgeLengthSq <=
		0.000001f)
	{
		return portal.left;
	}

	//--------------------------------------------------------
	// Point → Portal Start
	//--------------------------------------------------------

	const Math::Vector3 toPoint =
		point -
		portal.left;

	//--------------------------------------------------------
	// XZ Dot
	//--------------------------------------------------------

	const float dot =
		toPoint.x * edge.x +
		toPoint.z * edge.z;

	//--------------------------------------------------------
	// Projection
	//--------------------------------------------------------

	float t =
		dot /
		edgeLengthSq;

	//--------------------------------------------------------
	// Clamp
	//--------------------------------------------------------

	t =
		std::clamp(
			t,
			0.0f,
			1.0f);

	//--------------------------------------------------------
	// Result
	//--------------------------------------------------------

	return
		portal.left +
		edge * t;
}

//============================================================
// CalculateDistanceXZ
//============================================================

float
NavMeshPathfinder::CalculateDistanceXZ(
	const Math::Vector3& a,
	const Math::Vector3& b) const
{
	const float dx =
		b.x -
		a.x;

	const float dz =
		b.z -
		a.z;

	return std::sqrt(
		dx * dx +
		dz * dz);
}

//============================================================
// CalculateHeuristic
//============================================================

float
NavMeshPathfinder::CalculateHeuristic(
	const Math::Vector3& currentPoint,
	const Math::Vector3& goalPoint) const
{
	return CalculateDistanceXZ(
		currentPoint,
		goalPoint);
}

//============================================================
// ReconstructPath
//============================================================

std::vector<int>
NavMeshPathfinder::ReconstructPath(
	const std::unordered_map<
	State,
	Node,
	StateHash>& nodes,
	const State& goalState) const
{
	std::vector<int> path;

	State currentState =
		goalState;

	//--------------------------------------------------------
	// Goal → Start
	//--------------------------------------------------------

	while (true)
	{
		auto it =
			nodes.find(
				currentState);

		if (it == nodes.end())
		{
			return {};
		}

		const Node& node =
			it->second;

		path.push_back(
			node.state.polygonID);

		//----------------------------------------------------
		// Start
		//----------------------------------------------------

		if (!node.hasParent)
		{
			break;
		}

		currentState =
			node.parentState;
	}

	//--------------------------------------------------------
	// Start → Goal
	//--------------------------------------------------------

	std::reverse(
		path.begin(),
		path.end());

	return path;
}
