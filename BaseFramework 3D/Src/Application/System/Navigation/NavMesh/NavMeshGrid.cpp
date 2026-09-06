#include "NavMeshGrid.h"

//============================================================
// Clear
//============================================================

void NavMeshGrid::Clear()
{
	m_cells.clear();

	m_minX = 0;
	m_maxX = 0;

	m_minZ = 0;
	m_maxZ = 0;

	m_hasBounds = false;
}


//============================================================
// Build From Grounds
//============================================================
//
// Ground 1個 = 1 Tile
//
// Groundの中心座標をGrid座標へ変換する。
//
// 例:
//
// position = (0, 0, 0)
//     ↓
// Grid = (0, 0)
//
// position = (4, 0, 0)
//     ↓
// Grid = (1, 0)
//
// position = (8, 2, 0)
//     ↓
// Grid = (2, 0)
//
// heightはGroundのY座標をそのまま使用する。
//
//============================================================

void NavMeshGrid::BuildFromGrounds(
	const std::vector<NavMeshGroundTile>& groundTiles)
{
	//--------------------------------------------------------
	// 初期化
	//--------------------------------------------------------

	Clear();

	//--------------------------------------------------------
	// Groundを1つずつ処理
	//--------------------------------------------------------

	for (const auto& ground : groundTiles)
	{
		const Math::Vector3& position =
			ground.position;

		//----------------------------------------------------
		// World → Grid
		//----------------------------------------------------

		const int gridX =
			WorldToGrid(position.x);

		const int gridZ =
			WorldToGrid(position.z);

		//----------------------------------------------------
		// Cell
		//----------------------------------------------------

		NavMeshGridCell cell;

		cell.exists = true;

		cell.x = gridX;
		cell.z = gridZ;

		cell.height =
			position.y;

		//----------------------------------------------------
		// Cell全体を最初のWalkable Polygonにする
		//----------------------------------------------------
		//
		// Tile:
		//
		//      minZ             maxZ
		//        +----------------+
		//        |                |
		//        |                |
		//        |                |
		//        +----------------+
		//      minX             maxX
		//
		//----------------------------------------------------

		const float centerX =
			GridToWorld(gridX);

		const float centerZ =
			GridToWorld(gridZ);

		const float halfSize =
			GRID_SIZE * 0.5f;

		const float minX =
			centerX - halfSize;

		const float maxX =
			centerX + halfSize;

		const float minZ =
			centerZ - halfSize;

		const float maxZ =
			centerZ + halfSize;

		std::vector<Math::Vector3> polygon =
		{
			Math::Vector3(
				minX,
				cell.height,
				minZ),

			Math::Vector3(
				maxX,
				cell.height,
				minZ),

			Math::Vector3(
				maxX,
				cell.height,
				maxZ),

			Math::Vector3(
				minX,
				cell.height,
				maxZ)
		};

		cell.polygons.push_back(
			std::move(polygon));

		//----------------------------------------------------
		// 登録
		//----------------------------------------------------

		m_cells[MakeKey(
			gridX,
			gridZ)] = std::move(cell);

		//----------------------------------------------------
		// Bounds
		//----------------------------------------------------

		UpdateBounds(
			gridX,
			gridZ);
	}
}


//============================================================
// Apply Obstacles
//============================================================
//
// ObstacleによってCell内のWalkable PolygonをClipする。
//
// 重要:
//   Cell自体は削除しない。
//
// 例えば:
//
// +----------------+
// |                |
// |      XXXX      |
// |      XXXX      |
// |                |
// +----------------+
//
// のような場合、
// Cellのpolygonsは複数のPolygonに分割される。
//
//============================================================

