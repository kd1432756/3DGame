#include "NavMeshPolygonBuilder.h"

namespace
{
	//============================================================
	// VertexId Hash
	//============================================================

	// 頂点座標を効率よく検索するためのハッシュ
	struct VertexIdHash
	{
		size_t operator()(const NavMeshPolygonBuilder::VertexId& v) const noexcept
		{
			// ハッシュ化
			const size_t hx = std::hash<int>{}(v.x);
			const size_t hy = std::hash<int>{}(v.y);

			// 合成
			return hx ^ (hy << 1);
		}
	};

	//============================================================
	// Edge Hash
	//============================================================

	// 辺を効率よく検索するためのハッシュ
	struct EdgeHash
	{
		size_t operator()(const NavMeshPolygonBuilder::Edge& edge) const noexcept
		{
			const VertexIdHash vertexHash;

			// 辺の両端の頂点をハッシュ化
			const size_t ha = vertexHash(edge.a);
			const size_t hb = vertexHash(edge.b);

			// 合成
			return ha ^ (hb << 1);
		}
	};

	//============================================================
	// DirectedEdge
	//============================================================

	// 方向を持つ辺
	struct DirectedEdge
	{
		NavMeshPolygonBuilder::VertexId a;
		NavMeshPolygonBuilder::VertexId b;

		// 始点と終点が一致するか
		bool operator==(const DirectedEdge& other) const
		{
			return a == other.a && b == other.b;
		}
	};

	// 方向付き辺のハッシュ
	struct DirectedEdgeHash
	{
		size_t operator()(const DirectedEdge& edge) const noexcept
		{
			const VertexIdHash vertexHash;

			// 辺の両端の頂点をハッシュ化
			const size_t ha = vertexHash(edge.a);
			const size_t hb = vertexHash(edge.b);

			// 合成
			return ha ^ (hb << 1);
		}
	};

	//============================================================
	// Cross / Signed Area
	//
	// > 0 : CCW
	// < 0 : CW
	//============================================================

	double CalculateSignedArea(const std::vector<NavMeshPolygonBuilder::VertexId>& polygon)
	{
		if (polygon.size() < 3)
		{
			return 0.0;
		}

		double area = 0.0;

		for (size_t i = 0; i < polygon.size(); ++i)
		{
			// すべての辺を調べる
			const auto& a = polygon[i];
			const auto& b = polygon[(i + 1) % polygon.size()];		// 最初の頂点に戻れる

			// 靴紐公式
			area +=
				static_cast<double>(a.x) * static_cast<double>(b.y) -
				static_cast<double>(b.x) * static_cast<double>(a.y);
		}

		return area * 0.5;
	}

	//============================================================
	// Direction
	//============================================================

	int GetDirection(const NavMeshPolygonBuilder::VertexId& from, const NavMeshPolygonBuilder::VertexId& to)
	{
		// 移動量を計算
		const int dx = to.x - from.x;
		const int dy = to.y - from.y;

		// ４方向判定
		if (dx > 0)
		{
			return 0;	// East
		}

		if (dy > 0)
		{
			return 1;	// North
		}

		if (dx < 0)
		{
			return 2;	// West
		}

		return 3;	// South
	}

	//============================================================
	// Turn priority
	//============================================================

	int GetTurnPriority(int incomingDirection, int outgoingDirection)
	{
		// outgoingがincomingに対して何度回転するか
		const int turn = (outgoingDirection - incomingDirection + 4) % 4;

		// CCWの境界を作るため左方向を優先
		switch (turn)
		{
		case 1:
			return 0; // 左
		case 0:
			return 1; // 直進
		case 2:
			return 2; // 右
		default:
			return 3; // U-turn
		}
	}

	//============================================================
	// Find next directed edge
	//============================================================

	bool FindNextEdge(
		const NavMeshPolygonBuilder::VertexId& current,
		int incomingDirection,
		const std::unordered_set<
		DirectedEdge,
		DirectedEdgeHash>& edges,
		DirectedEdge& outEdge)
	{
		bool found = false;

		int bestPriority = std::numeric_limits<int>::max();

		for (const DirectedEdge& edge : edges)
		{
			// 現在の頂点から出ていない
			if (!(edge.a == current))
			{
				continue;
			}

			// 次の辺の方向を取得
			const int outgoingDirection = GetDirection(edge.a, edge.b);

			// 曲がる方向を取得（左方向優先）
			const int priority = GetTurnPriority(incomingDirection, outgoingDirection);

			// 最も優先順位の高い辺を選択
			if (!found || priority < bestPriority)
			{
				found = true;
				bestPriority = priority;
				outEdge = edge;
			}
		}

		return found;
	}

