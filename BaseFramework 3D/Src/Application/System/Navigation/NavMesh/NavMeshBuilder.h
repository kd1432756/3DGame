#pragma once

class NavMeshManager;

class NavMeshBuilder
{
public:
	struct Params
	{
		float tileSize = 4.0f;
		float agentRadius = 0.2f;
	};

	using ObjectList = std::list<std::shared_ptr<KdGameObject>>;

	static int Build(const ObjectList& objects,
		NavMeshManager& manager,
		const Params& params);

private:
	static void BuildConnections(NavMeshManager& manager);
};