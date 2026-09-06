#include "NavMeshPolygonBuilder.h"

namespace
{
	//============================================================
	// VertexId Hash
	//============================================================

	struct VertexIdHash
	{
		size_t operator()(
			const NavMeshPolygonBuilder::VertexId& v) const noexcept
		{
			const size_t hx = std::hash<int>{}(v.x);
			const size_t hy = std::hash<int>{}(v.y);

			return hx ^ (hy << 1);
		}
	};


	//============================================================
	// Edge Hash
	//============================================================

	struct EdgeHash
	{
		size_t operator()(
			const NavMeshPolygonBuilder::Edge& edge) const noexcept
		{
			const VertexIdHash vertexHash;

			const size_t ha = vertexHash(edge.a);
			const size_t hb = vertexHash(edge.b);

			return ha ^ (hb << 1);
		}
	};


	//============================================================
	// DirectedEdge
	//
	// Edge と違い、a -> b の方向を保持する。
	//============================================================

	struct DirectedEdge
	{
		NavMeshPolygonBuilder::VertexId a;
		NavMeshPolygonBuilder::VertexId b;

		bool operator==(const DirectedEdge& other) const
		{
			return a == other.a &&
				b == other.b;
		}
	};


	struct DirectedEdgeHash
	{
		size_t operator()(const DirectedEdge& edge) const noexcept
		{
			const VertexIdHash vertexHash;

			const size_t ha = vertexHash(edge.a);
			const size_t hb = vertexHash(edge.b);

			return ha ^ (hb << 1);
		}
	};


	//============================================================
	// Cross / Signed Area
	//
	// > 0 : CCW
	// < 0 : CW
	//============================================================

	double CalculateSignedArea(
		const std::vector<NavMeshPolygonBuilder::VertexId>& polygon)
	{
		if (polygon.size() < 3)
		{
			return 0.0;
		}

		double area = 0.0;

		for (size_t i = 0; i < polygon.size(); ++i)
		{
			const auto& a = polygon[i];
			const auto& b = polygon[(i + 1) % polygon.size()];

			area +=
				static_cast<double>(a.x) * static_cast<double>(b.y) -
				static_cast<double>(b.x) * static_cast<double>(a.y);
		}

		return area * 0.5;
	}


	//============================================================
	// Direction
	//
	// Grid 上の VertexId 同士の方向を返す。
	//
	// 0 = East
	// 1 = North
	// 2 = West
	// 3 = South
	//============================================================

	int GetDirection(
		const NavMeshPolygonBuilder::VertexId& from,
		const NavMeshPolygonBuilder::VertexId& to)
	{
		const int dx = to.x - from.x;
		const int dy = to.y - from.y;

		if (dx > 0)
		{
			return 0; // East
		}

		if (dy > 0)
		{
			return 1; // North
		}

		if (dx < 0)
		{
			return 2; // West
		}

		return 3; // South
	}


	//============================================================
	// Turn priority
	//
	// Boundary を辿る際、
	// 「内側を左側に維持する」ような方向を優先する。
	//
	// 同じ Vertex に複数の Edge が集まるケース
	// （特に点で接触するケース）にも対応する。
	//============================================================

