#include "NavMeshCDT.h"

std::vector<NavMeshCDT::Triangle> NavMeshCDT::Triangulate(const std::vector<NavMeshPolygonBuilder::Polygon>& polygons) const
{
	std::vector<Triangle> result;

	if (polygons.empty())
	{
		return result;
	}

	for (const auto& polygon : polygons)
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

		const std::size_t outerStart = vertices.size();

		// 外周の頂点を登録
		for (const auto& vertex : polygon.outer)
		{
			vertices.push_back(CDT::V2d<double>(static_cast<double>(vertex.x), static_cast<double>(vertex.y)));
		}

		const std::size_t outerCount = polygon.outer.size();

		// 外周の辺を登録
		for (std::size_t i = 0; i < outerCount; ++i)
		{
			const std::size_t a = outerStart + i;

			const std::size_t b = outerStart + ((i + 1) % outerCount);

			edges.emplace_back(a, b);
		}

		//--------------------------------------------------------
		// Holes
		//--------------------------------------------------------

		for (const auto& hole : polygon.holes)
		{
			if (hole.size() < 3)
			{
				continue;
			}

			const std::size_t holeStart = vertices.size();

			// Holeの頂点を登録
			for (const auto& vertex : hole)
			{
				vertices.push_back(CDT::V2d<double>(static_cast<double>(vertex.x), static_cast<double>(vertex.y)));
			}

			const std::size_t holeCount = hole.size();

			// Holeの辺を登録
			for (std::size_t i = 0; i < holeCount; ++i)
			{
				const std::size_t a = holeStart + i;

				const std::size_t b = holeStart + ((i + 1) % holeCount);

				edges.emplace_back(a, b);
			}
		}

		//--------------------------------------------------------
		// CDT
		//--------------------------------------------------------

		CDT::Triangulation<double> cdt;

		// 頂点を登録
		cdt.insertVertices(vertices);

		// 制約辺を登録
		cdt.insertEdges(edges);

		// 外側と穴の三角形を削除
		cdt.eraseOuterTrianglesAndHoles();

		//--------------------------------------------------------
		// Triangle → NavMesh Triangle
		//--------------------------------------------------------

		for (const auto& triangle : cdt.triangles)
		{
			// 三角形をNav Mesh用のTriangleに変換
			const auto& indices = triangle.vertices;

			const auto& a = cdt.vertices[indices[0]];

			const auto& b = cdt.vertices[indices[1]];

			const auto& c = cdt.vertices[indices[2]];

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