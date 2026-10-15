#include "NavMeshGenerator.h"

std::vector<PolygonBoolean::Polygon>
NavMeshGenerator::Generate(
	const Input& input) const
{
	std::vector<PolygonBoolean::Polygon> result;

	if (input.tiles.empty())
	{
		return result;
	}

	if (input.tileSize <= 0.0f)
	{
		return result;
	}

	if (input.agentRadius < 0.0f)
	{
		return result;
	}


	//============================================================
	// Builder
	//============================================================

	NavMeshPolygonBuilder builder;


	//============================================================
	// 1. Tile → Polygon
	//============================================================

	const auto tilePolygons =
		builder.BuildTiles(
			input.tiles,
			input.tileSize);

	if (tilePolygons.empty())
	{
		return result;
	}


	//============================================================
	// 2. Tile → PolygonBoolean
	//============================================================

	std::vector<PolygonBoolean::Polygon>
		tileBooleanPolygons;

	tileBooleanPolygons.reserve(
		tilePolygons.size());

	for (const auto& tilePolygon :
		tilePolygons)
	{
		tileBooleanPolygons.emplace_back(
			PolygonBoolean::Polygon
			{
				tilePolygon.outer,
				tilePolygon.holes
			});
	}


	//============================================================
	// 3. Tile Union
	//
	// 全Walkable Tileをまとめる
	//============================================================

	result =
		PolygonBoolean::Union(
			tileBooleanPolygons);

	if (result.empty())
	{
		return result;
	}


	//============================================================
	// 4. Obstacle → Polygon
	//============================================================

	const auto obstaclePolygons =
		builder.BuildObstacles(
			input.obstacles,
			input.agentRadius);

	if (obstaclePolygons.empty())
	{
		return result;
	}


	//============================================================
	// 5. Obstacle → PolygonBoolean
	//============================================================

	std::vector<PolygonBoolean::Polygon>
		obstacleBooleanPolygons;

	obstacleBooleanPolygons.reserve(
		obstaclePolygons.size());

	for (const auto& obstacle :
		obstaclePolygons)
	{
		if (obstacle.outer.size() < 3)
		{
			continue;
		}

		obstacleBooleanPolygons.emplace_back(
			PolygonBoolean::Polygon
			{
				obstacle.outer,
				obstacle.holes
			});
	}

	if (obstacleBooleanPolygons.empty())
	{
		return result;
	}

	//============================================================
	// 6. Obstacle Union
	//
	// 複数のObstacleを一つの障害物領域にまとめる
	//
	// A + B + C
	//     ↓
	// Union
	//     ↓
	// All Obstacles
	//============================================================

	const auto unionedObstacles =
		PolygonBoolean::Union(
			obstacleBooleanPolygons);

	if (unionedObstacles.empty())
	{
		return result;
	}
	

	//============================================================
	// 7. Walkable - All Obstacles
	//
	// Obstacleを1個ずつDifferenceしない。
	// Union済みのObstacleをまとめて引く。
	//============================================================

	result =
		PolygonBoolean::Difference(
			result,
			unionedObstacles);

	return result;
}


//============================================================
// Generate
//============================================================

void NavMeshGenerator::Generate(
	NavMeshManager& navMeshManager,
	const std::vector<std::pair<
	std::shared_ptr<KdModelData>,
	Math::Matrix>>&meshes,
	const std::vector<NavMeshObstacleBounds>& obstacles)
{
	//--------------------------------------------------------
	// Clear
	//--------------------------------------------------------

	navMeshManager.Clear();

	//--------------------------------------------------------
	// Polygon生成
	//--------------------------------------------------------

	for (const auto& mesh : meshes)
	{
		const auto& modelData =
			mesh.first;

		const auto& worldMatrix =
			mesh.second;

		//----------------------------------------------------
		// ModelDataチェック
		//----------------------------------------------------

		if (!modelData)
		{
			continue;
		}

		GeneratePolygons(
			navMeshManager,
			modelData,
			worldMatrix,
			obstacles);
	}

	//--------------------------------------------------------
	// Connection
	//--------------------------------------------------------

	BuildConnections(
		navMeshManager);
}

//============================================================
// Tile 方式の Generate
//============================================================

void NavMeshGenerator::Generate(
	NavMeshManager& navMeshManager,
	const std::vector<NavMeshTile>& tiles,
	const std::vector<NavMeshObstacleBounds>& obstacles)
{
	//--------------------------------------------------------
	// Clear
	//--------------------------------------------------------

	navMeshManager.Clear();

	//--------------------------------------------------------
	// Stage TileからPolygon生成
	//--------------------------------------------------------

	for (const auto& tile : tiles)
	{
		GenerateTilePolygons(
			navMeshManager,
			tile,
			obstacles);
	}

	//--------------------------------------------------------
	// Connection
	//--------------------------------------------------------

	BuildConnections(
		navMeshManager);
}

//============================================================
// Generate SoloMesh
//============================================================

