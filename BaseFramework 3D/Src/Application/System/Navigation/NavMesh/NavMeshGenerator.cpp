#include "NavMeshGenerator.h"

std::vector<PolygonBoolean::Polygon>NavMeshGenerator::Generate(const Input& input) const
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

	const auto tilePolygons = builder.BuildTiles(input.tiles, input.tileSize);

	if (tilePolygons.empty())
	{
		return result;
	}

	//============================================================
	// 2. Tile Union
	//============================================================

	result = PolygonBoolean::Union(tilePolygons);

	if (result.empty())
	{
		return result;
	}

	//============================================================
	// 3. Obstacle → Polygon
	//============================================================

	const auto obstaclePolygons = builder.BuildObstacles(input.obstacles, input.agentRadius);

	if (obstaclePolygons.empty())
	{
		return result;
	}

	//============================================================
	// 4. Obstacle Union
	//============================================================

	const auto unionedObstacles = PolygonBoolean::Union(obstaclePolygons);

	if (unionedObstacles.empty())
	{
		return result;
	}

	//============================================================
	// 5. Walkable - All Obstacles
	//============================================================

	result = PolygonBoolean::Difference(result, unionedObstacles);

	return result;
}