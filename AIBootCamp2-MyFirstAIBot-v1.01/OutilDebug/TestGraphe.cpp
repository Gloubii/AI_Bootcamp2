#include "TestGraphe.h"
#include <assert.h>

using namespace std;

void CheckNeighbours(Graph& graph)
{
	bool result = true;
	
	auto hexes = graph.GetHexes();
	
	for (auto h : hexes) {
		auto edgesSortants = graph.getConnections(h);
		bool resultTemp = false;
		
		auto checkCoherent = [&resultTemp, &graph, &h](Edge e) {
			Hex h2 = e.getTo();
			auto edgesSortants2 = graph.getConnections(h2);
			for (Edge e2 : edgesSortants2) {
				resultTemp = resultTemp || (h == e2.getTo());
			}
		};
		
		for_each(edgesSortants.begin(), edgesSortants.end(), checkCoherent);

		result = result && resultTemp;
	}

	assert(result);
}