void NavMeshGenerator::GenerateSoloMesh(
	const std::vector<NavMeshTile>& tiles,
	const std::vector<NavMeshObstacleBounds>& obstacles,
	NavMeshManager& navMeshManager)
{
	//--------------------------------------------------------
	// Clear
	//--------------------------------------------------------

	navMeshManager.Clear();

	//--------------------------------------------------------
	// Walkable Region生成
	//--------------------------------------------------------

	WalkableRegion region =
		BuildWalkableRegion(
			tiles,
			obstacles);

	//--------------------------------------------------------
	// Delaunay
	//--------------------------------------------------------

	std::vector<Math::Vector3> vertices;

	std::vector<DelaunayTriangle> triangles;

	DelaunayTriangulate(
		region,
		vertices,
		triangles);

	//--------------------------------------------------------
	// TriangleをNavMeshへ登録
	//--------------------------------------------------------

	for (const auto& triangle : triangles)
	{
		if (triangle.a < 0 ||
			triangle.b < 0 ||
			triangle.c < 0)
		{
			continue;
		}

		if (triangle.a >= vertices.size() ||
			triangle.b >= vertices.size() ||
			triangle.c >= vertices.size())
		{
			continue;
		}

		const Math::Vector3& p0 =
			vertices[triangle.a];

		const Math::Vector3& p1 =
			vertices[triangle.b];

		const Math::Vector3& p2 =
			vertices[triangle.c];

		//----------------------------------------------------
		// Walkableチェック
		//----------------------------------------------------

		if (!IsTriangleInsideWalkableRegion(
			p0,
			p1,
			p2,
			region))
		{
			continue;
		}

		//----------------------------------------------------
		// Polygon登録
		//----------------------------------------------------

		navMeshManager.CreatePolygon(
			{
				p0,
				p1,
				p2
			});
	}

	//--------------------------------------------------------
	// Connection
	//--------------------------------------------------------

	BuildConnections(
		navMeshManager);
}

//============================================================
// Generate Tile Polygons
//============================================================

void NavMeshGenerator::GenerateTilePolygons(
	NavMeshManager& navMeshManager,
	const NavMeshTile& tile,
	const std::vector<NavMeshObstacleBounds>& obstacles)
{
	//--------------------------------------------------------
	// Tile
	//
	// v0 -------- v1
	// |          / |
	// |        /   |
	// |      /     |
	// |    /       |
	// |  /         |
	// v2 -------- v3
	//
	// Triangle 0 : v0 -> v1 -> v2
	// Triangle 1 : v1 -> v3 -> v2
	//
	//--------------------------------------------------------

	const std::array<
		std::array<Math::Vector3, 3>,
		2> triangles =
	{ {
		{
			tile.v0,
			tile.v2,
			tile.v1
		},
		{
			tile.v1,
			tile.v2,
			tile.v3
		}
	} };

	//--------------------------------------------------------
	// Triangleごとに処理
	//--------------------------------------------------------

	for (const auto& triangle : triangles)
	{
		const Math::Vector3& p0 =
			triangle[0];

		const Math::Vector3& p1 =
			triangle[1];

		const Math::Vector3& p2 =
			triangle[2];

		//----------------------------------------------------
		// Walkable
		//----------------------------------------------------

		if (!IsWalkable(
			p0,
			p1,
			p2))
		{
			continue;
		}

		//----------------------------------------------------
		// Obstacle Clip
		//----------------------------------------------------

		std::vector<
			std::vector<Math::Vector3>>
			clippedPolygons;

		ClipTriangleByObstacles(
			p0,
			p1,
			p2,
			obstacles,
			clippedPolygons);

		//----------------------------------------------------
		// Clip後のPolygonをTriangle化
		//----------------------------------------------------

		for (const auto& polygon :
			clippedPolygons)
		{
			std::vector<
				std::vector<Math::Vector3>>
				clippedTriangles;

			TriangulatePolygon(
				polygon,
				clippedTriangles);

			//------------------------------------------------
			// NavMesh登録
			//------------------------------------------------

			for (const auto& clippedTriangle :
				clippedTriangles)
			{
				navMeshManager.CreatePolygon(
					clippedTriangle);
			}
		}
	}
}

//============================================================
// Generate Polygons
//============================================================