void NavMeshGrid::ApplyObstacles(
	const std::vector<NavMeshObstacleBounds>& obstacles,
	float agentRadius)
{
	for (auto& pair : m_cells)
	{
		NavMeshGridCell& cell =
			pair.second;

		if (!cell.exists)
		{
			continue;
		}

		if (cell.polygons.empty())
		{
			continue;
		}

		for (const auto& obstacle : obstacles)
		{
			//------------------------------------------------
			// Cell AABBとObstacle AABBが交差しないなら
			// Clip不要
			//------------------------------------------------

			if (!IsCellOverlappingObstacle(
				cell,
				obstacle,
				agentRadius))
			{
				continue;
			}

			//------------------------------------------------
			// 現在のPolygon群
			//------------------------------------------------

			std::vector<
				std::vector<Math::Vector3>
			> currentPolygons =
				std::move(cell.polygons);

			cell.polygons.clear();

			//------------------------------------------------
			// 全PolygonをClip
			//------------------------------------------------

			for (const auto& polygon :
				currentPolygons)
			{
				std::vector<
					std::vector<Math::Vector3>
				> clippedPolygons;

				ClipPolygonByObstacle(
					polygon,
					obstacle,
					agentRadius,
					clippedPolygons);

				for (auto& clipped :
					clippedPolygons)
				{
					AddPolygonIfValid(
						cell.polygons,
						std::move(clipped));
				}
			}

			//------------------------------------------------
			// 完全に塞がれた
			//------------------------------------------------

			if (cell.polygons.empty())
			{
				break;
			}
		}
	}
}


//============================================================
// Get Cell const
//============================================================

const NavMeshGridCell*
NavMeshGrid::GetCell(
	int x,
	int z) const
{
	const auto it =
		m_cells.find(
			MakeKey(x, z));

	if (it == m_cells.end())
	{
		return nullptr;
	}

	return &it->second;
}


//============================================================
// Get Cell
//============================================================

NavMeshGridCell*
NavMeshGrid::GetCell(
	int x,
	int z)
{
	const auto it =
		m_cells.find(
			MakeKey(x, z));

	if (it == m_cells.end())
	{
		return nullptr;
	}

	return &it->second;
}


//============================================================
// Exists
//============================================================

bool NavMeshGrid::Exists(
	int x,
	int z) const
{
	const NavMeshGridCell* cell =
		GetCell(x, z);

	return
		cell != nullptr &&
		cell->exists;
}


//============================================================
// Is Walkable
//============================================================
//
// Cell内にWalkable Polygonが残っていれば歩行可能。
//
//============================================================

bool NavMeshGrid::IsWalkable(
	int x,
	int z) const
{
	const NavMeshGridCell* cell =
		GetCell(x, z);

	if (!cell)
	{
		return false;
	}

	if (!cell->exists)
	{
		return false;
	}

	return !cell->polygons.empty();
}


//============================================================
// Can Connect
//============================================================
//
// 2つのCellがNavMesh上で接続可能か判定する。
//
// ・Cell存在
// ・Walkable Polygon存在
// ・隣接Cell
// ・高さ差
//
//============================================================

bool NavMeshGrid::CanConnect(
	int x1,
	int z1,
	int x2,
	int z2) const
{
	//--------------------------------------------------------
	// Cell取得
	//--------------------------------------------------------

	const NavMeshGridCell* cellA =
		GetCell(x1, z1);

	const NavMeshGridCell* cellB =
		GetCell(x2, z2);

	if (!cellA ||
		!cellB)
	{
		return false;
	}

	//--------------------------------------------------------
	// 存在チェック
	//--------------------------------------------------------

	if (!cellA->exists ||
		!cellB->exists)
	{
		return false;
	}

	//--------------------------------------------------------
	// Walkableチェック
	//--------------------------------------------------------

	if (cellA->polygons.empty() ||
		cellB->polygons.empty())
	{
		return false;
	}

	//--------------------------------------------------------
	// 隣接チェック
	//--------------------------------------------------------
	//
	// 4方向だけを接続対象とする。
	//
	//--------------------------------------------------------

	const int dx =
		std::abs(x2 - x1);

	const int dz =
		std::abs(z2 - z1);

	if (dx + dz != 1)
	{
		return false;
	}

	//--------------------------------------------------------
	// 高さ差
	//--------------------------------------------------------

	const float heightDifference =
		std::abs(
			cellA->height -
			cellB->height);

	//--------------------------------------------------------
	// Step Height
	//--------------------------------------------------------

	if (heightDifference >
		MAX_STEP_HEIGHT)
	{
		return false;
	}

	return true;
}


