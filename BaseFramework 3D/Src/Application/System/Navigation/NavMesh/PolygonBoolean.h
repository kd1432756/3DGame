#pragma once

#include "NavMeshPolygonBuilder.h"

class PolygonBoolean
{
public:

	using Polygon =
		NavMeshPolygonBuilder::Polygon;

	//============================================================
	// Union
	//
	// Result = subject ∪ clip
	//
	// Disconnected Region は std::vector<Polygon> の
	// 別要素として返す。
	//============================================================

	static std::vector<Polygon> Union(
		const Polygon& subject,
		const Polygon& clip);

	static std::vector<Polygon> Union(
		const std::vector<Polygon>& polygons);

	//============================================================
	// Difference
	//
	// Result = subject - clip
	//
	// Output:
	//   Outer = CCW
	//   Hole  = CW
	//
	// Disconnected Region は std::vector<Polygon> の
	// 別要素として返す。
	//============================================================

	static std::vector<Polygon> Difference(
		const Polygon& subject,
		const Polygon& clip);

	static std::vector<Polygon> Difference(
		const std::vector<Polygon>& subjects,
		const std::vector<Polygon>& clips);
};
