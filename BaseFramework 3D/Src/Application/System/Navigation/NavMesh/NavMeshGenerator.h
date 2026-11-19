#pragma once

#include "NavMeshPolygonBuilder.h"
#include "PolygonBoolean.h"

class NavMeshGenerator
{
public:
	
	struct Input
	{
		// タイルの集合
		std::vector<NavMeshPolygonBuilder::Tile> tiles;
		// 障害物の集合
		std::vector<NavMeshPolygonBuilder::Obstacle> obstacles;

		// タイルの大きさ
		float tileSize = 1.0f;
		// 移動エージェントの半径
		float agentRadius = 0.2f;
	};

	// Nav Meshポリゴンを生成
	std::vector<PolygonBoolean::Polygon> Generate(const Input& input) const;
};