//============================================================
// Get Cell Center
//============================================================

Math::Vector3
NavMeshGrid::GetCellCenter(
	int x,
	int z) const
{
	const NavMeshGridCell* cell =
		GetCell(x, z);

	if (!cell)
	{
		return Math::Vector3::Zero;
	}

	return Math::Vector3(
		GridToWorld(x),
		cell->height,
		GridToWorld(z));
}


//============================================================
// World To Grid
//============================================================

int NavMeshGrid::WorldToGrid(
	float value)
{
	return static_cast<int>(
		std::round(
			value / GRID_SIZE));
}


//============================================================
// Grid To World
//============================================================

float NavMeshGrid::GridToWorld(
	int value)
{
	return
		static_cast<float>(value) *
		GRID_SIZE;
}


//============================================================
// Make Key
//============================================================

long long NavMeshGrid::MakeKey(
	int x,
	int z)
{
	const std::uint32_t ux =
		static_cast<std::uint32_t>(x);

	const std::uint32_t uz =
		static_cast<std::uint32_t>(z);

	return
		(static_cast<long long>(ux) << 32) |
		static_cast<long long>(uz);
}


//============================================================
// Update Bounds
//============================================================

void NavMeshGrid::UpdateBounds(
	int x,
	int z)
{
	if (!m_hasBounds)
	{
		m_minX = x;
		m_maxX = x;

		m_minZ = z;
		m_maxZ = z;

		m_hasBounds = true;

		return;
	}

	m_minX =
		std::min(
			m_minX,
			x);

	m_maxX =
		std::max(
			m_maxX,
			x);

	m_minZ =
		std::min(
			m_minZ,
			z);

	m_maxZ =
		std::max(
			m_maxZ,
			z);
}


//============================================================
// Is Cell Overlapping Obstacle
//============================================================
//
// CellのXZ AABBとObstacleのXZ AABBが重なっているか。
//
//============================================================

bool NavMeshGrid::IsCellOverlappingObstacle(
	const NavMeshGridCell& cell,
	const NavMeshObstacleBounds& obstacle,
	float agentRadius)
{
	//--------------------------------------------------------
	// Tile中心
	//--------------------------------------------------------

	const float centerX =
		GridToWorld(cell.x);

	const float centerZ =
		GridToWorld(cell.z);

	//--------------------------------------------------------
	// Tile Half Size
	//--------------------------------------------------------

	const float halfSize =
		GRID_SIZE * 0.5f;

	//--------------------------------------------------------
	// Cell Bounds
	//--------------------------------------------------------

	const float cellMinX =
		centerX - halfSize;

	const float cellMaxX =
		centerX + halfSize;

	const float cellMinZ =
		centerZ - halfSize;

	const float cellMaxZ =
		centerZ + halfSize;

	//--------------------------------------------------------
	// Obstacle Bounds
	//--------------------------------------------------------

	const float obstacleMinX =
		obstacle.minX -
		agentRadius;

	const float obstacleMaxX =
		obstacle.maxX +
		agentRadius;

	const float obstacleMinZ =
		obstacle.minZ -
		agentRadius;

	const float obstacleMaxZ =
		obstacle.maxZ +
		agentRadius;

	//--------------------------------------------------------
	// AABB Overlap
	//--------------------------------------------------------

	const bool overlapX =
		cellMaxX > obstacleMinX &&
		cellMinX < obstacleMaxX;

	const bool overlapZ =
		cellMaxZ > obstacleMinZ &&
		cellMinZ < obstacleMaxZ;

	return
		overlapX &&
		overlapZ;
}


//============================================================
// Clip Polygon By Obstacle
//============================================================
//
// PolygonとObstacleの差分を作る。
//
// 今回のNavMesh PolygonはXZ平面上の矩形として
// 管理されているため、矩形同士の差分として処理する。
//
// 最大4個のRectangleが生成される。
//
//============================================================

