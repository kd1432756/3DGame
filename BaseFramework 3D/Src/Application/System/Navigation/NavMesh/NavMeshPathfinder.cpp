#include "NavMeshPathfinder.h"

std::vector<int> NavMeshPathfinder::FindPath(
	const NavMeshManager& navMeshManager,
	const Math::Vector3& start,
	const Math::Vector3& goal) const
{
	// スタート地点のポリゴンを取得
	auto startPolygon = navMeshManager.FindPolygonFromPoint(start);

	if (!startPolygon)
	{
		return {};
	}
	
	// ゴール地点のポリゴンを取得
	auto goalPolygon = navMeshManager.FindPolygonFromPoint(goal);

	if (!goalPolygon)
	{
		return {};
	}

	const int startPolygonID = startPolygon->GetID();

	const int goalPolygonID = goalPolygon->GetID();

	// 同じポリゴン
	if (startPolygonID == goalPolygonID)
	{
		return
		{
			startPolygonID
		};
	}

	// 探索したノード情報を確保
	std::unordered_map<State, Node, StateHash> nodes;

	// 探索候補のState
	std::vector<State> openList;

	// 探索済のState
	std::unordered_set<State, StateHash> closedSet;

	// 初期設定
	State startState;

	startState.polygonID = startPolygonID;

	startState.entryPolygonID = -1;

	Node startNode;

	startNode.state = startState;

	startNode.entryPoint = start;

	startNode.gCost = 0.0f;

	startNode.hCost = CalculateHeuristic(start, goal);

	startNode.hasParent = false;

	nodes.emplace(startState, startNode);

	openList.push_back(startState);

	// AStar
	int iteration = 0;

	while (!openList.empty())
	{
		++iteration;

		// 最小Fコスト
		size_t bestIndex = 0;

		for (size_t i = 1; i < openList.size(); ++i)
		{
			const Node& candidate = nodes.at(openList[i]);

			const Node& best = nodes.at(openList[bestIndex]);

			if (candidate.GetFCost() < best.GetFCost())
			{
				bestIndex = i;
			}
			else if (candidate.GetFCost() == best.GetFCost())
			{
				if (candidate.hCost < best.hCost)
				{
					bestIndex = i;
				}
			}
		}

		// 対象として取得
		const State currentState = openList[bestIndex];

		const Node currentNode = nodes.at(currentState);

		// 探索済みとして削除
		openList.erase(openList.begin() + bestIndex);

		// 探索済みとして登録
		closedSet.insert(currentState);

		// ゴールに到達するか
		if (currentState.polygonID == goalPolygonID)
		{
			const auto path = ReconstructPath(nodes, currentState);

			return path;
		}

		auto currentPolygon = navMeshManager.FindPolygon(currentState.polygonID);

		if (!currentPolygon)
		{
			continue;
		}

		for (int neighborID : currentPolygon->GetConnections())
		{
			// 隣接ポリゴンを調べる
			auto neighborPolygon = navMeshManager.FindPolygon(neighborID);

			if (!neighborPolygon)
			{
				continue;
			}

			// ポータル作成
			Portal portal;

			if (!CreatePortal(*currentPolygon, *neighborPolygon, portal))
			{
				continue;
			}

			// ポータル上の最近点を取得
			const Math::Vector3 portalPoint = GetClosestPointOnPortal(currentNode.entryPoint, portal);

			const float moveCost = CalculateDistanceXZ(currentNode.entryPoint, portalPoint);

			const float newGCost = currentNode.gCost + moveCost;
					
			// 次の探索State
			State neighborState;

			neighborState.polygonID = neighborID;

			neighborState.entryPolygonID = currentState.polygonID;
				
			// 探索済みStateは無視
			if (closedSet.find(neighborState) != closedSet.end())
			{
				continue;
			}
			
			// ノードが存在するか
			auto it = nodes.find(neighborState);

			if (it == nodes.end())
			{
				// 新しいノードを生成
				Node neighborNode;

				neighborNode.state = neighborState;

				neighborNode.entryPoint = portalPoint;

				neighborNode.gCost = newGCost;

				neighborNode.hCost = CalculateHeuristic(portalPoint, goal);

				neighborNode.parentState = currentState;

				neighborNode.hasParent = true;

				nodes.emplace(neighborState, neighborNode);

				openList.push_back(neighborState);
			}
			else
			{
				Node& neighborNode = it->second;

				// 経路が短いか
				if (newGCost < neighborNode.gCost)
				{
					neighborNode.entryPoint = portalPoint;

					neighborNode.gCost = newGCost;

					neighborNode.hCost = CalculateHeuristic(portalPoint, goal);

					neighborNode.parentState = currentState;
				}
			}
		}
	}

	return {};
}