	//============================================================
	// Remove duplicate consecutive vertices
	//============================================================

	void RemoveDuplicateVertices(std::vector<NavMeshPolygonBuilder::VertexId>& polygon)
	{
		if (polygon.empty())
		{
			return;
		}

		// 新しい頂点配列
		std::vector<NavMeshPolygonBuilder::VertexId> result;
		result.reserve(polygon.size());

		for (const auto& vertex : polygon)
		{
			// 現在の頂点と追加した頂点が同じなら、追加した頂点を削除
			if (!result.empty() && result.back() == vertex)
			{
				continue;
			}

			result.push_back(vertex);
		}

		// 先頭と末尾が同じなら末尾を削除
		if (result.size() >= 2 && result.front() == result.back())
		{
			result.pop_back();
		}

		// 置き換える
		polygon = std::move(result);
	}

	//============================================================
	// Remove collinear vertices 
	//============================================================

	void RemoveCollinearVertices(std::vector<NavMeshPolygonBuilder::VertexId>& polygon)
	{
		if (polygon.size() < 3)
		{
			return;
		}

		// 削除したか　
		bool removed = true;

		while (removed && polygon.size() >= 3)
		{
			removed = false;

			const size_t count = polygon.size();

			for (size_t i = 0; i < count; ++i)
			{
				// 1つ前の頂点
				const auto& prev = polygon[(i + count - 1) % count];

				// 現在の頂点
				const auto& current = polygon[i];

				// 1つ後の頂点
				const auto& next = polygon[(i + 1) % count];

				// 頂点の方向を計算
				const int dx1 = current.x - prev.x;
				const int dy1 = current.y - prev.y;

				const int dx2 = next.x - current.x;
				const int dy2 = next.y - current.y;

				// 外積を計算
				const int cross = dx1 * dy2 - dy1 * dx2;

				// 一直線でない
				if (cross != 0)
				{
					continue;
				}

				// 一直線なら現在の頂点を削除
				polygon.erase(polygon.begin() + static_cast<std::ptrdiff_t>(i));

				removed = true;
				break;
			}
		}
	}
}

//============================================================
// Build
//============================================================

