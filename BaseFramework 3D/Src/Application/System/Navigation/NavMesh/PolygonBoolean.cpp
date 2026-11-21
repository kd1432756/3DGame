#include "PolygonBoolean.h"

using namespace Clipper2Lib;

namespace
{
	// 小数第四桁までを整数にする
	constexpr double kScale = 10000.0;

	// 3点が一直線上か
	bool IsCollinear(const Math::Vector2& a, const Math::Vector2& b, const Math::Vector2& c)
	{
		// ベクトルを計算
		const double abX = static_cast<double>(b.x) - static_cast<double>(a.x);
		const double abY = static_cast<double>(b.y) - static_cast<double>(a.y);

		const double bcX = static_cast<double>(c.x) - static_cast<double>(b.x);
		const double bcY = static_cast<double>(c.y) - static_cast<double>(b.y);

		// 外積を計算
		const double cross = abX * bcY - abY * bcX;

		constexpr double epsilon = 1e-8;

		// ほぼ0なら同一直線上
		return std::abs(cross) < epsilon;
	}

	// 同一直線上の中間頂点を削除
	std::vector<Math::Vector2> RemoveCollinearPoints(const std::vector<Math::Vector2>& polygon)
	{
		if (polygon.size() <= 3)
		{
			return polygon;
		}

		std::vector<Math::Vector2> result = polygon;

		bool removed = true;

		while (removed && result.size() > 3)
		{
			removed = false;

			for (std::size_t i = 0; i < result.size(); ++i)
			{
				// 前後の頂点番号を取得
				const std::size_t prev = (i + result.size() - 1) % result.size();

				const std::size_t next = (i + 1) % result.size();

				// 前後と現在の頂点を取得
				const Math::Vector2& a = result[prev];

				const Math::Vector2& b = result[i];

				const Math::Vector2& c = result[next];

				// 一直線上なら削除
				if (IsCollinear(a, b, c))
				{
					result.erase(result.begin() + i);

					removed = true;

					break;
				}
			}
		}

		return result;
	}

	// Vector2 -> Point64
	Point64 ToClipperPoint(const Math::Vector2& v)
	{
		// 整数座標に変換
		return Point64(
			static_cast<int64_t>(std::llround(static_cast<double>(v.x) * kScale)),
			static_cast<int64_t>(std::llround(static_cast<double>(v.y) * kScale)));
	}

	// Point64 -> Vector2
	Math::Vector2 FromClipperPoint(const Point64& p)
	{
		// 小数座標に変換
		return Math::Vector2(
			static_cast<float>(static_cast<double>(p.x) / kScale),
			static_cast<float>(static_cast<double>(p.y) / kScale));
	}

	// Vector2 polygon -> Path64
	Path64 ToClipperPath(const std::vector<Math::Vector2>& polygon)
	{
		Path64 result;

		if (polygon.empty()) return result;

		result.reserve(polygon.size());

		// すべての頂点を整数座標に変換
		for (const auto& v : polygon)
		{
			result.emplace_back(ToClipperPoint(v));
		}

		return result;
	}

	// Path64 -> Vector2 polygon
	std::vector<Math::Vector2> FromClipperPath(const Path64& path)
	{
		std::vector<Math::Vector2> result;

		if (path.empty()) return result;

		result.reserve(path.size());

		// すべての頂点を小数座標に変換
		for (const auto& p : path)
		{
			result.emplace_back(FromClipperPoint(p));
		}

		// 同一直線上の中間頂点を削除
		return RemoveCollinearPoints(result);
	}

	// Polygon -> Paths64
	Paths64 ToClipperPaths(const PolygonBoolean::Polygon& polygon)
	{
		Paths64 result;

		// Outer
		if (!polygon.outer.empty())
		{
			Path64 outer = ToClipperPath(polygon.outer);

			// 回転方向を確認
			if (!IsPositive(outer))
			{
				// 回転方向を統一
				std::reverse(outer.begin(), outer.end());
			}

			result.emplace_back(std::move(outer));
		}

		// Holes
		for (const auto& hole : polygon.holes)
		{
			if (hole.empty()) continue;

			Path64 path = ToClipperPath(hole);

			// 回転方向を確認
			if (IsPositive(path))
			{
				// 回転方向を統一
				std::reverse(path.begin(), path.end());
			}

			result.emplace_back(std::move(path));
		}

		return result;
	}

