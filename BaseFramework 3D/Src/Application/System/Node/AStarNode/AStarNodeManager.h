#pragma once

#include "AStarNode.h"

class AStarNodeManager
{
public:

	//============================================================
	// Node
	//============================================================

	// Nodeを作成
	std::shared_ptr<AStarNode> CreateNode(
		const Math::Vector3& pos);

	std::shared_ptr<AStarNode> CreateNode(
		int id,
		const Math::Vector3& pos);


	// Nodeを削除
	void DeleteNode(int id);

	// 全Node削除
	void Clear();

	// Nodeを検索
	std::shared_ptr<AStarNode> FindNode(int id);

	// const版
	std::shared_ptr<const AStarNode> FindNode(int id) const;

	// 全Node取得
	const std::vector<std::shared_ptr<AStarNode>>&
		GetNodes() const
	{
		return m_nodes;
	}

	//============================================================
	// Connection
	//============================================================

	// Node同士を接続
	void ConnectNodes(int fromID, int toID);

	// Node同士の接続を解除
	void DisconnectNodes(int fromID, int toID);

	//============================================================
	// ID
	//============================================================

	int GetNextID() const
	{
		return m_nextID;
	}

	void ResetID()
	{
		m_nextID = 0;
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
	AStarNodeManager() = default;

	std::vector<std::shared_ptr<AStarNode>> m_nodes;

	int m_nextID = 0;

	bool m_isLoaded = false;

public:

	// シングルトンパターン
	static AStarNodeManager& Instance()
	{
		static AStarNodeManager instance;
		return instance;
	}
};