void NavMeshGenerator::GeneratePolygons(
	NavMeshManager& navMeshManager,
	const std::shared_ptr<KdModelData>& modelData,
	const Math::Matrix& worldMatrix,
	const std::vector<NavMeshObstacleBounds>& obstacles)
{
	//--------------------------------------------------------
	// ModelDataチェック
	//--------------------------------------------------------

	if (!modelData)
	{
		return;
	}

	//--------------------------------------------------------
	// Node取得
	//--------------------------------------------------------

	const auto& nodes =
		modelData->GetOriginalNodes();

	//--------------------------------------------------------
	// COL Node取得
	//--------------------------------------------------------

	const auto& collisionNodeIndices =
		modelData->GetCollisionMeshNodeIndices();

	//--------------------------------------------------------
	// COL Nodeを処理
	//--------------------------------------------------------

	for (int nodeIndex : collisionNodeIndices)
	{
		//----------------------------------------------------
		// Node Indexチェック
		//----------------------------------------------------

		if (nodeIndex < 0 ||
			nodeIndex >= static_cast<int>(nodes.size()))
		{
			continue;
		}

		const auto& node =
			nodes[nodeIndex];

		//----------------------------------------------------
		// Meshチェック
		//----------------------------------------------------

		if (!node.m_spMesh)
		{
			continue;
		}

		//----------------------------------------------------
		// Meshデータ取得
		//----------------------------------------------------

		const auto& vertices =
			node.m_spMesh->GetVertexPositions();

		const auto& faces =
			node.m_spMesh->GetFaces();

		if (vertices.empty() ||
			faces.empty())
		{
			continue;
		}

		//----------------------------------------------------
		// Faceを1枚ずつ処理
		//----------------------------------------------------

		for (const auto& face : faces)
		{
			//------------------------------------------------
			// Index取得
			//------------------------------------------------

			const UINT index0 =
				face.Idx[0];

			const UINT index1 =
				face.Idx[1];

			const UINT index2 =
				face.Idx[2];

			//------------------------------------------------
			// Index範囲チェック
			//------------------------------------------------

			if (index0 >= vertices.size() ||
				index1 >= vertices.size() ||
				index2 >= vertices.size())
			{
				continue;
			}

			//------------------------------------------------
			// Local座標取得
			//------------------------------------------------

			Math::Vector3 p0 =
				vertices[index0];

			Math::Vector3 p1 =
				vertices[index1];

			Math::Vector3 p2 =
				vertices[index2];

			//------------------------------------------------
			// Mesh Local
			// ↓
			// Node World
			//------------------------------------------------

			p0 = Math::Vector3::Transform(
				p0,
				node.m_worldTransform);

			p1 = Math::Vector3::Transform(
				p1,
				node.m_worldTransform);

			p2 = Math::Vector3::Transform(
				p2,
				node.m_worldTransform);

			//------------------------------------------------
			// Model World
			//------------------------------------------------
			//
			// NodeのWorldTransformに加えて、
			// Model自体のWorldMatrixを適用する。
			//
			//------------------------------------------------

			p0 = Math::Vector3::Transform(
				p0,
				worldMatrix);

			p1 = Math::Vector3::Transform(
				p1,
				worldMatrix);

			p2 = Math::Vector3::Transform(
				p2,
				worldMatrix);

			//------------------------------------------------
			// 歩行可能判定
			//------------------------------------------------

			if (!IsWalkable(
				p0,
				p1,
				p2))
			{
				continue;
			}

			//------------------------------------------------
			// ObstacleでTriangleを切り抜く
			//------------------------------------------------

			std::vector<std::vector<Math::Vector3>> clippedPolygons;

			ClipTriangleByObstacles(
				p0,
				p1,
				p2,
				obstacles,
				clippedPolygons);

			//------------------------------------------------
			// 切り抜いたPolygonをTriangleに分割
			//------------------------------------------------

			for (const auto& polygon : clippedPolygons)
			{
				std::vector<std::vector<Math::Vector3>> triangles;

				TriangulatePolygon(
					polygon,
					triangles);

				//------------------------------------------------
				// TriangleをNavMeshとして登録
				//------------------------------------------------

				for (const auto& triangle : triangles)
				{
					navMeshManager.CreatePolygon(
						triangle);
				}
			}
		}
	}
}


//============================================================
// Is Walkable
//============================================================
//
// 三角形の法線から歩行可能か判定する。
//
// XZ平面が地面。
// Y方向が上。
// 最大傾斜45度を想定。
//
//============================================================

bool NavMeshGenerator::IsWalkable(
	const Math::Vector3& p0,
	const Math::Vector3& p1,
	const Math::Vector3& p2) const
{
	//--------------------------------------------------------
	// Edge
	//--------------------------------------------------------

	const Math::Vector3 edge1 =
		p1 - p0;

	const Math::Vector3 edge2 =
		p2 - p0;

	//--------------------------------------------------------
	// 法線
	//--------------------------------------------------------

	Math::Vector3 normal =
		edge1.Cross(edge2);

	//--------------------------------------------------------
	// 面積ゼロチェック
	//--------------------------------------------------------

	if (normal.LengthSquared() <= 0.000001f)
	{
		return false;
	}

	//--------------------------------------------------------
	// 正規化
	//--------------------------------------------------------

	normal.Normalize();

	//--------------------------------------------------------
	// 最大傾斜45度
	//--------------------------------------------------------

	constexpr float MIN_NORMAL_Y =
		0.70710678f;

	//--------------------------------------------------------
	// 上向きの面だけ許可
	//--------------------------------------------------------

	if (normal.y < MIN_NORMAL_Y)
	{
		return false;
	}

	return true;
}

void NavMeshGenerator::ClipTriangleByObstacles(
	const Math::Vector3& p0,
	const Math::Vector3& p1,
	const Math::Vector3& p2,
	const std::vector<NavMeshObstacleBounds>& obstacles,
	std::vector<std::vector<Math::Vector3>>& result) const
{
	//--------------------------------------------------------
	// 初期Triangle
	//--------------------------------------------------------

	std::vector<std::vector<Math::Vector3>> polygons;

	polygons.push_back(
		{
			p0,
			p1,
			p2
		});

	//--------------------------------------------------------
	// Obstacleを1つずつ処理
	//--------------------------------------------------------

	for (const auto& obstacle : obstacles)
	{
		std::vector<std::vector<Math::Vector3>> nextPolygons;

		for (const auto& polygon : polygons)
		{
			ClipPolygonByObstacle(
				polygon,
				obstacle,
				nextPolygons);
		}

		polygons =
			std::move(nextPolygons);

		//----------------------------------------------------
		// 全て削除された
		//----------------------------------------------------

		if (polygons.empty())
		{
			break;
		}
	}

	result =
		std::move(polygons);
}

