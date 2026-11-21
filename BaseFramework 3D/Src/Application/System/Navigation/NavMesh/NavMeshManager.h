#pragma once

#include "NavMeshPolygon.h"

class NavMeshManager
{
public:

	// ポリゴンを作成
	std::shared_ptr<NavMeshPolygon> CreatePolygon(const std::vector<Math::Vector3>& vertices);

	// IDを指定して作成
	std::shared_ptr<NavMeshPolygon> CreatePolygon(int id, const std::vector<Math::Vector3>& vertices);

	// ポリゴン削除
	void DeletePolygon(int id);

	// 全削除
	void Clear();

	// ポリゴンの頂点を変更
	void SetPolygonVertex(int polygonID, size_t vertexIndex, const Math::Vector3& position);

	// ポリゴンの検索
	std::shared_ptr<NavMeshPolygon> FindPolygon(int id);

	// ポリゴンの検索読み取り用
	std::shared_ptr<const NavMeshPolygon> FindPolygon(int id) const;

	// 全ポリゴン取得
	const std::vector<std::shared_ptr<NavMeshPolygon>>& GetPolygons() const
	{
		return m_polygons;
	}

	// ポリゴン同士を接続
	void ConnectPolygons(int fromID, int toID);

	// ポリゴン同士の接続を解除
	void DisconnectPolygons(int fromID, int toID);

	// 指定座標からポリゴンを検索
	std::shared_ptr<NavMeshPolygon> FindPolygonFromPoint(const Math::Vector3& point);

	// 指定座標からポリゴンを検索読み取り用
	std::shared_ptr<const NavMeshPolygon> FindPolygonFromPoint(const Math::Vector3& point) const;
	
	// 最も近いNavMesh上の座標を検索
	Math::Vector3 FindNearestPoint(const Math::Vector3& point, float maxSearchRadius = -1.0f) const;

	// ID
	int GetNextID() const
	{
		return m_nextID;
	}

	// Load
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

	std::vector<std::shared_ptr<NavMeshPolygon>> m_polygons;

	int m_nextID = 0;

	// ロードフラグ
	bool m_isLoaded = false;

public:

	// シングルトンパターン
	static NavMeshManager& Instance()
	{
		static NavMeshManager instance;
		return instance;
	}
};