#pragma once

#include "NavMeshGroundTile.h"
#include "NavMeshObstacleBounds.h"

//============================================================
// NavMesh Grid Cell
//============================================================
//
// 1 Cell = 1 Ground Tile
//
// 1 Tile = 4m × 4m
//
// Cell内の歩行可能領域をPolygonとして保持する。
//
// 通常:
//     polygons.size() == 1
//
// Obstacleによって分割された場合:
//     polygons.size() >= 2
//
// Obstacleに完全に覆われた場合:
//     polygons.empty()
//
//============================================================

struct NavMeshGridCell
{
	//============================================================
	// State
	//============================================================

	// このGrid位置にGroundが存在するか
	bool exists = false;

	//============================================================
	// Grid Coordinate
	//============================================================

	int x = 0;
	int z = 0;

	//============================================================
	// Height
	//============================================================

	// Tileの地面の高さ
	float height = 0.0f;

	//============================================================
	// Walkable Polygon
	//============================================================

	// Cell内に残っている歩行可能領域
	//
	// すべてXZ平面上のPolygon。
	// YにはTileのheightを使用する。
	//
	std::vector<
		std::vector<Math::Vector3>
	> polygons;
};


//============================================================
// NavMesh Grid
//============================================================
//
// Ground Tileを4m単位のGridとして管理する。
//
// ・Ground → Grid
// ・Cell → 4m × 4m Polygon
// ・Obstacle → Polygon Clip
// ・高さ差 → Connection判定
//
//============================================================

class NavMeshGrid
{
public:

	//============================================================
	// Constant
	//============================================================

	// 1 Tileのサイズ
	static constexpr float GRID_SIZE = 4.0f;

	//============================================================
	// Height
	//============================================================

	// 隣接Tileとの最大高低差
	static constexpr float MAX_STEP_HEIGHT = 1.0f;

	//============================================================
	// Clear
	//============================================================

	void Clear();

	//============================================================
	// Build
	//============================================================

	// Ground TileからGridを構築
	void BuildFromGrounds(
		const std::vector<NavMeshGroundTile>& groundTiles);

	//============================================================
	// Obstacle
	//============================================================

	// ObstacleによってCell内のPolygonをClipする
	//
	// Cellそのものを削除するのではなく、
	// Cell内の歩行可能領域だけを削る。
	//
	void ApplyObstacles(
		const std::vector<NavMeshObstacleBounds>& obstacles,
		float agentRadius = 0.0f);

	//============================================================
	// Cell
	//============================================================

	const NavMeshGridCell* GetCell(
		int x,
		int z) const;

	NavMeshGridCell* GetCell(
		int x,
		int z);

	//============================================================
	// State
	//============================================================

	// Cell内に歩行可能Polygonが存在するか
	bool IsWalkable(
		int x,
		int z) const;

	bool Exists(
		int x,
		int z) const;

	//============================================================
	// Connection
	//============================================================

	// 2つのCellがNavMesh上で接続可能か
	bool CanConnect(
		int x1,
		int z1,
		int x2,
		int z2) const;

	//============================================================
	// World Position
	//============================================================

	// Grid座標 → Tile中心World座標
	Math::Vector3 GetCellCenter(
		int x,
		int z) const;

	//============================================================
	// Grid Coordinate
	//============================================================

	static int WorldToGrid(
		float value);

	static float GridToWorld(
		int value);

	//============================================================
	// Bounds
	//============================================================

	bool HasBounds() const
	{
		return m_hasBounds;
	}

	int GetMinX() const
	{
		return m_minX;
	}

	int GetMaxX() const
	{
		return m_maxX;
	}

	int GetMinZ() const
	{
		return m_minZ;
	}

	int GetMaxZ() const
	{
		return m_maxZ;
	}

	//============================================================
	// Cell Container
	//============================================================

	const std::unordered_map<
		long long,
		NavMeshGridCell
	>& GetCells() const
	{
		return m_cells;
	}

private:

	//============================================================
	// Cell
	//============================================================

	std::unordered_map<
		long long,
		NavMeshGridCell
	> m_cells;

	//============================================================
	// Bounds
	//============================================================

	int m_minX = 0;
	int m_maxX = 0;

	int m_minZ = 0;
	int m_maxZ = 0;

	bool m_hasBounds = false;

	//============================================================
	// Key
	//============================================================

	static long long MakeKey(
		int x,
		int z);

	//============================================================
	// Bounds
	//============================================================

	void UpdateBounds(
		int x,
		int z);

	//============================================================
	// Obstacle
	//============================================================

	static bool IsCellOverlappingObstacle(
		const NavMeshGridCell& cell,
		const NavMeshObstacleBounds& obstacle,
		float agentRadius);

	//============================================================
	// Polygon Clip
	//============================================================

	// PolygonからObstacle内部を削除する。
	//
	// 結果:
	//   result = Obstacle外側のPolygon群
	//
	static void ClipPolygonByObstacle(
		const std::vector<Math::Vector3>& polygon,
		const NavMeshObstacleBounds& obstacle,
		float agentRadius,
		std::vector<std::vector<Math::Vector3>>& result);

	//============================================================
	// Polygon Split
	//============================================================

	// Polygonを1本のAxis-Aligned Lineで
	// inside / outside に分割する。
	//
	// axis:
	//   0 = X
	//   1 = Z
	//
	// keepGreater:
	//   true  = coordinate >= value
	//   false = coordinate <= value
	//
	static void SplitPolygonByLine(
		const std::vector<Math::Vector3>& polygon,
		int axis,
		float value,
		bool keepGreater,
		std::vector<Math::Vector3>& inside,
		std::vector<Math::Vector3>& outside);

	//============================================================
	// Polygon Validation
	//============================================================

	static void AddPolygonIfValid(
		std::vector<std::vector<Math::Vector3>>& polygons,
		std::vector<Math::Vector3> polygon);
};