std::vector<Math::Vector3> NavMeshPathfinder::FindPathPoints(
	const NavMeshManager& navMeshManager,
	const Math::Vector3& start,
	const Math::Vector3& goal) const
{

	// AStarの経路探索
	const std::vector<int> polygonPath = FindPath(navMeshManager, start, goal);

	if (polygonPath.empty())
	{
		return {};
	}

	// 同じポリゴンの場合
	if (polygonPath.size() == 1)
	{
		return		{ start,			goal };
	}

	// ポータルリストを作成
	std::vector<Portal> portals;

	portals.reserve(polygonPath.size() + 1);

	// スタートのポータル
	{
		Portal portal;

		portal.left = start;

		portal.right = start;

		portals.push_back(portal);
	}

	// 隣接するポリゴン同士の共有辺
	for (size_t i = 0; i + 1 < polygonPath.size(); ++i)
	{
		// 隣接するポリゴンを取得
		auto fromPolygon = navMeshManager.FindPolygon(polygonPath[i]);

		auto toPolygon = navMeshManager.FindPolygon(polygonPath[i + 1]);

		if (!fromPolygon || !toPolygon)
		{
			return {};
		}

		Portal portal;

		// 共有辺を取得
		if (!CreatePortal(*fromPolygon, *toPolygon, portal))
		{
			return {};
		}

		// 左右を統一
		Math::Vector3 travelDirection = toPolygon->GetCenter() - fromPolygon->GetCenter();

		const float travelLengthSq = travelDirection.x * travelDirection.x + travelDirection.z * travelDirection.z;

		if (travelLengthSq <= 0.000001f)
		{
			travelDirection = goal - start;
		}

		// 辺の方向を取得
		const Math::Vector3 portalEdge = portal.right - portal.left;
		
		// 外積を計算
		const float cross = travelDirection.x * portalEdge.z - travelDirection.z * portalEdge.x;

		// 外積の符号で左右を統一
		if (cross > 0.0f)
		{
			std::swap(portal.left, portal.right);
		}

		portals.push_back(portal);
	}

	// ゴールのポータル
	{
		Portal portal;

		portal.left = goal;

		portal.right = goal;

		portals.push_back(portal);
	}

	// 三角形の面積の符号から3点が並んでいる向きを計算
	auto TriArea2 = [](const Math::Vector3& a, const Math::Vector3& b, const Math::Vector3& c)
		{
			const float abx = b.x - a.x;

			const float abz = b.z - a.z;

			const float acx = c.x - a.x;

			const float acz = c.z - a.z;

			return acx * abz - abx * acz;};

	// 2点がほぼ同じ位置か
	auto IsSamePoint = [](const Math::Vector3& a, const Math::Vector3& b)
		{
			constexpr float EPSILON = 0.001f;

			const float dx = a.x - b.x;

			const float dy = a.y - b.y;

			const float dz = a.z - b.z;

			return dx * dx + dy * dy + dz * dz <= EPSILON * EPSILON;
		};

	// funnel初期化
	Math::Vector3 apex = portals[0].left;

	Math::Vector3 funnelLeft = portals[0].left;

	Math::Vector3 funnelRight = portals[0].right;

	size_t apexIndex = 0;

	size_t leftIndex = 0;

	size_t rightIndex = 0;

	std::vector<Math::Vector3> path;

	path.push_back(start);

	size_t i = 1;

	// funnelアルゴリズム
	while (i < portals.size())
	{
		const Math::Vector3 newLeft = portals[i].left;

		const Math::Vector3 newRight = portals[i].right;

		// 右側を更新
		const float rightArea = TriArea2(apex, funnelRight, newRight);

		if (rightArea <= 0.0f)
		{
			// 外積を計算
			const float rightCross = TriArea2(apex, funnelLeft, newRight);

			// 右側の境界をポータルの端点に狭める
			if (IsSamePoint(apex, funnelRight) || rightCross > 0.0f)
			{
				funnelRight = newRight;

				rightIndex = i;
			}
			else
			{
				// 左側と交差する
				if (!IsSamePoint(path.back(), funnelLeft))
				{
					path.push_back(funnelLeft);
				}

				apex = funnelLeft;

				apexIndex = leftIndex;

				// funnelリセット
				funnelLeft = apex;

				funnelRight = apex;

				leftIndex = apexIndex;

				rightIndex = apexIndex;
						
				// 新しいスタート地点とする
				i = apexIndex;

				++i;

				continue;
			}
		}

		// 左側を更新
		const float leftArea = TriArea2(apex, funnelLeft, newLeft);

		if (leftArea >= 0.0f)
		{
			const float leftCross = TriArea2(apex, funnelRight, newLeft);

			// 左側の境界をポータルの端点に狭める
			if (IsSamePoint(apex, funnelLeft) || leftCross < 0.0f)
			{
				funnelLeft = newLeft;

				leftIndex = i;
			}
			else
			{
				// 右側と交差する
				if (!IsSamePoint(path.back(), funnelRight))
				{
					path.push_back(funnelRight);
				}

				apex = funnelRight;

				apexIndex = rightIndex;

				// funnelリセット
				funnelLeft = apex;

				funnelRight = apex;

				leftIndex = apexIndex;

				rightIndex = apexIndex;

				// 新しいスタート地点とする
				i = apexIndex;

				++i;

				continue;
			}
		}

		// 次のポータルへ
		++i;
	}

	// 最後にポータルを作成
	if (path.empty() || !IsSamePoint(path.back(), goal))
	{
		path.push_back(goal);
	}

	return path;
}

