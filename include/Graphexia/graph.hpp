#ifndef _GRAPHEXIA_GRAPH__HPP_
#define _GRAPHEXIA_GRAPH__HPP_

#include <Graphexia/core.hpp>
#include <limits>
#include <vector>

namespace gpx {
    struct Vertex final {
        usize id;
        i16 x, y;
        u32 size;

        bool collides(i16 x, i16 y) const {
            i16 dX = this->x - x;
            i16 dY = this->y - y;

            return static_cast<u32>(dX * dX) + static_cast<u32>(dY * dY) <= this->size * this->size;
        }
    };

    struct Edge final {
        usize fromId, toId;
        f32 weight; 
    };

    enum class IncidenceState : i8 {
        Leaves = -1,
        None = 0,
        Incident = 1
    };

    struct Graph final {
        static constexpr usize NoVertex = std::numeric_limits<usize>::max();

        bool IsDirected() const { return this->directed; }
        void SetDirected(bool directed) { this->directed = directed; }

        usize AddVertex(i16 x, i16 y, u32 size = 5) {
            usize vertexId = this->vertices.size();

            this->vertices.push_back(Vertex{vertexId, x, y, size});
            this->edgesForVertex.push_back(std::vector<usize>());
            return vertexId;
        }
        void AddEdge(usize from, usize to, f32 weight = 0) {
            usize edgeId = this->edges.size();

            this->edgesForVertex[from].push_back(edgeId);
            this->edgesForVertex[to].push_back(edgeId);

            this->edges.push_back(Edge{from, to, weight});
        }

        const std::vector<Vertex>& Vertices() const { return this->vertices; }
        const std::vector<Edge>& Edges() const { return this->edges; }

        usize DegreeOf(usize vertex) const { return this->edgesForVertex[vertex].size(); }
        usize FindVertex(i16 x, i16 y) const;

        std::vector<usize> GetAdjacencyMatrix() const;
        std::vector<IncidenceState> GetIncidenceMatrix() const;
    private:
        std::vector<Vertex> vertices;
        std::vector<Edge> edges;
        std::vector<std::vector<usize>> edgesForVertex;

        bool directed = false;
    };
} // namespace gpx

#endif
