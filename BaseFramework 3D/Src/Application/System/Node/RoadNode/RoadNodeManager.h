#pragma once

class RoadNode;

class RoadNodeManager
{
public:

	// 初期化
	void Init();

	// ノード一覧取得
	const std::vector<std::shared_ptr<RoadNode>>&
		GetNodes() const
	{
		return m_nodes;
	}

private:

	// ノード生成
	void CreateNodes(float scale);

	// ノード同士の接続設定
	void CreateConnections();

	void ConnectNodes(int indexA, int indexB);

	// 道路ノード
	std::vector<std::shared_ptr<RoadNode>> m_nodes;

public:

	// シングルトンパターン
	static RoadNodeManager& Instance()
	{
		static RoadNodeManager instance;
		return instance;
	}
};