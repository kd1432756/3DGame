#include "NavMeshSerializer.h"

#include "json.hpp"

using json = nlohmann::json;

//============================================================
// Vector3
//============================================================

static json Vector3ToJson(
	const Math::Vector3& v)
{
	return
	{
		{ "x", v.x },
		{ "y", v.y },
		{ "z", v.z }
	};
}

static bool JsonToVector3(
	const json& j,
	Math::Vector3& out)
{
	if (!j.is_object() ||
		!j.contains("x") ||
		!j.contains("y") ||
		!j.contains("z"))
	{
		return false;
	}

	try
	{
		out.x = j["x"].get<float>();
		out.y = j["y"].get<float>();
		out.z = j["z"].get<float>();
	}
	catch (...)
	{
		return false;
	}

	return true;
}

//============================================================
// Save
//============================================================

bool NavMeshSerializer::Save(
	const std::string& path,
	const NavMeshManager& manager)
{
	json root;

	root["polygons"] = json::array();

	//--------------------------------------------------------
	// Polygon
	//--------------------------------------------------------

	for (const auto& polygon :
		manager.GetPolygons())
	{
		if (!polygon)
		{
			continue;
		}

		json polygonJson;

		//----------------------------------------------------
		// ID
		//----------------------------------------------------

		polygonJson["id"] =
			polygon->GetID();

		//----------------------------------------------------
		// Vertices
		//----------------------------------------------------

		polygonJson["vertices"] =
			json::array();

		for (const auto& vertex :
			polygon->GetVertices())
		{
			polygonJson["vertices"].push_back(
				Vector3ToJson(vertex));
		}

		//----------------------------------------------------
		// Connections
		//----------------------------------------------------

		polygonJson["connections"] =
			polygon->GetConnections();

		//----------------------------------------------------
		// Add
		//----------------------------------------------------

		root["polygons"].push_back(
			polygonJson);
	}

	//--------------------------------------------------------
	// File
	//--------------------------------------------------------

	std::ofstream ofs(path);

	if (!ofs.is_open())
	{
		return false;
	}

	ofs << root.dump(4);

	return true;
}

//============================================================
// Load
//============================================================

bool NavMeshSerializer::Load(
	const std::string& path,
	NavMeshManager& manager)
{
	// すでにロード済みなら何もしない
	if (manager.IsLoaded())
	{
		return true;
	}

	std::ifstream ifs(path);

	if (!ifs.is_open())
	{
		return false;
	}

	json root;

	try
	{
		ifs >> root;
	}
	catch (...)
	{
		return false;
	}

	//--------------------------------------------------------
	// Clear
	//--------------------------------------------------------

	manager.Clear();

	//--------------------------------------------------------
	// Polygon
	//--------------------------------------------------------

	if (!root.contains("polygons") ||
		!root["polygons"].is_array())
	{
		manager.SetLoaded(true);
		return true;
	}

	//--------------------------------------------------------
	// Polygon生成
	//--------------------------------------------------------
	//
	// まず全Polygonを作成する。
	// Connectionsはこの後で復元する。
	//
	//--------------------------------------------------------

	for (const auto& polygonJson :
		root["polygons"])
	{
		if (!polygonJson.contains("id") ||
			!polygonJson.contains("vertices"))
		{
			continue;
		}

		//----------------------------------------------------
		// ID
		//----------------------------------------------------

		int id = -1;

		try
		{
			id =
				polygonJson["id"].get<int>();
		}
		catch (...)
		{
			continue;
		}

		//----------------------------------------------------
		// Vertices
		//----------------------------------------------------

		const auto& verticesJson =
			polygonJson["vertices"];

		if (!verticesJson.is_array() ||
			verticesJson.empty())
		{
			continue;
		}

		std::vector<Math::Vector3> vertices;

		for (const auto& vertexJson :
			verticesJson)
		{
			Math::Vector3 vertex;

			if (!JsonToVector3(
				vertexJson,
				vertex))
			{
				vertices.clear();
				break;
			}

			vertices.push_back(vertex);
		}

		if (vertices.empty())
		{
			continue;
		}

		//----------------------------------------------------
		// Polygon生成
		//----------------------------------------------------

		manager.CreatePolygon(
			id,
			vertices);
	}

	//--------------------------------------------------------
	// Connections復元
	//--------------------------------------------------------
	//
	// 全Polygon生成後に接続する。
	//
	//--------------------------------------------------------

	for (const auto& polygonJson :
		root["polygons"])
	{
		if (!polygonJson.contains("id") ||
			!polygonJson.contains("connections"))
		{
			continue;
		}

		int fromID = -1;

		try
		{
			fromID =
				polygonJson["id"].get<int>();
		}
		catch (...)
		{
			continue;
		}

		const auto& connectionsJson =
			polygonJson["connections"];

		if (!connectionsJson.is_array())
		{
			continue;
		}

		for (const auto& connectionJson :
			connectionsJson)
		{
			int toID = -1;

			try
			{
				toID =
					connectionJson.get<int>();
			}
			catch (...)
			{
				continue;
			}

			manager.ConnectPolygons(
				fromID,
				toID);
		}
	}

	manager.SetLoaded(true);

	return true;
}