std::vector<NavMeshPolygonBuilder::Polygon> NavMeshPolygonBuilder::BuildTiles(
	const std::vector<Tile>& tiles, float tileSize)
{
	std::vector<Polygon> result;

	if (tiles.empty())
	{
		return result;
	}
	if (tileSize <= 0.0f)
	{
		return result;
	}

	//============================================================
	// 1. 全 Tile の境界 Edge を抽出
	//============================================================

	std::unordered_set<DirectedEdge, DirectedEdgeHash> boundaryEdges;

	boundaryEdges.reserve(tiles.size() * 4);

	for (const Tile& tile : tiles)
	{
		// 頂点を取得
		const auto vertices = GetTileVertices(tile, tileSize);

		// 辺を取得
		const DirectedEdge edges[] =
		{
			{ vertices[0], vertices[1] },
			{ vertices[1], vertices[2] },
			{ vertices[2], vertices[3] },
			{ vertices[3], vertices[0] }
		};

		for (const DirectedEdge& edge : edges)
		{
			// 逆向きの辺を作る
			const DirectedEdge reverse{ edge.b, edge.a };

			// 共有辺を探す
			auto reverseIt = boundaryEdges.find(reverse);

			if (reverseIt != boundaryEdges.end())
			{
				// 存在したらその辺を消す
				boundaryEdges.erase(reverseIt);
			}
			else
			{
				// 存在しなかったら追加する
				boundaryEdges.insert(edge);
			}
		}
	}

	if (boundaryEdges.empty())
	{
		return result;
	}

	//============================================================
	// 2. Boundary Edge から Loop を作る
	//============================================================

	std::vector<std::vector<VertexId>> loops;

	// すべての境界辺をLoopにする
	while (!boundaryEdges.empty())
	{
		// 最初の辺を取得
		auto firstIt = boundaryEdges.begin();

		const DirectedEdge firstEdge = *firstIt;

		boundaryEdges.erase(firstIt);

		// loopの頂点群
		std::vector<VertexId> loop;

		loop.reserve(16);

		loop.push_back(firstEdge.a);

		// 現在頂点
		VertexId current = firstEdge.b;

		// 開始頂点
		const VertexId start = firstEdge.a;

		// 辺の進行方向を取得
		int incomingDirection = GetDirection(firstEdge.a, firstEdge.b);

		size_t safetyCounter = 0;

		const size_t safetyLimit = tiles.size() * 4 + 16;

		// Loopが完成するまで
		while (safetyCounter++ < safetyLimit)
		{
			loop.push_back(current);

			// 開始点にもどったか
			if (current == start)
			{
				break;
			}

			DirectedEdge nextEdge;

			// 左折優先で次の辺を選択
			if (!FindNextEdge(
				current,
				incomingDirection,
				boundaryEdges,
				nextEdge))
			{
				break;
			}

			boundaryEdges.erase(nextEdge);

			// 進行方法更新
			incomingDirection = GetDirection(nextEdge.a, nextEdge.b);

			// 現在頂点更新
			current = nextEdge.b;
		}

		// 閉じているか
		if (loop.size() < 4 || loop.back() != start)
		{
			continue;
		}

		// 最後のstart（開始点と重なる）を削除
		loop.pop_back();

		// 重複した頂点を削除
		RemoveDuplicateVertices(loop);

		// 直線上にある中間頂点を削除
		RemoveCollinearVertices(loop);

		if (loop.size() < 3)
		{
			continue;
		}

		const double area = CalculateSignedArea(loop);

		if (std::abs(area) < 0.000001)
		{
			continue;
		}

		// loopsに移して追加
		loops.push_back(std::move(loop));
	}

	if (loops.empty())
	{
		return result;
	}

	//============================================================
	// 3. Outer / Hole に分類
	//============================================================

	std::vector<std::vector<VertexId>>	outerLoops;

	std::vector<std::vector<VertexId>>	holeLoops;

	for (auto& loop : loops)
	{
		// 頂点の格納順から符号付で面積を求める
		const double area = CalculateSignedArea(loop);

		if (area > 0.0)
		{
			// 正の値（反時計回り）だったらOuter
			outerLoops.push_back(std::move(loop));
		}
		else
		{
			// 負の値（時計回り）だったらHole
			holeLoops.push_back(std::move(loop));
		}
	}

	//============================================================
	// 4. Grid Polygon を作る
	//============================================================

	struct GridPolygon
	{
		std::vector<VertexId> outer;

		std::vector<std::vector<VertexId>> holes;
	};

	std::vector<GridPolygon>	gridPolygons;

	gridPolygons.reserve(outerLoops.size());

	// 外周のLoopをGrid Polygonに変換
	for (auto& outer : outerLoops)
	{
		GridPolygon polygon;

		polygon.outer = std::move(outer);

		gridPolygons.push_back(std::move(polygon));
	}

	//============================================================
	// 5. Hole の所属する Outer を探す
	//============================================================

	// 頂点がポリゴンの内部にあるか判定するラムダ式の関数
	auto IsPointInsidePolygon = [](const VertexId& point,
		const std::vector<VertexId>& polygon)
		{
			bool inside = false;

			const size_t count = polygon.size();

			if (count < 3)
			{
				return false;
			}

			for (size_t i = 0, j = count - 1; i < count; j = i++)
			{
				const VertexId& a = polygon[i];

				const VertexId& b = polygon[j];

				// レイキャスティング法
				const bool intersect = ((a.y > point.y) != (b.y > point.y)) &&
					(
						static_cast<double>(point.x) <
						(
							static_cast<double>(b.x - a.x) *
							static_cast<double>(point.y - a.y) /
							static_cast<double>(b.y - a.y) +
							static_cast<double>(a.x)
							)
						);

				// 奇数回交差したら内部判定
				if (intersect)
				{
					inside = !inside;
				}
			}

			return inside;
		};

	for (auto& hole : holeLoops)
	{
		if (hole.empty())
		{
			continue;
		}

		// 内部かの判定用頂点
		const VertexId testPoint = hole.front();

		// 所属先のポリゴン
		int bestPolygonIndex = -1;

		// 最小外周面積
		double bestArea = std::numeric_limits<double>::max();

		for (size_t i = 0; i < gridPolygons.size(); ++i)
		{
			const GridPolygon& polygon = gridPolygons[i];

			// HoleがOuterの中にあるか
			if (!IsPointInsidePolygon(testPoint, polygon.outer))
			{
				continue;
			}

			const double area = std::abs(CalculateSignedArea(polygon.outer));

			// 最も小さいOuterを選択
			if (area < bestArea)
			{
				bestArea = area;

				bestPolygonIndex = static_cast<int>(i);
			}
		}

		if (bestPolygonIndex >= 0)
		{
			// Grid Polygonに追加
			gridPolygons[bestPolygonIndex].holes.push_back(std::move(hole));
		}
	}

	//============================================================
	// 6. VertexId → Vector2
	//============================================================

	result.reserve(gridPolygons.size());

	for (auto& gridPolygon : gridPolygons)
	{
		Polygon polygon;

		//--------------------------------------------------------
		// Outer
		//--------------------------------------------------------

		polygon.outer.reserve(gridPolygon.outer.size());

		// 頂点番号を座標に変換
		for (const VertexId& vertex : gridPolygon.outer)
		{
			polygon.outer.push_back(Math::Vector2{
					static_cast<float>(vertex.x) * (tileSize * 0.5f),
					static_cast<float>(vertex.y) * (tileSize * 0.5f)
				});
		}

		//--------------------------------------------------------
		// Holes
		//--------------------------------------------------------

		polygon.holes.reserve(gridPolygon.holes.size());

		for (const auto& gridHole : gridPolygon.holes)
		{
			std::vector<Math::Vector2> hole;

			hole.reserve(gridHole.size());

			// 頂点番号を座標に変換
			for (const VertexId& vertex : gridHole)
			{
				hole.push_back(Math::Vector2{
						static_cast<float>(vertex.x) * (tileSize * 0.5f),
						static_cast<float>(vertex.y) * (tileSize * 0.5f)
					});
			}

			polygon.holes.push_back(std::move(hole));
		}

		result.push_back(std::move(polygon));
	}

	return result;
}

