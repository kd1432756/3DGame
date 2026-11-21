#pragma once

#include "NavMeshManager.h"

class NavMeshPathfinder
{
public:

	// ポリゴン経路探索
	std::vector<int> FindPath(
		const NavMeshManager& navMeshManager,
		const Math::Vector3& start,
		const Math::Vector3& goal) const;

	// 移動する座標を取得
	std::vector<Math::Vector3> FindPathPoints(
		const NavMeshManager& navMeshManager,
		const Math::Vector3& start,
		const Math::Vector3& goal) const;

private:

	// 探索状態
	struct State
	{
		// 現在のポリゴン
		int polygonID = -1;

		// 来たポリゴン
		int entryPolygonID = -1;

		bool operator==(const State& other) const
		{
			return polygonID == other.polygonID && entryPolygonID == other.entryPolygonID;
		}
	};

	// 探索状態のハッシュ
	struct StateHash
	{
		size_t operator()(const State& state) const
		{
			const size_t h1 = std::hash<int>{}(state.polygonID);

			const size_t h2 = std::hash<int>{}(state.entryPolygonID);

			return h1 ^
				(
					h2 + static_cast<size_t>(0x9e3779b9) +
					(h1 << 6) + (h1 >> 2)
					);
		}
	};

	// AStarのノード
	struct Node
	{
		// 現在の探索状態
		State state;

		// 入ってきた位置
		Math::Vector3 entryPoint;

		// スタートから現在までの移動コスト
		float gCost = 0.0f;

		// 現在からゴールまでの推定コスト
		float hCost = 0.0f;

		// ひとつ前の探索状態
		State parentState;

		// 親を持っているか
		bool hasParent = false;

		float GetFCost() const
		{
			return gCost + hCost;
		}
	};

	// ポータル
	struct Portal
	{
		Math::Vector3 left;

		Math::Vector3 right;
	};

	// ポータル生成
	bool CreatePortal(
		const NavMeshPolygon& from,
		const NavMeshPolygon& to,
		Portal& outPortal) const;

	// ポータル上の最近点を取得
	Math::Vector3 GetClosestPointOnPortal(
		const Math::Vector3& point,
		const Portal& portal) const;

	// 推定コスト計算
	float CalculateHeuristic(
		const Math::Vector3& currentPoint,
		const Math::Vector3& goalPoint) const;

	// 移動距離計算
	float CalculateDistanceXZ(
		const Math::Vector3& a,
		const Math::Vector3& b) const;

	// ポリゴン経路復元
	std::vector<int> ReconstructPath(
		const std::unordered_map<
		State,
		Node,
		StateHash>& nodes,
		const State& goalState) const;
};
