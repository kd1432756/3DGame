#pragma once

#include "NavMeshManager.h"

#include "NavMeshPolygonBuilder.h"
#include "PolygonBoolean.h"
//============================================================
// NavMesh Generator
//============================================================
//
// NavMeshを生成するGenerator
//
// ・COLメッシュを入力として使用
// ・COLメッシュの三角形をNavMeshPolygonとして生成
// ・Polygon同士の共有辺からConnectionを構築
//
//============================================================

class NavMeshGenerator
{
public:

	//============================================================
	struct Input
	{
		std::vector<NavMeshPolygonBuilder::Tile> tiles;
		std::vector<NavMeshPolygonBuilder::Obstacle> obstacles;

		float tileSize = 1.0f;
		float agentRadius = 0.2f;
	};

	std::vector<PolygonBoolean::Polygon> Generate(
		const Input& input) const;
	//============================================================

	struct NavMeshObstacleBounds
	{
		float minX = 0.0f;
		float maxX = 0.0f;

		float minZ = 0.0f;
		float maxZ = 0.0f;
	};

	//============================================================
	// Generate
	//============================================================

	void Generate(
		NavMeshManager& navMeshManager,
		const std::vector<std::pair<
		std::shared_ptr<KdModelData>,
		Math::Matrix>>&meshes,
		const std::vector<NavMeshObstacleBounds>& obstacles);

	// =====

	//============================================================
	// Stage Tile
	//============================================================

	struct NavMeshTile
	{
		Math::Vector3 v0;
		Math::Vector3 v1;
		Math::Vector3 v2;
		Math::Vector3 v3;
	};

	//============================================================
	// Tile 方式の Generate
	//============================================================

	void Generate(
		NavMeshManager& navMeshManager,
		const std::vector<NavMeshTile>& tiles,
		const std::vector<NavMeshObstacleBounds>& obstacles);

	// =====

	//============================================================
	// Walkable Region
	//============================================================

	struct WalkableRegion
	{
		// Groundの外周
		std::vector<
			std::vector<Math::Vector3>>
			outerBoundaries;

		// 障害物による穴
		std::vector<
			std::vector<Math::Vector3>>
			holes;
	};


	//============================================================
	// Delaunay Triangle
	//============================================================

	struct DelaunayTriangle
	{
		int a = -1;
		int b = -1;
		int c = -1;
	};

	//============================================================
	// Generate SoloMesh
	//============================================================

	void GenerateSoloMesh(
		const std::vector<NavMeshTile>& tiles,
		const std::vector<NavMeshObstacleBounds>& obstacles,
		NavMeshManager& navMeshManager);

	//============================================================
	// Agent Radius
	//============================================================

	void SetAgentRadius(float radius)
	{
		m_agentRadius = radius;
	}

	float GetAgentRadius() const
	{
		return m_agentRadius;
	}

private:

	//============================================================
	// Stage Tile
	//============================================================

	void GenerateTilePolygons(
		NavMeshManager& navMeshManager,
		const NavMeshTile& tile,
		const std::vector<NavMeshObstacleBounds>& obstacles);

	// =====

	//============================================================
	// Polygon
	//============================================================

	void GeneratePolygons(
		NavMeshManager& navMeshManager,
		const std::shared_ptr<KdModelData>& modelData,
		const Math::Matrix& worldMatrix,
		const std::vector<NavMeshObstacleBounds>& obstacles);

	//============================================================
	// Walkable
	//============================================================

	bool IsWalkable(
		const Math::Vector3& p0,
		const Math::Vector3& p1,
		const Math::Vector3& p2) const;

	//============================================================
	// Obstacle Clip
	//============================================================

	void ClipTriangleByObstacles(
		const Math::Vector3& p0,
		const Math::Vector3& p1,
		const Math::Vector3& p2,
		const std::vector<NavMeshObstacleBounds>& obstacles,
		std::vector<std::vector<Math::Vector3>>& result) const;

	void ClipPolygonByObstacle(
		const std::vector<Math::Vector3>& polygon,
		const NavMeshObstacleBounds& obstacle,
		std::vector<std::vector<Math::Vector3>>& result) const;

	void SplitPolygonByLine(
		const std::vector<Math::Vector3>& polygon,
		int axis,
		float value,
		bool keepGreater,
		std::vector<Math::Vector3>& inside,
		std::vector<Math::Vector3>& outside) const;

	void AddPolygonIfValid(
		std::vector<std::vector<Math::Vector3>>& polygons,
		std::vector<Math::Vector3> polygon) const;

	void TriangulatePolygon(
		const std::vector<Math::Vector3>& polygon,
		std::vector<std::vector<Math::Vector3>>& triangles) const;

	//============================================================
	// Connection
	//============================================================

	void BuildConnections(
		NavMeshManager& navMeshManager);

	// =====

	 //============================================================
	// Walkable Region
	//============================================================

	WalkableRegion BuildWalkableRegion(
		const std::vector<NavMeshTile>& tiles,
		const std::vector<NavMeshObstacleBounds>& obstacles) const;

	//============================================================
	// Delaunay
	//============================================================

	void DelaunayTriangulate(
		const WalkableRegion& region,
		std::vector<Math::Vector3>& vertices,
		std::vector<DelaunayTriangle>& triangles) const;

	//============================================================
	// Triangle判定
	//============================================================

	bool IsTriangleInsideWalkableRegion(
		const Math::Vector3& p0,
		const Math::Vector3& p1,
		const Math::Vector3& p2,
		const WalkableRegion& region) const;

	//============================================================
	// Point判定
	//============================================================

	bool IsPointInsidePolygon(
		float x,
		float z,
		const std::vector<Math::Vector3>& polygon) const;

	bool IsPointInsideHole(
		float x,
		float z,
		const WalkableRegion& region) const;

	//============================================================
	// Edge Intersection
	//============================================================

	bool TriangleIntersectsHole(
		const Math::Vector3& p0,
		const Math::Vector3& p1,
		const Math::Vector3& p2,
		const std::vector<Math::Vector3>& hole) const;

	bool SegmentsIntersect(
		float ax,
		float az,
		float bx,
		float bz,
		float cx,
		float cz,
		float dx,
		float dz) const;

	//============================================================
	// Delaunay内部
	//============================================================

	struct DelaunayEdge
	{
		int a = -1;
		int b = -1;

		bool operator==(const DelaunayEdge& rhs) const
		{
			return
				(a == rhs.a && b == rhs.b) ||
				(a == rhs.b && b == rhs.a);
		}
	};

	bool IsPointInsideCircumcircle(
		const Math::Vector3& point,
		const Math::Vector3& a,
		const Math::Vector3& b,
		const Math::Vector3& c) const;

private:

	//============================================================
	// Agent
	//============================================================

	float m_agentRadius = 0.2f;
};