void NavMeshGenerator::ClipPolygonByObstacle(
	const std::vector<Math::Vector3>& polygon,
	const NavMeshObstacleBounds& obstacle,
	std::vector<std::vector<Math::Vector3>>& result) const
{
	//--------------------------------------------------------
	// Agent Radius分だけBoundsを拡張
	//--------------------------------------------------------

	const float minX =
		obstacle.minX - m_agentRadius;

	const float maxX =
		obstacle.maxX + m_agentRadius;

	const float minZ =
		obstacle.minZ - m_agentRadius;

	const float maxZ =
		obstacle.maxZ + m_agentRadius;

	//--------------------------------------------------------
	// 現在「Obstacle内部に残っている候補」
	//--------------------------------------------------------

	std::vector<std::vector<Math::Vector3>> insidePolygons;

	insidePolygons.push_back(
		polygon);

	//--------------------------------------------------------
	// X >= minX
	//--------------------------------------------------------

	{
		std::vector<std::vector<Math::Vector3>> nextInside;

		for (const auto& current : insidePolygons)
		{
			std::vector<Math::Vector3> inside;
			std::vector<Math::Vector3> outside;

			SplitPolygonByLine(
				current,
				0,          // X
				minX,
				true,       // >=
				inside,
				outside);

			AddPolygonIfValid(
				result,
				std::move(outside));

			AddPolygonIfValid(
				nextInside,
				std::move(inside));
		}

		insidePolygons =
			std::move(nextInside);
	}

	//--------------------------------------------------------
	// X <= maxX
	//--------------------------------------------------------

	{
		std::vector<std::vector<Math::Vector3>> nextInside;

		for (const auto& current : insidePolygons)
		{
			std::vector<Math::Vector3> inside;
			std::vector<Math::Vector3> outside;

			SplitPolygonByLine(
				current,
				0,          // X
				maxX,
				false,      // <=
				inside,
				outside);

			AddPolygonIfValid(
				result,
				std::move(outside));

			AddPolygonIfValid(
				nextInside,
				std::move(inside));
		}

		insidePolygons =
			std::move(nextInside);
	}

	//--------------------------------------------------------
	// Z >= minZ
	//--------------------------------------------------------

	{
		std::vector<std::vector<Math::Vector3>> nextInside;

		for (const auto& current : insidePolygons)
		{
			std::vector<Math::Vector3> inside;
			std::vector<Math::Vector3> outside;

			SplitPolygonByLine(
				current,
				1,          // Z
				minZ,
				true,       // >=
				inside,
				outside);

			AddPolygonIfValid(
				result,
				std::move(outside));

			AddPolygonIfValid(
				nextInside,
				std::move(inside));
		}

		insidePolygons =
			std::move(nextInside);
	}

	//--------------------------------------------------------
	// Z <= maxZ
	//--------------------------------------------------------

	{
		std::vector<std::vector<Math::Vector3>> nextInside;

		for (const auto& current : insidePolygons)
		{
			std::vector<Math::Vector3> inside;
			std::vector<Math::Vector3> outside;

			SplitPolygonByLine(
				current,
				1,          // Z
				maxZ,
				false,      // <=
				inside,
				outside);

			AddPolygonIfValid(
				result,
				std::move(outside));

			AddPolygonIfValid(
				nextInside,
				std::move(inside));
		}

		insidePolygons =
			std::move(nextInside);
	}

	//--------------------------------------------------------
	// 最後までinsideに残った部分は
	// 完全にObstacle内部なので捨てる
	//--------------------------------------------------------
}

void NavMeshGenerator::SplitPolygonByLine(
	const std::vector<Math::Vector3>& polygon,
	int axis,
	float value,
	bool keepGreater,
	std::vector<Math::Vector3>& inside,
	std::vector<Math::Vector3>& outside) const
{
	if (polygon.size() < 3)
	{
		return;
	}

	constexpr float EPSILON =
		0.000001f;

	//--------------------------------------------------------
	// Pointがinside側か
	//--------------------------------------------------------

	auto IsInside =
		[axis, value, keepGreater](const Math::Vector3& p)
		{
			const float coordinate =
				(axis == 0)
				? p.x
				: p.z;

			if (keepGreater)
			{
				return coordinate >= value;
			}
			else
			{
				return coordinate <= value;
			}
		};

	//--------------------------------------------------------
	// 2点間の交点
	//--------------------------------------------------------

	auto CalculateIntersection =
		[axis, value](const Math::Vector3& a,
			const Math::Vector3& b)
		{
			const float aValue =
				(axis == 0)
				? a.x
				: a.z;

			const float bValue =
				(axis == 0)
				? b.x
				: b.z;

			const float denominator =
				bValue - aValue;

			if (std::abs(denominator) <= 0.000001f)
			{
				return a;
			}

			const float t =
				(value - aValue) /
				denominator;

			return a +
				(b - a) * t;
		};

	//--------------------------------------------------------
	// Sutherland-Hodgman形式で
	// inside / outside両方を生成
	//--------------------------------------------------------

	for (size_t i = 0;
		i < polygon.size();
		++i)
	{
		const Math::Vector3& current =
			polygon[i];

		const Math::Vector3& next =
			polygon[
				(i + 1) % polygon.size()
			];

		const bool currentInside =
			IsInside(current);

		const bool nextInside =
			IsInside(next);

		//----------------------------------------------------
		// Inside → Inside
		//----------------------------------------------------

		if (currentInside &&
			nextInside)
		{
			inside.push_back(next);
		}
		//----------------------------------------------------
		// Inside → Outside
		//----------------------------------------------------
		else if (currentInside &&
			!nextInside)
		{
			const Math::Vector3 intersection =
				CalculateIntersection(
					current,
					next);

			inside.push_back(
				intersection);

			outside.push_back(
				intersection);

			outside.push_back(
				next);
		}
		//----------------------------------------------------
		// Outside → Inside
		//----------------------------------------------------
		else if (!currentInside &&
			nextInside)
		{
			const Math::Vector3 intersection =
				CalculateIntersection(
					current,
					next);

			outside.push_back(
				intersection);

			inside.push_back(
				intersection);

			inside.push_back(
				next);
		}
		//----------------------------------------------------
		// Outside → Outside
		//----------------------------------------------------
		else
		{
			outside.push_back(next);
		}
	}
}

