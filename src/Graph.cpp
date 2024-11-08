#include <Graphexia/Graph.hpp>

namespace gpx {
    // FIXME: Think about this... deleting a vertex takes too much effort.

    void Graph::EraseEdge(usize id) {
        this->edges.erase(this->edges.begin() + id);

        for (usize i = 0; i < this->edgesForVertex.size(); ++i) {
            std::vector<usize>& vEdges = this->edgesForVertex[i];

            usize j = 0;
            while(j < vEdges.size() && vEdges[j] < id) {
                ++j;
            } 

            if(j < vEdges.size() && vEdges[j] == id) {
                vEdges.erase(vEdges.begin() + j);
            }

            while(j < vEdges.size()) {
                --vEdges[j++]; 
            }
        }
    }

    void Graph::EraseVertex(usize id) {
        std::vector<usize>& vEdges = this->edgesForVertex[id];
        // As edges are added incrementally, their id's will always be sorted
        // std::sort(edges.begin(), edges.end());

        for (usize i = vEdges.size(); i > 0; --i) {
            usize index = i - 1; 

            EraseEdge(vEdges[index]);
        }
        
        this->edgesForVertex.erase(this->edgesForVertex.begin() + id);
        if(id == --this->vertices) {
            return;
        }

        for (usize i = 0; i < this->edges.size(); ++i) {
            Edge& edge = this->edges[i]; 

            if(edge.fromId > id) {
                --edge.fromId;
            }

            if(edge.toId > id) {
                --edge.toId;
            }
        }
    }
} // namespace gpx
