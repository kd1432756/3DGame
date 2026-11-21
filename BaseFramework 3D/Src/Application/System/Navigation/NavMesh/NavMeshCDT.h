#pragma once

#include "NavMeshPolygonBuilder.h"

class NavMeshCDT
{
public:

	struct Triangle
	{
		Math::Vector2 a;
		Math::Vector2 b;
		Math::Vector2 c;

		std::vector<Math::Vector3> ToVector3() const
		{
			return
			{
				Math::Vector3(a.x, 0.5f, a.y),
				Math::Vector3(b.x, 0.5f, b.y),
				Math::Vector3(c.x, 0.5f, c.y)
			};
		}
	};

	std::vector<Triangle> Triangulate(const std::vector<NavMeshPolygonBuilder::Polygon>& polygons) const;
};