void NavMeshGenerator::AddPolygonIfValid(
	std::vector<std::vector<Math::Vector3>>& polygons,
	std::vector<Math::Vector3> polygon) const
{
	if (polygon.size() < 3)
	{
		return;
	}

	//--------------------------------------------------------
	// 連続する重複頂点を削除
	//--------------------------------------------------------

	constexpr float EPSILON_SQ =
		0.0001f * 0.0001f;

	std::vector<Math::Vector3> cleaned;

	for (const auto& vertex : polygon)
	{
		if (!cleaned.empty())
		{
			const Math::Vector3 diff =
				vertex - cleaned.back();

			if (diff.LengthSquared() <= EPSILON_SQ)
			{
				continue;
			}
		}

		cleaned.push_back(vertex);
	}

	//--------------------------------------------------------
	// 最後と最初が同じ場合
	//--------------------------------------------------------

	if (cleaned.size() >= 2)
	{
		const Math::Vector3 diff =
			cleaned.front() -
			cleaned.back();

		if (diff.LengthSquared() <= EPSILON_SQ)
		{
			cleaned.pop_back();
		}
	}

	if (cleaned.size() < 3)
	{
		return;
	}

	//--------------------------------------------------------
	// Polygon登録
	//--------------------------------------------------------

	polygons.push_back(
		std::move(cleaned));
}

void NavMeshGenerator::TriangulatePolygon(
	const std::vector<Math::Vector3>& polygon,
	std::vector<std::vector<Math::Vector3>>& triangles) const
{
	if (polygon.size() < 3)
	{
		return;
	}

	//--------------------------------------------------------
	// Triangle
	//--------------------------------------------------------

	for (size_t i = 1;
		i + 1 < polygon.size();
		++i)
	{
		const std::vector<Math::Vector3> triangle =
		{
			polygon[0],
			polygon[i],
			polygon[i + 1]
		};

		//----------------------------------------------------
		// 面積チェック
		//----------------------------------------------------

		const Math::Vector3 edge1 =
			triangle[1] -
			triangle[0];

		const Math::Vector3 edge2 =
			triangle[2] -
			triangle[0];

		const Math::Vector3 cross =
			edge1.Cross(edge2);

		if (cross.LengthSquared() <=
			0.000001f)
		{
			continue;
		}

		triangles.push_back(
			triangle);
	}
}

//============================================================
// Build Connections
//============================================================

void NavMeshGenerator::BuildConnections(
	NavMeshManager& navMeshManager)
{
	const auto& polygons =
		navMeshManager.GetPolygons();

	//--------------------------------------------------------
	// 全Polygonを比較
	//--------------------------------------------------------

	for (size_t i = 0;
		i < polygons.size();
		++i)
	{
		for (size_t j = i + 1;
			j < polygons.size();
			++j)
		{
			const auto& polygonA =
				polygons[i];

			const auto& polygonB =
				polygons[j];

			//------------------------------------------------
			// 辺を共有しているか
			//------------------------------------------------

			if (!polygonA->SharesEdgeWith(
				*polygonB))
			{
				continue;
			}

			//------------------------------------------------
			// 接続
			//------------------------------------------------

			navMeshManager.ConnectPolygons(
				polygonA->GetID(),
				polygonB->GetID());
		}
	}
}

//============================================================
// Walkable Region
//============================================================

NavMeshGenerator::WalkableRegion
NavMeshGenerator::BuildWalkableRegion(
	const std::vector<NavMeshTile>& tiles,
	const std::vector<NavMeshObstacleBounds>& obstacles) const
{
	WalkableRegion region;

	//--------------------------------------------------------
	// Ground
	//--------------------------------------------------------

	for (const auto& tile : tiles)
	{
		region.outerBoundaries.push_back(
			{
				tile.v0,
				tile.v1,
				tile.v3,
				tile.v2
			});
	}

	//--------------------------------------------------------
	// Obstacle
	//--------------------------------------------------------

	for (const auto& obstacle : obstacles)
	{
		const float minX =
			obstacle.minX - m_agentRadius;

		const float maxX =
			obstacle.maxX + m_agentRadius;

		const float minZ =
			obstacle.minZ - m_agentRadius;

		const float maxZ =
			obstacle.maxZ + m_agentRadius;

		//----------------------------------------------------
		// Groundのどれかと重なっているか
		//----------------------------------------------------

		bool overlapsGround = false;

		for (const auto& boundary :
			region.outerBoundaries)
		{
			float groundMinX =
				std::numeric_limits<float>::max();

			float groundMaxX =
				std::numeric_limits<float>::lowest();

			float groundMinZ =
				std::numeric_limits<float>::max();

			float groundMaxZ =
				std::numeric_limits<float>::lowest();

			for (const auto& p : boundary)
			{
				groundMinX =
					std::min(groundMinX, p.x);

				groundMaxX =
					std::max(groundMaxX, p.x);

				groundMinZ =
					std::min(groundMinZ, p.z);

				groundMaxZ =
					std::max(groundMaxZ, p.z);
			}

			if (maxX > groundMinX &&
				minX < groundMaxX &&
				maxZ > groundMinZ &&
				minZ < groundMaxZ)
			{
				overlapsGround = true;
				break;
			}
		}

		if (!overlapsGround)
		{
			continue;
		}

		//----------------------------------------------------
		// Hole
		//----------------------------------------------------

		const float y =
			region.outerBoundaries[0][0].y;

		region.holes.push_back(
			{
				{ minX, y, minZ },
				{ maxX, y, minZ },
				{ maxX, y, maxZ },
				{ minX, y, maxZ }
			});
	}

	return region;
}