	// PolyTree -> Polygon
	void ConvertPolyTreeNode(const PolyPath64& node, std::vector<PolygonBoolean::Polygon>& result)
	{
		// Holeは親で処理する
		if (node.IsHole()) return;

		PolygonBoolean::Polygon polygon;

		// Outer
		polygon.outer = FromClipperPath(node.Polygon());

		// 子ノードからHoleを取得
		for (size_t i = 0; i < node.Count(); ++i)
		{
			const PolyPath64* child = node.Child(i);

			if (child == nullptr) continue;

			if (!child->IsHole()) continue;

			std::vector<Math::Vector2> hole = FromClipperPath(child->Polygon());

			if (!hole.empty())
			{
				polygon.holes.emplace_back(std::move(hole));
			}
		}

		// Polygon追加
		if (!polygon.outer.empty())
		{
			result.emplace_back(std::move(polygon));
		}

		// Nested Outer
		for (size_t i = 0; i < node.Count(); ++i)
		{
			const PolyPath64* child = node.Child(i);

			if (child == nullptr) continue;

			if (!child->IsHole()) continue;

			// Holeの子ノードからOuterを探す
			for (size_t j = 0; j < child->Count(); ++j)
			{
				const PolyPath64* nested = child->Child(j);

				if (nested == nullptr) continue;

				if (!nested->IsHole())
				{
					// 自分自身を呼び出す
					ConvertPolyTreeNode(*nested, result);
				}
			}
		}
	}

	// PolyTree64 -> Polygon[]
	std::vector<PolygonBoolean::Polygon> ConvertPolyTree(const PolyTree64& tree)
	{
		std::vector<PolygonBoolean::Polygon> result;

		for (size_t i = 0; i < tree.Count(); ++i)
		{
			const PolyPath64* child = tree.Child(i);

			if (child == nullptr) continue;

			// 各OuterとHoleをポリゴンに変換
			ConvertPolyTreeNode(*child, result);
		}

		return result;
	}

	// Union
	std::vector<PolygonBoolean::Polygon> ExecuteUnion(const Paths64& subjects)
	{
		if (subjects.empty()) return {};

		Clipper64 clipper;

		// すべてのポリゴンを登録
		clipper.AddSubject(subjects);

		PolyTree64 tree;

		// Union実行
		clipper.Execute(ClipType::Union, FillRule::NonZero, tree);

		return ConvertPolyTree(tree);
	}

	// Difference
	std::vector<PolygonBoolean::Polygon> ExecuteDifference(const Paths64& subject, const Paths64& clip)
	{
		if (subject.empty()) return {};

		Clipper64 clipper;

		// すべての切り抜かれるポリゴンを登録
		clipper.AddSubject(subject);

		if (!clip.empty())
		{
			// すべての切り抜くポリゴンを登録
			clipper.AddClip(clip);
		}

		PolyTree64 tree;

		// Difference実行
		clipper.Execute(ClipType::Difference, FillRule::NonZero, tree);

		return ConvertPolyTree(tree);
	}
}

//============================================================
// Union
//============================================================

std::vector<PolygonBoolean::Polygon> PolygonBoolean::Union(const Polygon& subject, const Polygon& clip)
{
	Paths64 subjects = ToClipperPaths(subject);

	Paths64 clips = ToClipperPaths(clip);

	Clipper64 clipper;

	if (!subjects.empty())
	{
		clipper.AddSubject(subjects);
	}

	if (!clips.empty())
	{
		clipper.AddClip(clips);
	}

	PolyTree64 tree;

	// union実行
	clipper.Execute(ClipType::Union, FillRule::NonZero, tree);

	return ConvertPolyTree(tree);
}

std::vector<PolygonBoolean::Polygon> PolygonBoolean::Union(const std::vector<Polygon>& polygons)
{
	if (polygons.empty()) return {};

	Paths64 subjects;

	// すべてのポリゴンをループ
	for (const Polygon& polygon : polygons)
	{
		Paths64 paths = ToClipperPaths(polygon);

		for (auto& path : paths)
		{
			subjects.emplace_back(std::move(path));
		}
	}

	return ExecuteUnion(subjects);
}

//============================================================
// Difference
//============================================================

std::vector<PolygonBoolean::Polygon> PolygonBoolean::Difference(const Polygon& subject, const Polygon& clip)
{
	Paths64 subjectPaths = ToClipperPaths(subject);

	Paths64 clipPaths = ToClipperPaths(clip);

	// difference実行
	return ExecuteDifference(subjectPaths, clipPaths);
}

std::vector<PolygonBoolean::Polygon> PolygonBoolean::Difference(const std::vector<Polygon>& subjects, const std::vector<Polygon>& clips)
{
	if (subjects.empty())
	{
		return {};
	}

	Paths64 subjectPaths;

	// すべてのsubjectのポリゴンをループ
	for (const Polygon& polygon : subjects)
	{
		Paths64 paths = ToClipperPaths(polygon);

		for (auto& path : paths)
		{
			subjectPaths.emplace_back(std::move(path));
		}
	}

	Paths64 clipPaths;

	// すべてのclipのポリゴンをループ
	for (const Polygon& polygon : clips)
	{
		Paths64 paths = ToClipperPaths(polygon);

		for (auto& path : paths)
		{
			clipPaths.emplace_back(std::move(path));
		}
	}

	return ExecuteDifference(subjectPaths, clipPaths);
}