void NavMeshGrid::ClipPolygonByObstacle(
	const std::vector<Math::Vector3>& polygon,
	const NavMeshObstacleBounds& obstacle,
	float agentRadius,
	std::vector<std::vector<Math::Vector3>>& result)
{
	result.clear();

	if (polygon.size() < 3)
	{
		return;
	}

	//--------------------------------------------------------
	// Polygon Bounds
	//--------------------------------------------------------

	float polyMinX = polygon[0].x;
	float polyMaxX = polygon[0].x;

	float polyMinZ = polygon[0].z;
	float polyMaxZ = polygon[0].z;

	const float y = polygon[0].y;

	for (const auto& vertex : polygon)
	{
		polyMinX =
			std::min(polyMinX, vertex.x);

		polyMaxX =
			std::max(polyMaxX, vertex.x);

		polyMinZ =
			std::min(polyMinZ, vertex.z);

		polyMaxZ =
			std::max(polyMaxZ, vertex.z);
	}

	//--------------------------------------------------------
	// Obstacle Bounds
	//--------------------------------------------------------

	const float obstacleMinX =
		obstacle.minX - agentRadius;

	const float obstacleMaxX =
		obstacle.maxX + agentRadius;

	const float obstacleMinZ =
		obstacle.minZ - agentRadius;

	const float obstacleMaxZ =
		obstacle.maxZ + agentRadius;

	//--------------------------------------------------------
	// Intersection
	//--------------------------------------------------------

	const float overlapMinX =
		std::max(
			polyMinX,
			obstacleMinX);

	const float overlapMaxX =
		std::min(
			polyMaxX,
			obstacleMaxX);

	const float overlapMinZ =
		std::max(
			polyMinZ,
			obstacleMinZ);

	const float overlapMaxZ =
		std::min(
			polyMaxZ,
			obstacleMaxZ);

	//--------------------------------------------------------
	// 完全に交差していない
	//--------------------------------------------------------

	if (overlapMinX >= overlapMaxX ||
		overlapMinZ >= overlapMaxZ)
	{
		result.push_back(polygon);
		return;
	}

	//--------------------------------------------------------
	// ObstacleがPolygon全体を覆っている
	//--------------------------------------------------------

	if (overlapMinX <= polyMinX &&
		overlapMaxX >= polyMaxX &&
		overlapMinZ <= polyMinZ &&
		overlapMaxZ >= polyMaxZ)
	{
		// Walkable Polygonなし
		return;
	}

	//--------------------------------------------------------
	// Left
	//
	// Polygonの左側
	//
	// +----------------------+
	// |                      |
	// |       obstacle       |
	// |                      |
	// +----------------------+
	// ^
	// |
	// left
	//
	//--------------------------------------------------------

	if (polyMinX < overlapMinX)
	{
		std::vector<Math::Vector3> left =
		{
			{
				polyMinX,
				y,
				polyMinZ
			},

			{
				overlapMinX,
				y,
				polyMinZ
			},

			{
				overlapMinX,
				y,
				polyMaxZ
			},

			{
				polyMinX,
				y,
				polyMaxZ
			}
		};

		AddPolygonIfValid(
			result,
			std::move(left));
	}

	//--------------------------------------------------------
	// Right
	//
	//--------------------------------------------------------

	if (overlapMaxX < polyMaxX)
	{
		std::vector<Math::Vector3> right =
		{
			{
				overlapMaxX,
				y,
				polyMinZ
			},

			{
				polyMaxX,
				y,
				polyMinZ
			},

			{
				polyMaxX,
				y,
				polyMaxZ
			},

			{
				overlapMaxX,
				y,
				polyMaxZ
			}
		};

		AddPolygonIfValid(
			result,
			std::move(right));
	}

	//--------------------------------------------------------
	// Bottom
	//
	// Obstacleの下側。
	//
	// X方向はObstacleとの重複範囲だけ。
	//
	//--------------------------------------------------------

	if (polyMinZ < overlapMinZ)
	{
		std::vector<Math::Vector3> bottom =
		{
			{
				overlapMinX,
				y,
				polyMinZ
			},

			{
				overlapMaxX,
				y,
				polyMinZ
			},

			{
				overlapMaxX,
				y,
				overlapMinZ
			},

			{
				overlapMinX,
				y,
				overlapMinZ
			}
		};

		AddPolygonIfValid(
			result,
			std::move(bottom));
	}

	//--------------------------------------------------------
	// Top
	//
	// Obstacleの上側。
	//
	//--------------------------------------------------------

	if (overlapMaxZ < polyMaxZ)
	{
		std::vector<Math::Vector3> top =
		{
			{
				overlapMinX,
				y,
				overlapMaxZ
			},

			{
				overlapMaxX,
				y,
				overlapMaxZ
			},

			{
				overlapMaxX,
				y,
				polyMaxZ
			},

			{
				overlapMinX,
				y,
				polyMaxZ
			}
		};

		AddPolygonIfValid(
			result,
			std::move(top));
	}
}