//============================================================
// Delaunay
//============================================================

void NavMeshGenerator::DelaunayTriangulate(
	const WalkableRegion& region,
	std::vector<Math::Vector3>& vertices,
	std::vector<DelaunayTriangle>& triangles) const
{
	vertices.clear();
	triangles.clear();

	//--------------------------------------------------------
	// Walkable Regionの頂点を収集
	//--------------------------------------------------------

	for (const auto& boundary :
		region.outerBoundaries)
	{
		for (const auto& p :
			boundary)
		{
			vertices.push_back(p);
		}
	}

	for (const auto& hole :
		region.holes)
	{
		for (const auto& p :
			hole)
		{
			vertices.push_back(p);
		}
	}

	const int originalVertexCount =
		static_cast<int>(vertices.size());

	if (originalVertexCount < 3)
	{
		return;
	}

	//--------------------------------------------------------
	// Bounds
	//--------------------------------------------------------

	float minX =
		std::numeric_limits<float>::max();

	float maxX =
		std::numeric_limits<float>::lowest();

	float minZ =
		std::numeric_limits<float>::max();

	float maxZ =
		std::numeric_limits<float>::lowest();

	for (const auto& p : vertices)
	{
		minX = std::min(minX, p.x);
		maxX = std::max(maxX, p.x);

		minZ = std::min(minZ, p.z);
		maxZ = std::max(maxZ, p.z);
	}

	//--------------------------------------------------------
	// Super Triangle
	//--------------------------------------------------------

	const float width =
		maxX - minX;

	const float height =
		maxZ - minZ;

	const float delta =
		std::max(width, height) * 10.0f + 1.0f;

	const float centerX =
		(minX + maxX) * 0.5f;

	const float centerZ =
		(minZ + maxZ) * 0.5f;

	const int superA =
		static_cast<int>(vertices.size());

	const int superB =
		superA + 1;

	const int superC =
		superA + 2;

	if (region.outerBoundaries.empty())
	{
		return;
	}

	const float y =
		region.outerBoundaries[0][0].y;

	vertices.push_back(
		{
			centerX - 2.0f * delta,
			y,
			centerZ - delta
		});

	vertices.push_back(
		{
			centerX,
			y,
			centerZ + 2.0f * delta
		});

	vertices.push_back(
		{
			centerX + 2.0f * delta,
			y,
			centerZ - delta
		});

	//--------------------------------------------------------
	// Super Triangle
	//--------------------------------------------------------

	triangles.push_back(
		{
			superA,
			superB,
			superC
		});

	//--------------------------------------------------------
	// Bowyer-Watson
	//--------------------------------------------------------

	for (int pointIndex = 0;
		pointIndex < originalVertexCount;
		++pointIndex)
	{
		const Math::Vector3& point =
			vertices[pointIndex];

		//----------------------------------------------------
		// Bad Triangle
		//----------------------------------------------------

		std::vector<int> badTriangles;

		for (int i = 0;
			i < static_cast<int>(triangles.size());
			++i)
		{
			const auto& triangle =
				triangles[i];

			if (IsPointInsideCircumcircle(
				point,
				vertices[triangle.a],
				vertices[triangle.b],
				vertices[triangle.c]))
			{
				badTriangles.push_back(i);
			}
		}

		//----------------------------------------------------
		// Polygon Hole
		//----------------------------------------------------

		std::vector<DelaunayEdge> polygon;

		auto AddEdge =
			[&](const DelaunayEdge& edge)
			{
				auto it =
					std::find_if(
						polygon.begin(),
						polygon.end(),
						[&](const DelaunayEdge& e)
						{
							return
								(e.a == edge.a && e.b == edge.b) ||
								(e.a == edge.b && e.b == edge.a);
						});

				if (it != polygon.end())
				{
					polygon.erase(it);
				}
				else
				{
					polygon.push_back(edge);
				}
			};

		//----------------------------------------------------
		// Bad Triangleの辺を収集
		//----------------------------------------------------

		for (int index :
		badTriangles)
		{
			const auto& triangle =
				triangles[index];

			AddEdge(
				{
					triangle.a,
					triangle.b
				});

			AddEdge(
				{
					triangle.b,
					triangle.c
				});

			AddEdge(
				{
					triangle.c,
					triangle.a
				});
		}

		//----------------------------------------------------
		// Bad Triangle削除
		//----------------------------------------------------

		std::vector<DelaunayTriangle> newTriangles;

		for (int i = 0;
			i < static_cast<int>(triangles.size());
			++i)
		{
			if (std::find(
				badTriangles.begin(),
				badTriangles.end(),
				i) ==
				badTriangles.end())
			{
				newTriangles.push_back(
					triangles[i]);
			}
		}

		triangles =
			std::move(newTriangles);

		//----------------------------------------------------
		// 新しいTriangle
		//----------------------------------------------------

		for (const auto& edge :
			polygon)
		{
			DelaunayTriangle triangle;

			triangle.a =
				edge.a;

			triangle.b =
				edge.b;

			triangle.c =
				pointIndex;

			triangles.push_back(
				triangle);
		}
	}

	//--------------------------------------------------------
	// Super Triangleを含むものを削除
	//--------------------------------------------------------

	std::vector<DelaunayTriangle> result;

	for (const auto& triangle :
		triangles)
	{
		if (triangle.a >= originalVertexCount ||
			triangle.b >= originalVertexCount ||
			triangle.c >= originalVertexCount)
		{
			continue;
		}

		result.push_back(
			triangle);
	}

	triangles.clear();

	for (const auto& triangle : result)
	{
		const Math::Vector3& p0 =
			vertices[triangle.a];

		const Math::Vector3& p1 =
			vertices[triangle.b];

		const Math::Vector3& p2 =
			vertices[triangle.c];

		if (!IsTriangleInsideWalkableRegion(
			p0,
			p1,
			p2,
			region))
		{
			continue;
		}

		triangles.push_back(
			triangle);
	}

	//--------------------------------------------------------
	// Super Triangle頂点を削除
	//--------------------------------------------------------

	vertices.resize(
		originalVertexCount);
}