bool NavMeshPathfinder::CreatePortal(
	const NavMeshPolygon& from,
	const NavMeshPolygon& to,
	Portal& outPortal) const
{
	Math::Vector3 edgeStart;
	Math::Vector3 edgeEnd;

	// 共有辺を取得
	if (!from.GetSharedEdge(to, edgeStart, edgeEnd))
	{
		return false;
	}

	// ポータルに端点を格納
	outPortal.left = edgeStart;

	outPortal.right = edgeEnd;

	return true;
}

Math::Vector3 NavMeshPathfinder::GetClosestPointOnPortal(
	const Math::Vector3& point,
	const Portal& portal) const
{
	// 方向ベクトルを取得
	const Math::Vector3 edge = portal.right - portal.left;

	const float edgeLengthSq = edge.x * edge.x + edge.z * edge.z;

	// 0除算を起こさないため
	if (edgeLengthSq <= 0.000001f)
	{
		return portal.left;
	}

	// ポータルの始点へのベクトルを取得
	const Math::Vector3 toPoint = point - portal.left;

	// 内積を計算
	const float dot = toPoint.x * edge.x + toPoint.z * edge.z;

	// 射影位置を計算
	float t = dot / edgeLengthSq;

	// 線分内に収める
	t = std::clamp(t, 0.0f, 1.0f);

	return portal.left + edge * t;
}

float NavMeshPathfinder::CalculateDistanceXZ(
	const Math::Vector3& a,
	const Math::Vector3& b) const
{
	const float dx = b.x - a.x;

	const float dz = b.z - a.z;

	// 距離を計算
	return std::sqrt(dx * dx + dz * dz);
}

float NavMeshPathfinder::CalculateHeuristic(
	const Math::Vector3& currentPoint,
	const Math::Vector3& goalPoint) const
{
	// ヒューリスティックコスト計算
	return CalculateDistanceXZ(currentPoint, goalPoint);
}

std::vector<int> NavMeshPathfinder::ReconstructPath(
	const std::unordered_map<	State, Node, StateHash>& nodes,
	const State& goalState) const
{
	std::vector<int> path;

	State currentState = goalState;

	// ゴールからスタートに戻る
	while (true)
	{
		// 親ノードをたどる
		auto it = nodes.find(currentState);

		if (it == nodes.end())
		{
			return {};
		}

		const Node& node = it->second;

		path.push_back(node.state.polygonID);

		// スタートまでもどった
		if (!node.hasParent)
		{
			break;
		}

		currentState = node.parentState;
	}

	// 親ノードに移動
	std::reverse(path.begin(), path.end());

	return path;
}
