#pragma once

#include "NavMeshPolygon.h"

class NavMeshManager
{
public:

	//============================================================
	// Polygon
	//============================================================

	// Polygonを作成
	std::shared_ptr<NavMeshPolygon>
		CreatePolygon(
			const std::vector<Math::Vector3>& vertices);

	// IDを指定して作成
	std::shared_ptr<NavMeshPolygon>
		CreatePolygon(
			int id,
			const std::vector<Math::Vector3>& vertices);

	// Polygon削除
	void DeletePolygon(int id);

	// 全削除
	void Clear();

	//============================================================
	// Vertex
	//============================================================

	// Polygonの頂点を変更
	void SetPolygonVertex(
		int polygonID,
		size_t vertexIndex,
		const Math::Vector3& position);

	//============================================================
	// Find
	//============================================================

	std::shared_ptr<NavMeshPolygon>
		FindPolygon(int id);

	std::shared_ptr<const NavMeshPolygon>
		FindPolygon(int id) const;

	//============================================================
	// 全Polygon
	//============================================================

	const std::vector<
		std::shared_ptr<NavMeshPolygon>
	>& GetPolygons() const
	{
		return m_polygons;
	}

	//============================================================
	// Connection
	//============================================================

	// Polygon同士を接続
	void ConnectPolygons(
		int fromID,
		int toID);

	// Polygon同士の接続を解除
	void DisconnectPolygons(
		int fromID,
		int toID);

	//============================================================
	// Point
	//============================================================

	std::shared_ptr<NavMeshPolygon>
		FindPolygonFromPoint(
			const Math::Vector3& point);

	std::shared_ptr<const NavMeshPolygon>
		FindPolygonFromPoint(
			const Math::Vector3& point) const;

	//============================================================
	// FindNearestPoint
	//============================================================

	Math::Vector3 FindNearestPoint(
		const Math::Vector3& point,
		float maxSearchRadius = -1.0f) const;

	//============================================================
	// ID
	//============================================================

	int GetNextID() const
	{
		return m_nextID;
	}

	//============================================================
	// Load State
	//============================================================

	bool IsLoaded() const
	{
		return m_isLoaded;
	}

	void SetLoaded(bool loaded)
	{
		m_isLoaded = loaded;
	}


private:
	NavMeshManager() = default;

	std::vector<
		std::shared_ptr<NavMeshPolygon>
	> m_polygons;

	int m_nextID = 0;

	bool m_isLoaded = false;

public:

	// シングルトンパターン
	static NavMeshManager& Instance()
	{
		static NavMeshManager instance;
		return instance;
	}
};
