#include "PolygonBoolean.h"



using namespace Clipper2Lib;

namespace
{
	//============================================================
	// World -> Clipper
	//
	// 1 world unit = 10000 integer units
	// 小数点以下4桁
	//============================================================

	constexpr double kScale = 10000.0;


	//============================================================
	// 3点が一直線上か
	//============================================================

	bool IsCollinear(
		const Math::Vector2& a,
		const Math::Vector2& b,
		const Math::Vector2& c)
	{
		const double abX =
			static_cast<double>(b.x) -
			static_cast<double>(a.x);

		const double abY =
			static_cast<double>(b.y) -
			static_cast<double>(a.y);

		const double bcX =
			static_cast<double>(c.x) -
			static_cast<double>(b.x);

		const double bcY =
			static_cast<double>(c.y) -
			static_cast<double>(b.y);

		const double cross =
			abX * bcY -
			abY * bcX;

		constexpr double epsilon = 1e-8;

		return std::abs(cross) < epsilon;
	}


	//============================================================
	// 同一直線上の中間頂点を削除
	//============================================================

	std::vector<Math::Vector2>
		RemoveCollinearPoints(
			const std::vector<Math::Vector2>& polygon)
	{
		if (polygon.size() <= 3)
		{
			return polygon;
		}

		std::vector<Math::Vector2> result =
			polygon;

		bool removed = true;

		while (removed &&
			result.size() > 3)
		{
			removed = false;

			for (std::size_t i = 0;
				i < result.size();
				++i)
			{
				const std::size_t prev =
					(i + result.size() - 1) %
					result.size();

				const std::size_t next =
					(i + 1) %
					result.size();

				const Math::Vector2& a =
					result[prev];

				const Math::Vector2& b =
					result[i];

				const Math::Vector2& c =
					result[next];

				if (IsCollinear(a, b, c))
				{
					result.erase(
						result.begin() + i);

					removed = true;

					break;
				}
			}
		}

		return result;
	}

	//============================================================
	// Vector2 -> Point64
	//============================================================

	Point64 ToClipperPoint(
		const Math::Vector2& v)
	{
		return Point64(
			static_cast<int64_t>(
				std::llround(static_cast<double>(v.x) * kScale)),
			static_cast<int64_t>(
				std::llround(static_cast<double>(v.y) * kScale)));
	}

	//============================================================
	// Point64 -> Vector2
	//============================================================

	Math::Vector2 FromClipperPoint(
		const Point64& p)
	{
		return Math::Vector2(
			static_cast<float>(
				static_cast<double>(p.x) / kScale),
			static_cast<float>(
				static_cast<double>(p.y) / kScale));
	}

	//============================================================
	// Vector2 polygon -> Path64
	//============================================================

	Path64 ToClipperPath(
		const std::vector<Math::Vector2>& polygon)
	{
		Path64 result;

		if (polygon.empty())
			return result;

		result.reserve(polygon.size());

		for (const auto& v : polygon)
		{
			result.emplace_back(
				ToClipperPoint(v));
		}

		return result;
	}

	//============================================================
	// Path64 -> Vector2 polygon
	//============================================================

	std::vector<Math::Vector2> FromClipperPath(
		const Path64& path)
	{
		std::vector<Math::Vector2> result;

		if (path.empty())
			return result;

		result.reserve(path.size());

		for (const auto& p : path)
		{
			result.emplace_back(
				FromClipperPoint(p));
		}

		//--------------------------------------------------------
		// 同一直線上の中間頂点を削除
		//--------------------------------------------------------

		return RemoveCollinearPoints(
			result);
	}


	//============================================================
	// Polygon -> Paths64
	//
	// outer = positive
	// hole  = negative
	//============================================================

	Paths64 ToClipperPaths(
		const PolygonBoolean::Polygon& polygon)
	{
		Paths64 result;

		//--------------------------------------------------------
		// Outer
		//--------------------------------------------------------

		if (!polygon.outer.empty())
		{
			Path64 outer =
				ToClipperPath(polygon.outer);

			if (!IsPositive(outer))
			{
				std::reverse(
					outer.begin(),
					outer.end());
			}

			result.emplace_back(
				std::move(outer));
		}

		//--------------------------------------------------------
		// Holes
		//--------------------------------------------------------

		for (const auto& hole : polygon.holes)
		{
			if (hole.empty())
				continue;

			Path64 path =
				ToClipperPath(hole);

			if (IsPositive(path))
			{
				std::reverse(
					path.begin(),
					path.end());
			}

			result.emplace_back(
				std::move(path));
		}

		return result;
	}

	//============================================================
	// PolyTree -> Polygon
	//============================================================

