#pragma once

class NavMeshPolygonBuilder
{
public:


	struct Tile
	{
		Math::Vector2 centerPos;
	};

	struct Obstacle
	{
		float minX;
		float maxX;
		float minZ;
		float maxZ;
	};

	struct VertexId
	{
		int x;
		int y;

		bool operator==(const VertexId& other) const
		{
			return x == other.x &&
				y == other.y;
		}
	};

	struct Edge
	{
		VertexId a;
		VertexId b;
	};

	struct Polygon
	{
		std::vector<Math::Vector2> outer;
		std::vector<std::vector<Math::Vector2>> holes;
	};

public:
	std::vector<Polygon> BuildTiles(
		const std::vector<Tile>& tiles,
		float tileSize);

	std::vector<Polygon> BuildObstacles(
		const std::vector<Obstacle>& obstacles,
		float agentRadius);
private:
	VertexId GetTileOrigin(
		const Tile& tile,
		float tileSize) const;

	std::array<VertexId, 4> GetTileVertices(
		const Tile& tile,
		float tileSize) const;

	Edge MakeEdge(
		VertexId a,
		VertexId b) const;
};