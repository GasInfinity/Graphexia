#ifndef _GRAPHEXIA_GRAPH__HPP_
#define _GRAPHEXIA_GRAPH__HPP_

#include <Graphexia/core.hpp>
#include <cassert>
#include <span>
#include <limits>
#include <vector>

namespace gpx {
    struct Edge final {
        usize fromId, toId;
        f32 weight; 
    };

    struct Graph final {
        static constexpr usize NoVertex = std::numeric_limits<usize>::max();

        constexpr explicit Graph()
            : vertices(), edges(), edgesForVertex(), directed() {}

        constexpr explicit Graph(usize vertices)
            : vertices(vertices), edges(), edgesForVertex(std::vector<std::vector<usize>>(vertices)), directed() {}

        constexpr explicit Graph(usize vertices, std::span<Edge> edges)
            : vertices(vertices), edges(std::vector<Edge>(edges.size())), edgesForVertex(std::vector<std::vector<usize>>(vertices)), directed() {
            for (usize i = 0; i < edges.size(); ++i) {
                const Edge& edge = edges[i];

                assert(edge.fromId < vertices && edge.toId < vertices);
                this->edges[i] = edge;
            }
        }

        bool IsDirected() const { return this->directed; }
        void SetDirected(bool directed) { this->directed = directed; }

        usize AddVertex() {
            this->edgesForVertex.push_back(std::vector<usize>());
            return this->vertices++;
        }
        void AddVertices(usize n) {
            assert(n > 0);

            this->vertices += n;
            this->edgesForVertex.resize(this->vertices);
        }
        void AddEdge(usize from, usize to, f32 weight = 0) {
            usize edgeId = this->edges.size();

            this->edgesForVertex[from].push_back(edgeId);
            this->edgesForVertex[to].push_back(edgeId);

            this->edges.push_back(Edge{from, to, weight});
        }

        usize Vertices() const { return this->vertices; }
        const std::vector<Edge>& Edges() const { return this->edges; }
        const std::vector<usize>& EdgesForVertex(usize id) const { return this->edgesForVertex[id]; }
    private:
        usize vertices;
        std::vector<Edge> edges;
        std::vector<std::vector<usize>> edgesForVertex;
        bool directed;
    };
} // namespace gpx

#endif
