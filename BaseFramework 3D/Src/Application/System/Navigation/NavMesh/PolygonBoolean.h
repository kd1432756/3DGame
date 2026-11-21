#pragma once

#include "NavMeshPolygonBuilder.h"

class PolygonBoolean
{
public:

	using Polygon = NavMeshPolygonBuilder::Polygon;

	//============================================================
	// Union
	//============================================================

	// 2つのポリゴンを合体
	static std::vector<Polygon> Union(const Polygon& subject, const Polygon& clip);

	// 複数ポリゴンをまとめて合体
	static std::vector<Polygon> Union(const std::vector<Polygon>& polygons);

	//============================================================
	// Difference
	//============================================================

	// ポリゴンからポリゴンを切り抜く
	static std::vector<Polygon> Difference(const Polygon& subject, const Polygon& clip);

	// 複数ポリゴンから複数ポリゴンを切り抜く
	static std::vector<Polygon> Difference(const std::vector<Polygon>& subjects, const std::vector<Polygon>& clips);
};