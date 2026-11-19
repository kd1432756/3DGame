#include "NavMeshBuilder.h"

#include "NavMeshGenerator.h"
#include "NavMeshCDT.h"
#include "NavMeshManager.h"

#include "../../../GameObject/Object/StaticMeshObject/StaticMeshObject.h"
#include "../../../GameObject/Object/StaticMeshObject/Ground/Ground.h"
#include "../../../GameObject/Object/StaticMeshObject/InteractiveObject/InteractiveObject.h"

namespace
{
	void AppendObstacles(StaticMeshObject& obj,
		std::vector<NavMeshPolygonBuilder::Obstacle>& out)
	{
		auto modelData = obj.GetModelData();

		if (!modelData) return;

		const Math::Matrix worldMatrix = obj.GetMatrix();

		const auto& nodes = modelData->GetOriginalNodes();

		// COL ノードごとに
		for (int nodeIndex : modelData->GetCollisionMeshNodeIndices())
		{
			if (nodeIndex < 0 || nodeIndex >= static_cast<int>(nodes.size()))
			{
				continue;
			}

			const auto& node = nodes[nodeIndex];

			if (!node.m_spMesh) continue;

			// Local AABB → World AABB
			DirectX::BoundingBox worldAABB;

			node.m_spMesh->GetBoundingBox().Transform(
				worldAABB,
				node.m_worldTransform * worldMatrix);

			// XZ Bounds
			NavMeshPolygonBuilder::Obstacle obstacle;

			obstacle.minX = worldAABB.Center.x - worldAABB.Extents.x;
			obstacle.maxX = worldAABB.Center.x + worldAABB.Extents.x;
			obstacle.minZ = worldAABB.Center.z - worldAABB.Extents.z;
			obstacle.maxZ = worldAABB.Center.z + worldAABB.Extents.z;

			out.push_back(obstacle);
		}
	}
}

//============================================================
// Build
//============================================================

int NavMeshBuilder::Build(	const ObjectList& objects,
	NavMeshManager& manager,
	const Params& params)
{
	//--------------------------------------------------------
	// 1. シーンから入力を集める
	//--------------------------------------------------------

	NavMeshGenerator::Input input;

	input.tileSize = params.tileSize;
	input.agentRadius = params.agentRadius;

	for (auto& obj : objects)
	{
		if (!obj) continue;

		// Walkable Tile
		if (auto ground = std::dynamic_pointer_cast<Ground>(obj))
		{
			NavMeshPolygonBuilder::Tile tile;

			tile.centerPos = Math::Vector2{ ground->GetPos().x, ground->GetPos().z };

			input.tiles.push_back(tile);
		}

		// Obstacle
		if (auto mesh = std::dynamic_pointer_cast<StaticMeshObject>(obj))
		{
			if (mesh->GetCategory() == KdGameObject::ObjectCategory::Prop)
			{
				AppendObstacles(*mesh, input.obstacles);
			}
		}

		// Obstacle
	/*	if (auto interactive = std::dynamic_pointer_cast<InteractiveObject>(obj))
		{
			if (interactive->IsCollisionEnabled())
			{
				AppendObstacles(*interactive, input.obstacles);
			}
		}*/
	}

	//--------------------------------------------------------
	// 2. Polygon Boolean → CDT
	//--------------------------------------------------------

	NavMeshGenerator generator;
	NavMeshCDT cdt;

	const auto triangles = cdt.Triangulate(generator.Generate(input));

	//--------------------------------------------------------
	// 3. Manager へ登録
	//--------------------------------------------------------

	manager.Clear();

	for (const auto& triangle : triangles)
	{
		manager.CreatePolygon(triangle.ToVector3());
	}

	//--------------------------------------------------------
	// 4. 接続
	//--------------------------------------------------------

	BuildConnections(manager);

	return static_cast<int>(triangles.size());
}

//============================================================
// Connection
//============================================================

void NavMeshBuilder::BuildConnections(NavMeshManager& manager)
{
	const auto& polygons = manager.GetPolygons();

	for (size_t i = 0; i < polygons.size(); ++i)
	{
		for (size_t j = i + 1; j < polygons.size(); ++j)
		{
			const auto& a = polygons[i];
			const auto& b = polygons[j];

			if (!a || !b) continue;

			if (!a->SharesEdgeWith(*b)) continue;

			manager.ConnectPolygons(a->GetID(), b->GetID());
		}
	}
}