//============================================================
// Triangle判定
//============================================================

bool NavMeshGenerator::IsTriangleInsideWalkableRegion(
	const Math::Vector3& p0,
	const Math::Vector3& p1,
	const Math::Vector3& p2,
	const WalkableRegion& region) const
{
	//--------------------------------------------------------
	// Triangle Center
	//--------------------------------------------------------

	const float centerX =
		(p0.x + p1.x + p2.x) / 3.0f;

	const float centerZ =
		(p0.z + p1.z + p2.z) / 3.0f;

	//--------------------------------------------------------
	// 3頂点が同じGround内にあるか
	//--------------------------------------------------------

	bool insideSameGround = false;

	for (const auto& boundary :
		region.outerBoundaries)
	{
		const bool p0Inside =
			IsPointInsidePolygon(
				p0.x,
				p0.z,
				boundary);

		const bool p1Inside =
			IsPointInsidePolygon(
				p1.x,
				p1.z,
				boundary);

		const bool p2Inside =
			IsPointInsidePolygon(
				p2.x,
				p2.z,
				boundary);

		if (p0Inside &&
			p1Inside &&
			p2Inside)
		{
			insideSameGround = true;
			break;
		}
	}

	if (!insideSameGround)
	{
		return false;
	}

	//--------------------------------------------------------
	// Hole
	//--------------------------------------------------------

	if (IsPointInsideHole(
		centerX,
		centerZ,
		region))
	{
		return false;
	}

	//--------------------------------------------------------
	// Holeとの交差
	//--------------------------------------------------------

	for (const auto& hole :
		region.holes)
	{
		if (TriangleIntersectsHole(
			p0,
			p1,
			p2,
			hole))
		{
			return false;
		}
	}

	return true;
}

bool NavMeshGenerator::IsPointInsidePolygon(
	float x,
	float z,
	const std::vector<Math::Vector3>& polygon) const
{
	if (polygon.size() < 3)
	{
		return false;
	}
	constexpr float EPSILON = 0.0001f;

	//--------------------------------------------------------
	// 境界上ならInsideとして扱う
	//--------------------------------------------------------

	auto IsPointOnSegment =
		[EPSILON](
			float px,
			float pz,
			const Math::Vector3& a,
			const Math::Vector3& b)
		{
			const float cross =
				(b.x - a.x) * (pz - a.z)
				-
				(b.z - a.z) * (px - a.x);

			if (std::abs(cross) > EPSILON)
			{
				return false;
			}

			return
				px >= std::min(a.x, b.x) - EPSILON &&
				px <= std::max(a.x, b.x) + EPSILON &&
				pz >= std::min(a.z, b.z) - EPSILON &&
				pz <= std::max(a.z, b.z) + EPSILON;
		};

	//--------------------------------------------------------
	// Polygonの辺上にあるか
	//--------------------------------------------------------

	for (size_t i = 0;
		i < polygon.size();
		++i)
	{
		const auto& a =
			polygon[i];

		const auto& b =
			polygon[(i + 1) % polygon.size()];

		if (IsPointOnSegment(
			x,
			z,
			a,
			b))
		{
			return true;
		}
	}

	//--------------------------------------------------------
	// Ray Casting
	//--------------------------------------------------------

	bool inside = false;

	for (size_t i = 0;
		i < polygon.size();
		++i)
	{
		const auto& a =
			polygon[i];

		const auto& b =
			polygon[
				(i + 1) % polygon.size()
			];

		const bool intersect =
			((a.z > z) != (b.z > z)) &&
			(x <
				(b.x - a.x) *
				(z - a.z) /
				(b.z - a.z + 0.0000001f)
				+ a.x);

		if (intersect)
		{
			inside = !inside;
		}
	}

	return inside;
}

bool NavMeshGenerator::IsPointInsideHole(
	float x,
	float z,
	const WalkableRegion& region) const
{
	for (const auto& hole :
		region.holes)
	{
		if (IsPointInsidePolygon(
			x,
			z,
			hole))
		{
			return true;
		}
	}

	return false;
}

//============================================================
// Edge Intersection
//============================================================