bool NavMeshSerializer::LoadWithScale(const std::string& path, NavMeshManager& manager, float scale)
{	
	// すでにロード済みなら何もしない
	if (manager.IsLoaded())
	{
		return true;
	}

	std::ifstream ifs(path);

	if (!ifs.is_open())
	{
		return false;
	}

	json root;

	try
	{
		ifs >> root;
	}
	catch (...)
	{
		return false;
	}

	//--------------------------------------------------------
	// Clear
	//--------------------------------------------------------

	manager.Clear();

	//--------------------------------------------------------
	// Polygon
	//--------------------------------------------------------

	if (!root.contains("polygons") ||
		!root["polygons"].is_array())
	{
		manager.SetLoaded(true);
		return true;
	}

	//--------------------------------------------------------
	// Polygon生成
	//--------------------------------------------------------
	//
	// まず全Polygonを作成する。
	// Connectionsはこの後で復元する。
	//
	//--------------------------------------------------------

	for (const auto& polygonJson :
		root["polygons"])
	{
		if (!polygonJson.contains("id") ||
			!polygonJson.contains("vertices"))
		{
			continue;
		}

		//----------------------------------------------------
		// ID
		//----------------------------------------------------

		int id = -1;

		try
		{
			id =
				polygonJson["id"].get<int>();
		}
		catch (...)
		{
			continue;
		}

		//----------------------------------------------------
		// Vertices
		//----------------------------------------------------

		const auto& verticesJson =
			polygonJson["vertices"];

		if (!verticesJson.is_array() ||
			verticesJson.empty())
		{
			continue;
		}

		std::vector<Math::Vector3> vertices;

		for (const auto& vertexJson :
			verticesJson)
		{
			Math::Vector3 vertex;

			if (!JsonToVector3(
				vertexJson,
				vertex))
			{
				vertices.clear();
				break;
			}

			//------------------------------------------------
			// Scale
			//------------------------------------------------

			vertex *= scale;

			vertices.push_back(vertex);
		}

		if (vertices.empty())
		{
			continue;
		}

		//----------------------------------------------------
		// Polygon生成
		//----------------------------------------------------

		manager.CreatePolygon(
			id,
			vertices);
	}

	//--------------------------------------------------------
	// Connections復元
	//--------------------------------------------------------
	//
	// 全Polygon生成後に接続する。
	//
	//--------------------------------------------------------

	for (const auto& polygonJson :
		root["polygons"])
	{
		if (!polygonJson.contains("id") ||
			!polygonJson.contains("connections"))
		{
			continue;
		}

		int fromID = -1;

		try
		{
			fromID =
				polygonJson["id"].get<int>();
		}
		catch (...)
		{
			continue;
		}

		const auto& connectionsJson =
			polygonJson["connections"];

		if (!connectionsJson.is_array())
		{
			continue;
		}

		for (const auto& connectionJson :
			connectionsJson)
		{
			int toID = -1;

			try
			{
				toID =
					connectionJson.get<int>();
			}
			catch (...)
			{
				continue;
			}

			manager.ConnectPolygons(
				fromID,
				toID);
		}
	}

	manager.SetLoaded(true);

	return true;
}