	int GetTurnPriority(
		int incomingDirection,
		int outgoingDirection)
	{
		// outgoing が incoming に対して何度回転するか
		const int turn =
			(outgoingDirection - incomingDirection + 4) % 4;

		// 左  -> 0
		// 直進 -> 1
		// 右  -> 2
		// Uターン -> 3
		//
		// CCW の境界を作るため左方向を優先。
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

		int bestPriority =
			std::numeric_limits<int>::max();

		for (const DirectedEdge& edge : edges)
		{
			// 現在のVertexから出ていない
			if (!(edge.a == current))
			{
				continue;
			}

			const int outgoingDirection =
				GetDirection(
					edge.a,
					edge.b);

			const int priority =
				GetTurnPriority(
					incomingDirection,
					outgoingDirection);

			if (!found ||
				priority < bestPriority)
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

	void RemoveDuplicateVertices(
		std::vector<NavMeshPolygonBuilder::VertexId>& polygon)
	{
		if (polygon.empty())
		{
			return;
		}

		std::vector<NavMeshPolygonBuilder::VertexId> result;
		result.reserve(polygon.size());

		for (const auto& vertex : polygon)
		{
			if (!result.empty() &&
				result.back() == vertex)
			{
				continue;
			}

			result.push_back(vertex);
		}

		// 先頭と末尾が同じなら末尾を削除
		if (result.size() >= 2 &&
			result.front() == result.back())
		{
			result.pop_back();
		}

		polygon = std::move(result);
	}


	//============================================================
	// Collinear vertices を削除
	//
	// 例えば
	//
	// (0,0) -> (1,0) -> (2,0)
	//
	// を
	//
	// (0,0) -> (2,0)
	//
	// にする。
	//============================================================

	void RemoveCollinearVertices(
		std::vector<NavMeshPolygonBuilder::VertexId>& polygon)
	{
		if (polygon.size() < 3)
		{
			return;
		}

		bool removed = true;

		while (removed && polygon.size() >= 3)
		{
			removed = false;

			const size_t count = polygon.size();

			for (size_t i = 0; i < count; ++i)
			{
				const auto& prev =
					polygon[(i + count - 1) % count];

				const auto& current =
					polygon[i];

				const auto& next =
					polygon[(i + 1) % count];

				const int dx1 =
					current.x - prev.x;

				const int dy1 =
					current.y - prev.y;

				const int dx2 =
					next.x - current.x;

				const int dy2 =
					next.y - current.y;

				const int cross =
					dx1 * dy2 -
					dy1 * dx2;

				if (cross != 0)
				{
					continue;
				}

				polygon.erase(
					polygon.begin() + static_cast<std::ptrdiff_t>(i));

				removed = true;
				break;
			}
		}
	}
}


//============================================================
// Build
//============================================================

std::vector<NavMeshPolygonBuilder::Polygon>
NavMeshPolygonBuilder::BuildTiles(
	const std::vector<Tile>& tiles,
	float tileSize)
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

	std::unordered_set<
		DirectedEdge,
		DirectedEdgeHash> boundaryEdges;

	boundaryEdges.reserve(
		tiles.size() * 4);

	for (const Tile& tile : tiles)
	{
		const auto vertices =
			GetTileVertices(
				tile,
				tileSize);

		const DirectedEdge edges[] =
		{
			{ vertices[0], vertices[1] },
			{ vertices[1], vertices[2] },
			{ vertices[2], vertices[3] },
			{ vertices[3], vertices[0] }
		};

		for (const DirectedEdge& edge : edges)
		{
			const DirectedEdge reverse{
				edge.b,
				edge.a
			};

			auto reverseIt =
				boundaryEdges.find(reverse);

			if (reverseIt != boundaryEdges.end())
			{
				boundaryEdges.erase(reverseIt);
			}
			else
			{
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

	while (!boundaryEdges.empty())
	{
		auto firstIt =
			boundaryEdges.begin();

		const DirectedEdge firstEdge =
			*firstIt;

		boundaryEdges.erase(firstIt);


		std::vector<VertexId> loop;

		loop.reserve(16);

		loop.push_back(
			firstEdge.a);


		VertexId current =
			firstEdge.b;

		const VertexId start =
			firstEdge.a;

		int incomingDirection =
			GetDirection(
				firstEdge.a,
				firstEdge.b);


		size_t safetyCounter = 0;

		const size_t safetyLimit =
			tiles.size() * 4 + 16;


		while (safetyCounter++ < safetyLimit)
		{
			loop.push_back(
				current);

			if (current == start)
			{
				break;
			}


			DirectedEdge nextEdge;

			if (!FindNextEdge(
				current,
				incomingDirection,
				boundaryEdges,
				nextEdge))
			{
				break;
			}


			boundaryEdges.erase(
				nextEdge);


			incomingDirection =
				GetDirection(
					nextEdge.a,
					nextEdge.b);


			current =
				nextEdge.b;
		}


		// 閉じていない
		if (loop.size() < 4 ||
			loop.back() != start)
		{
			continue;
		}


		// 最後の start を削除
		loop.pop_back();


		RemoveDuplicateVertices(
			loop);

		RemoveCollinearVertices(
			loop);


		if (loop.size() < 3)
		{
			continue;
		}


		const double area =
			CalculateSignedArea(
				loop);

		if (std::abs(area) < 0.000001)
		{
			continue;
		}


		loops.push_back(
			std::move(loop));
	}


	if (loops.empty())
	{
		return result;
	}


	//============================================================
	// 3. Outer / Hole に分類
	//============================================================

	std::vector<std::vector<VertexId>>
		outerLoops;

	std::vector<std::vector<VertexId>>
		holeLoops;


	for (auto& loop : loops)
	{
		const double area =
			CalculateSignedArea(
				loop);

		if (area > 0.0)
		{
			outerLoops.push_back(
				std::move(loop));
		}
		else
		{
			holeLoops.push_back(
				std::move(loop));
		}
	}


	//============================================================
	// 4. Grid Polygon を作る
	//============================================================
	//
	// ここではまだ VertexId のまま。
	//

	struct GridPolygon
	{
		std::vector<VertexId> outer;

		std::vector<
			std::vector<VertexId>> holes;
	};


	std::vector<GridPolygon>
		gridPolygons;

	gridPolygons.reserve(
		outerLoops.size());


	for (auto& outer : outerLoops)
	{
		GridPolygon polygon;

		polygon.outer =
			std::move(outer);

		gridPolygons.push_back(
			std::move(polygon));
	}


	//============================================================
	// 5. Hole の所属する Outer を探す
	//============================================================

	auto IsPointInsidePolygon =
		[](
			const VertexId& point,
			const std::vector<VertexId>& polygon)
		{
			bool inside = false;

			const size_t count =
				polygon.size();

			if (count < 3)
			{
				return false;
			}


			for (size_t i = 0, j = count - 1;
				i < count;
				j = i++)
			{
				const VertexId& a =
					polygon[i];

				const VertexId& b =
					polygon[j];


				const bool intersect =
					((a.y > point.y) !=
						(b.y > point.y)) &&
					(
						static_cast<double>(point.x) <
						(
							static_cast<double>(b.x - a.x) *
							static_cast<double>(point.y - a.y) /
							static_cast<double>(b.y - a.y) +
							static_cast<double>(a.x)
							)
						);


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


		const VertexId testPoint =
			hole.front();


		int bestPolygonIndex = -1;

		double bestArea =
			std::numeric_limits<double>::max();


		for (size_t i = 0;
			i < gridPolygons.size();
			++i)
		{
			const GridPolygon& polygon =
				gridPolygons[i];


			if (!IsPointInsidePolygon(
				testPoint,
				polygon.outer))
			{
				continue;
			}


			const double area =
				std::abs(
					CalculateSignedArea(
						polygon.outer));


			if (area < bestArea)
			{
				bestArea = area;

				bestPolygonIndex =
					static_cast<int>(i);
			}
		}


		if (bestPolygonIndex >= 0)
		{
			gridPolygons[
				bestPolygonIndex]
				.holes
				.push_back(
					std::move(hole));
		}
	}


	//============================================================
	// 6. VertexId → Vector2
	//============================================================

	result.reserve(
		gridPolygons.size());


	for (auto& gridPolygon :
		gridPolygons)
	{
		Polygon polygon;


		//--------------------------------------------------------
		// Outer
		//--------------------------------------------------------

		polygon.outer.reserve(
			gridPolygon.outer.size());


		for (const VertexId& vertex :
			gridPolygon.outer)
		{
			polygon.outer.push_back(
				Math::Vector2{
					static_cast<float>(vertex.x) *
						(tileSize * 0.5f),

					static_cast<float>(vertex.y) *
						(tileSize * 0.5f)
				});
		}


		//--------------------------------------------------------
		// Holes
		//--------------------------------------------------------

		polygon.holes.reserve(
			gridPolygon.holes.size());


		for (const auto& gridHole :
			gridPolygon.holes)
		{
			std::vector<Math::Vector2>
				hole;

			hole.reserve(
				gridHole.size());


			for (const VertexId& vertex :
				gridHole)
			{
				hole.push_back(
					Math::Vector2{
						static_cast<float>(vertex.x) *
							(tileSize * 0.5f),

						static_cast<float>(vertex.y) *
							(tileSize * 0.5f)
					});
			}


			polygon.holes.push_back(
				std::move(hole));
		}


		result.push_back(
			std::move(polygon));
	}


	return result;
}


std::vector<NavMeshPolygonBuilder::Polygon>
NavMeshPolygonBuilder::BuildObstacles(
	const std::vector<Obstacle>& obstacles,
	float agentRadius)
{
	std::vector<Polygon> result;

	if (obstacles.empty())
	{
		return result;
	}

	result.reserve(
		obstacles.size());

	for (const Obstacle& obstacle : obstacles)
	{
		// 不正なAABBは無視
		if (obstacle.minX >= obstacle.maxX ||
			obstacle.minZ >= obstacle.maxZ)
		{
			continue;
		}

		Polygon polygon;

		polygon.outer.reserve(4);

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

		result.push_back(
			std::move(polygon));
	}

	return result;
}

//============================================================
// GetTileOrigin
//============================================================

NavMeshPolygonBuilder::VertexId
NavMeshPolygonBuilder::GetTileOrigin(
	const Tile& tile,
	float tileSize) const
{
	const float vertexGridSize =
		tileSize * 0.5f;

	const int centerX =
		static_cast<int>(
			std::round(
				tile.centerPos.x /
				vertexGridSize));

	const int centerY =
		static_cast<int>(
			std::round(
				tile.centerPos.y /
				vertexGridSize));

	return
	{
		centerX - 1,
		centerY - 1
	};
}


//============================================================
// GetTileVertices
//============================================================

std::array<NavMeshPolygonBuilder::VertexId, 4>
NavMeshPolygonBuilder::GetTileVertices(
	const Tile& tile,
	float tileSize) const
{
	const VertexId origin =
		GetTileOrigin(
			tile,
			tileSize);

	const int tileWidth =
		static_cast<int>(
			std::round(
				tileSize /
				(tileSize * 0.5f)));

	return
	{ {
		{ origin.x,          origin.y          },
		{ origin.x + tileWidth, origin.y       },
		{ origin.x + tileWidth, origin.y + tileWidth },
		{ origin.x,          origin.y + tileWidth }
	} };
}

//============================================================
// MakeEdge
//
// 無向 Edge として比較したい場合に使用。
//============================================================

NavMeshPolygonBuilder::Edge
NavMeshPolygonBuilder::MakeEdge(
	VertexId a,
	VertexId b) const
{
	if (a.x > b.x ||
		(a.x == b.x && a.y > b.y))
	{
		std::swap(a, b);
	}

	return
	{
		a,
		b
	};
}