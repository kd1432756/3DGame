#include "NavMeshCDT.h"

namespace
{
	struct CDTPoint
	{
		double x;
		double y;
	};


	struct CDTEdge
	{
		std::size_t a;
		std::size_t b;
	};
}


std::vector<NavMeshCDT::Triangle>
NavMeshCDT::Triangulate(
	const std::vector<NavMeshPolygonBuilder::Polygon>& polygons) const
{
	std::vector<Triangle> result;

	if (polygons.empty())
	{
		return result;
	}


	for (const auto& polygon :
		polygons)
	{
		if (polygon.outer.size() < 3)
		{
			continue;
		}


		std::vector<CDT::V2d<double>> vertices;

		std::vector<CDT::Edge> edges;


		//--------------------------------------------------------
		// Outer
		//--------------------------------------------------------

		const std::size_t outerStart =
			vertices.size();


		for (const auto& vertex :
			polygon.outer)
		{
			vertices.push_back(
				CDT::V2d<double>(
					static_cast<double>(vertex.x),
					static_cast<double>(vertex.y)));
		}


		const std::size_t outerCount =
			polygon.outer.size();


		for (std::size_t i = 0;
			i < outerCount;
			++i)
		{
			const std::size_t a =
				outerStart + i;

			const std::size_t b =
				outerStart +
				((i + 1) % outerCount);

			edges.emplace_back(
				a,
				b);
		}


		//--------------------------------------------------------
		// Holes
		//--------------------------------------------------------

		for (const auto& hole :
			polygon.holes)
		{
			if (hole.size() < 3)
			{
				continue;
			}


			const std::size_t holeStart =
				vertices.size();


			for (const auto& vertex :
				hole)
			{
				vertices.push_back(
					CDT::V2d<double>(
						static_cast<double>(vertex.x),
						static_cast<double>(vertex.y)));
			}


			const std::size_t holeCount =
				hole.size();


			for (std::size_t i = 0;
				i < holeCount;
				++i)
			{
				const std::size_t a =
					holeStart + i;

				const std::size_t b =
					holeStart +
					((i + 1) % holeCount);

				edges.emplace_back(
					a,
					b);
			}
		}


		//--------------------------------------------------------
		// CDT
		//--------------------------------------------------------

		CDT::Triangulation<double> cdt;


		cdt.insertVertices(
			vertices);


		cdt.insertEdges(
			edges);


		cdt.eraseOuterTrianglesAndHoles();


		//--------------------------------------------------------
		// Triangle → NavMesh Triangle
		//--------------------------------------------------------

		for (const auto& triangle :
			cdt.triangles)
		{
			const auto& indices =
				triangle.vertices;


			const auto& a =
				cdt.vertices[indices[0]];

			const auto& b =
				cdt.vertices[indices[1]];

			const auto& c =
				cdt.vertices[indices[2]];


			result.push_back(
				Triangle
				{
					Math::Vector2{
						static_cast<float>(a.x),
						static_cast<float>(a.y)
					},

					Math::Vector2{
						static_cast<float>(b.x),
						static_cast<float>(b.y)
					},

					Math::Vector2{
						static_cast<float>(c.x),
						static_cast<float>(c.y)
					}
				});
		}
	}


	return result;
}