	void ConvertPolyTreeNode(
		const PolyPath64& node,
		std::vector<PolygonBoolean::Polygon>& result)
	{
		//--------------------------------------------------------
		// Hole は親で処理する
		//--------------------------------------------------------

		if (node.IsHole())
			return;

		PolygonBoolean::Polygon polygon;

		//--------------------------------------------------------
		// Outer
		//--------------------------------------------------------

		polygon.outer =
			FromClipperPath(node.Polygon());

		//--------------------------------------------------------
		// Direct children
		//
		// Outer
		//   └─ Hole
		//--------------------------------------------------------

		for (size_t i = 0;
			i < node.Count();
			++i)
		{
			const PolyPath64* child =
				node.Child(i);

			if (child == nullptr)
				continue;

			if (!child->IsHole())
				continue;

			std::vector<Math::Vector2> hole =
				FromClipperPath(child->Polygon());

			if (!hole.empty())
			{
				polygon.holes.emplace_back(
					std::move(hole));
			}
		}

		//--------------------------------------------------------
		// Polygon追加
		//--------------------------------------------------------

		if (!polygon.outer.empty())
		{
			result.emplace_back(
				std::move(polygon));
		}

		//--------------------------------------------------------
		// Nested Outer
		//
		// Outer
		//   └─ Hole
		//        └─ Outer
		//
		// Nested Outer は別 Polygon として返す。
		//--------------------------------------------------------

		for (size_t i = 0;
			i < node.Count();
			++i)
		{
			const PolyPath64* child =
				node.Child(i);

			if (child == nullptr)
				continue;

			if (!child->IsHole())
				continue;

			for (size_t j = 0;
				j < child->Count();
				++j)
			{
				const PolyPath64* nested =
					child->Child(j);

				if (nested == nullptr)
					continue;

				if (!nested->IsHole())
				{
					ConvertPolyTreeNode(
						*nested,
						result);
				}
			}
		}
	}

	//============================================================
	// PolyTree64 -> Polygon[]
	//============================================================

	std::vector<PolygonBoolean::Polygon>
		ConvertPolyTree(
			const PolyTree64& tree)
	{
		std::vector<PolygonBoolean::Polygon> result;

		for (size_t i = 0;
			i < tree.Count();
			++i)
		{
			const PolyPath64* child =
				tree.Child(i);

			if (child == nullptr)
				continue;

			ConvertPolyTreeNode(
				*child,
				result);
		}

		return result;
	}

	//============================================================
	// Union
	//============================================================

	std::vector<PolygonBoolean::Polygon>
		ExecuteUnion(
			const Paths64& subjects)
	{
		if (subjects.empty())
			return {};

		Clipper64 clipper;

		clipper.AddSubject(subjects);

		PolyTree64 tree;

		clipper.Execute(
			ClipType::Union,
			FillRule::NonZero,
			tree);

		return ConvertPolyTree(tree);
	}

	//============================================================
	// Difference
	//============================================================

	std::vector<PolygonBoolean::Polygon>
		ExecuteDifference(
			const Paths64& subject,
			const Paths64& clip)
	{
		if (subject.empty())
			return {};

		Clipper64 clipper;

		clipper.AddSubject(subject);

		if (!clip.empty())
		{
			clipper.AddClip(clip);
		}

		PolyTree64 tree;

		clipper.Execute(
			ClipType::Difference,
			FillRule::NonZero,
			tree);

		return ConvertPolyTree(tree);
	}
}

//============================================================
// Union
//
// subject ∪ clip
//============================================================

std::vector<PolygonBoolean::Polygon>
PolygonBoolean::Union(
	const Polygon& subject,
	const Polygon& clip)
{
	Paths64 subjects =
		ToClipperPaths(subject);

	Paths64 clips =
		ToClipperPaths(clip);

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

	clipper.Execute(
		ClipType::Union,
		FillRule::NonZero,
		tree);

	return ConvertPolyTree(tree);
}

//============================================================
// Union
//
// polygons[0] ∪ polygons[1] ∪ ...
//============================================================

std::vector<PolygonBoolean::Polygon>
PolygonBoolean::Union(
	const std::vector<Polygon>& polygons)
{
	if (polygons.empty())
		return {};

	Paths64 subjects;

	for (const Polygon& polygon : polygons)
	{
		Paths64 paths =
			ToClipperPaths(polygon);

		for (auto& path : paths)
		{
			subjects.emplace_back(
				std::move(path));
		}
	}

	return ExecuteUnion(subjects);
}

//============================================================
// Difference
//
// subject - clip
//============================================================

std::vector<PolygonBoolean::Polygon>
PolygonBoolean::Difference(
	const Polygon& subject,
	const Polygon& clip)
{
	Paths64 subjectPaths =
		ToClipperPaths(subject);

	Paths64 clipPaths =
		ToClipperPaths(clip);

	return ExecuteDifference(
		subjectPaths,
		clipPaths);
}

//============================================================
// Difference
//
// subjects - clips
//
// 複数のWalkable Polygonから
// 複数のObstacle Polygonを一括で削除する。
//============================================================

std::vector<PolygonBoolean::Polygon>
PolygonBoolean::Difference(
	const std::vector<Polygon>& subjects,
	const std::vector<Polygon>& clips)
{
	if (subjects.empty())
	{
		return {};
	}

	//============================================================
	// Subject
	//============================================================

	Paths64 subjectPaths;

	for (const Polygon& polygon :
		subjects)
	{
		Paths64 paths =
			ToClipperPaths(polygon);

		for (auto& path : paths)
		{
			subjectPaths.emplace_back(
				std::move(path));
		}
	}

	//============================================================
	// Clip
	//============================================================

	Paths64 clipPaths;

	for (const Polygon& polygon :
		clips)
	{
		Paths64 paths =
			ToClipperPaths(polygon);

		for (auto& path : paths)
		{
			clipPaths.emplace_back(
				std::move(path));
		}
	}

	//============================================================
	// Difference
	//============================================================

	return ExecuteDifference(
		subjectPaths,
		clipPaths);
}
