#pragma once

#include "../../../Scene/SceneManager.h"

class NavMeshManager;
class PolygonBoolean;

class NavMeshPolygonBuilder
{
public:

	// タイルの情報
	struct Tile
	{
		// タイルの中心座標
		Math::Vector2 centerPos;
	};

	// AABBで障害物を管理
	struct Obstacle
	{
		float minX;
		float maxX;
		float minZ;
		float maxZ;
	};

	// グリッド上の頂点の識別用
	struct VertexId
	{
		int x;
		int y;

		bool operator==(const VertexId& other) const
		{
			return x == other.x && y == other.y;
		}
	};

	// 辺
	struct Edge
	{
		VertexId a;
		VertexId b;
	};

	// ポリゴンの外周と穴
	struct Polygon
	{
		std::vector<Math::Vector2> outer;
		std::vector<std::vector<Math::Vector2>> holes;
	};

public:
	// タイルからポリゴンを生成
	std::vector<Polygon> BuildTiles(const std::vector<Tile>& tiles, float tileSize);

	// 障害物群からポリゴンの生成
	std::vector<Polygon> BuildObstacles(const std::vector<Obstacle>& obstacles, float agentRadius);

private:
	// タイルの基準座標を取得
	VertexId GetTileOrigin(const Tile& tile, float tileSize) const;

	// タイルの４頂点の座標を取得
	std::array<VertexId, 4> GetTileVertices(const Tile& tile, float tileSize) const;

	// 2頂点から辺を生成
	Edge MakeEdge(VertexId a, VertexId b) const;
};