bool NavMeshGenerator::TriangleIntersectsHole(
	const Math::Vector3& p0,
	const Math::Vector3& p1,
	const Math::Vector3& p2,
	const std::vector<Math::Vector3>& hole) const
{
	if (hole.size() < 3)
	{
		return false;
	}

	//--------------------------------------------------------
	// Triangle Edge
	//--------------------------------------------------------

	const Math::Vector3 triangleEdges[3][2] =
	{
		{ p0, p1 },
		{ p1, p2 },
		{ p2, p0 }
	};

	//--------------------------------------------------------
	// Hole Edge
	//--------------------------------------------------------

	for (size_t i = 0;
		i < hole.size();
		++i)
	{
		const Math::Vector3& h0 =
			hole[i];

		const Math::Vector3& h1 =
			hole[
				(i + 1) % hole.size()
			];

		for (const auto& edge :
			triangleEdges)
		{
			if (SegmentsIntersect(
				edge[0].x,
				edge[0].z,
				edge[1].x,
				edge[1].z,
				h0.x,
				h0.z,
				h1.x,
				h1.z))
			{
				return true;
			}
		}
	}

	//--------------------------------------------------------
	// TriangleがHoleを完全に含むケース
	//--------------------------------------------------------

	const Math::Vector3 trianglePoints[3] =
	{
		p0,
		p1,
		p2
	};

	for (const auto& point :
		hole)
	{
		if (IsPointInsidePolygon(
			point.x,
			point.z,
			{
				p0,
				p1,
				p2
			}))
		{
			return true;
		}
	}

	return false;
}

bool NavMeshGenerator::SegmentsIntersect(
	float ax,
	float az,
	float bx,
	float bz,
	float cx,
	float cz,
	float dx,
	float dz) const
{
	constexpr float EPSILON =
		0.000001f;

	auto Cross =
		[](float ax,
			float az,
			float bx,
			float bz,
			float cx,
			float cz)
		{
			return
				(bx - ax) *
				(cz - az)
				-
				(bz - az) *
				(cx - ax);
		};

	auto OnSegment =
		[EPSILON](
			float ax,
			float az,
			float bx,
			float bz,
			float px,
			float pz)
		{
			return
				px >= std::min(ax, bx) - EPSILON &&
				px <= std::max(ax, bx) + EPSILON &&
				pz >= std::min(az, bz) - EPSILON &&
				pz <= std::max(az, bz) + EPSILON;
		};

	const float c1 =
		Cross(
			ax, az,
			bx, bz,
			cx, cz);

	const float c2 =
		Cross(
			ax, az,
			bx, bz,
			dx, dz);

	const float c3 =
		Cross(
			cx, cz,
			dx, dz,
			ax, az);

	const float c4 =
		Cross(
			cx, cz,
			dx, dz,
			bx, bz);

	//--------------------------------------------------------
	// 通常の交差
	//--------------------------------------------------------

	if (((c1 > EPSILON && c2 < -EPSILON) ||
		(c1 < -EPSILON && c2 > EPSILON)) &&
		((c3 > EPSILON && c4 < -EPSILON) ||
			(c3 < -EPSILON && c4 > EPSILON)))
	{
		return true;
	}

	//--------------------------------------------------------
	// Collinear / 接触
	//--------------------------------------------------------

	if (std::abs(c1) <= EPSILON &&
		OnSegment(
			ax, az,
			bx, bz,
			cx, cz))
	{
		return true;
	}

	if (std::abs(c2) <= EPSILON &&
		OnSegment(
			ax, az,
			bx, bz,
			dx, dz))
	{
		return true;
	}

	if (std::abs(c3) <= EPSILON &&
		OnSegment(
			cx, cz,
			dx, dz,
			ax, az))
	{
		return true;
	}

	if (std::abs(c4) <= EPSILON &&
		OnSegment(
			cx, cz,
			dx, dz,
			bx, bz))
	{
		return true;
	}

	return false;
}

//============================================================
// Delaunay内部
//============================================================

bool NavMeshGenerator::IsPointInsideCircumcircle(
	const Math::Vector3& point,
	const Math::Vector3& a,
	const Math::Vector3& b,
	const Math::Vector3& c) const
{
	const double ax =
		static_cast<double>(a.x) -
		static_cast<double>(point.x);

	const double az =
		static_cast<double>(a.z) -
		static_cast<double>(point.z);

	const double bx =
		static_cast<double>(b.x) -
		static_cast<double>(point.x);

	const double bz =
		static_cast<double>(b.z) -
		static_cast<double>(point.z);

	const double cx =
		static_cast<double>(c.x) -
		static_cast<double>(point.x);

	const double cz =
		static_cast<double>(c.z) -
		static_cast<double>(point.z);

	const double determinant =
		(ax * ax + az * az) *
		(bx * cz - bz * cx)
		-
		(bx * bx + bz * bz) *
		(ax * cz - az * cx)
		+
		(cx * cx + cz * cz) *
		(ax * bz - az * bx);

	//--------------------------------------------------------
	// Triangleの向き
	//--------------------------------------------------------

	const double orientation =
		(static_cast<double>(b.x) -
			static_cast<double>(a.x))
		*
		(static_cast<double>(c.z) -
			static_cast<double>(a.z))
		-
		(static_cast<double>(b.z) -
			static_cast<double>(a.z))
		*
		(static_cast<double>(c.x) -
			static_cast<double>(a.x));

	const double epsilon =
		0.000000001;

	if (orientation > 0.0)
	{
		return determinant > epsilon;
	}

	return determinant < -epsilon;
}