//============================================================
// Split Polygon By Line
//============================================================
//
// PolygonをAxis-Aligned Lineで
//
//   inside
//   outside
//
// の2つに分割する。
//
// Sutherland-Hodgman型の処理。
//
//============================================================

void NavMeshGrid::SplitPolygonByLine(
	const std::vector<Math::Vector3>& polygon,
	int axis,
	float value,
	bool keepGreater,
	std::vector<Math::Vector3>& inside,
	std::vector<Math::Vector3>& outside)
{
	if (polygon.size() < 3)
	{
		return;
	}

	constexpr float EPSILON =
		0.000001f;

	//--------------------------------------------------------
	// Inside判定
	//--------------------------------------------------------

	auto IsInside =
		[axis, value, keepGreater](
			const Math::Vector3& p)
		{
			const float coordinate =
				(axis == 0)
				? p.x
				: p.z;

			if (keepGreater)
			{
				return coordinate >= value;
			}

			return coordinate <= value;
		};

	//--------------------------------------------------------
	// 交点計算
	//--------------------------------------------------------

	auto CalculateIntersection =
		[axis, value, EPSILON](
			const Math::Vector3& a,
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

			if (std::abs(denominator) <=
				EPSILON)
			{
				return a;
			}

			const float t =
				(value - aValue) /
				denominator;

			return
				a +
				(b - a) * t;
		};

	//--------------------------------------------------------
	// Edgeを1本ずつ処理
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


//============================================================
// Add Polygon If Valid
//============================================================

void NavMeshGrid::AddPolygonIfValid(
	std::vector<std::vector<Math::Vector3>>& polygons,
	std::vector<Math::Vector3> polygon)
{
	if (polygon.size() < 3)
	{
		return;
	}

	constexpr float EPSILON =
		0.0001f;

	constexpr float EPSILON_SQ =
		EPSILON * EPSILON;

	//--------------------------------------------------------
	// 連続する重複頂点を削除
	//--------------------------------------------------------

	std::vector<Math::Vector3> cleaned;

	for (const auto& vertex :
		polygon)
	{
		if (!cleaned.empty())
		{
			const Math::Vector3 diff =
				vertex -
				cleaned.back();

			if (diff.LengthSquared() <=
				EPSILON_SQ)
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

		if (diff.LengthSquared() <=
			EPSILON_SQ)
		{
			cleaned.pop_back();
		}
	}

	//--------------------------------------------------------
	// 頂点数
	//--------------------------------------------------------

	if (cleaned.size() < 3)
	{
		return;
	}

	//--------------------------------------------------------
	// 面積チェック
	//--------------------------------------------------------

	float area =
		0.0f;

	for (size_t i = 0;
		i < cleaned.size();
		++i)
	{
		const auto& a =
			cleaned[i];

		const auto& b =
			cleaned[
				(i + 1) % cleaned.size()
			];

		area +=
			a.x * b.z -
			b.x * a.z;
	}

	area =
		std::abs(area) * 0.5f;

	if (area <=
		EPSILON)
	{
		return;
	}

	//--------------------------------------------------------
	// Polygon追加
	//--------------------------------------------------------

	polygons.push_back(
		std::move(cleaned));
}