std::vector<NavMeshPolygonBuilder::Polygon> NavMeshPolygonBuilder::BuildObstacles(
	const std::vector<Obstacle>& obstacles, float agentRadius)
{
	std::vector<Polygon> result;

	if (obstacles.empty())
	{
		return result;
	}

	result.reserve(obstacles.size());

	for (const Obstacle& obstacle : obstacles)
	{
		// 不正なAABBは除外
		if (obstacle.minX >= obstacle.maxX || obstacle.minZ >= obstacle.maxZ)
		{
			continue;
		}

		Polygon polygon;

		polygon.outer.reserve(4);

		// agent radius分外側に広げる
		// Bottom Left
		polygon.outer.push_back(
			Math::Vector2{
				obstacle.minX - agentRadius,
				obstacle.minZ - agentRadius
			});

		// Bottom Right
		polygon.outer.push_back(
			Math::Vector2{
				obstacle.maxX + agentRadius,
				obstacle.minZ - agentRadius
			});

		// Top Right
		polygon.outer.push_back(
			Math::Vector2{
				obstacle.maxX + agentRadius,
				obstacle.maxZ + agentRadius
			});

		// Top Left
		polygon.outer.push_back(
			Math::Vector2{
				obstacle.minX - agentRadius,
				obstacle.maxZ + agentRadius
			});

		result.push_back(std::move(polygon));
	}

	return result;
}

//============================================================
// GetTileOrigin
//============================================================

NavMeshPolygonBuilder::VertexId NavMeshPolygonBuilder::GetTileOrigin(
	const Tile& tile, float tileSize) const
{
	// 頂点間隔
	const float vertexGridSize = tileSize * 0.5f;

	// 中心座標のグリッドを計算
	const int centerX = static_cast<int>(std::round(tile.centerPos.x / vertexGridSize));
	const int centerY = static_cast<int>(std::round(tile.centerPos.y / vertexGridSize));

	// 基準の頂点グリッド
	return { centerX - 1, centerY - 1 };
}

//============================================================
// GetTileVertices
//============================================================

std::array<NavMeshPolygonBuilder::VertexId, 4> NavMeshPolygonBuilder::GetTileVertices(
	const Tile& tile, float tileSize) const
{
	// 基準の頂点を取得
	const VertexId origin = GetTileOrigin(tile, tileSize);

	const int tileWidth = static_cast<int>(std::round(tileSize / (tileSize * 0.5f)));

	// ４頂点の座標
	return
	{ {
		{ origin.x, origin.y },
		{ origin.x + tileWidth, origin.y },
		{ origin.x + tileWidth, origin.y + tileWidth },
		{ origin.x, origin.y + tileWidth }
	} };
}

//============================================================
// MakeEdge
//============================================================

NavMeshPolygonBuilder::Edge NavMeshPolygonBuilder::MakeEdge(VertexId a, VertexId b) const
{
	// 辺の向きを統一
	if (a.x > b.x || (a.x == b.x && a.y > b.y))
	{
		std::swap(a, b);
	}

	return	{ a, b };
}