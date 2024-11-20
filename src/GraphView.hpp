#ifndef _GRAPHEXIA_APP_GRAPHVIEW__HPP_
#define _GRAPHEXIA_APP_GRAPHVIEW__HPP_

#include <Graphexia/Graph.hpp>
#include "GraphViewRenderer.hpp"

#include <array>
#include <vector>

struct Vertex {
    static constexpr usize MaxLabelLength = 19;

    u8 labelSize;
    std::array<char, MaxLabelLength> label;
    f32x2 position;
    f32 size;

    bool Collides(f32x2 p) const {
        f32 dX = this->position.x - p.x;
        f32 dY = this->position.y - p.y;

        return (dX * dX) + (dY * dY) <= this->size * this->size;
    }
};

struct GraphView final {
    static constexpr usize NoId = std::numeric_limits<usize>::max();

    constexpr explicit GraphView()
        : graph(), vertices() {}

    template<GraphViewRenderer Renderer>
    constexpr explicit GraphView(const gpx::Graph& graph, Renderer renderer)
        : graph(graph), vertices(std::vector<Vertex>(graph.Vertices())) {
        for (usize i = 0; i < graph.Vertices(); ++i) {
            this->vertices[i] = {0, {}, renderer.Render(i), 3};
        }
    }

    void SetDirected(bool directed) { this->graph.SetDirected(directed); }

    usize AddVertex(f32x2 position, f32 size = 3) {
        usize vertexId = this->graph.AddVertex();

        this->vertices.push_back(Vertex{0, {}, position, size});
        return vertexId;
    }
    void AddEdge(usize from, usize to, f32 weight = 0) { return this->graph.AddEdge(from, to, weight); }
    void EraseEdge(usize id) { this->graph.EraseEdge(id); }
    void EraseVertex(usize id) {
        this->graph.EraseVertex(id);
        this->vertices.erase(this->vertices.begin() + id);
    }

    usize FindVertex(f32x2 position, usize startingVertex = NoId) const;
    usize FindEdge(f32x2 position, f32 minimumDistance = 1) const;

    void MoveVertex(usize id, f32x2 position) { this->vertices[id].position = position; }
    Vertex& View(usize id) { return this->vertices[id]; }

    f32& EdgeWeight(usize id) { return this->graph.EdgeWeight(id); }

    const std::vector<Vertex>& Vertices() const { return this->vertices; }
    const gpx::Graph& GetGraph() const { return this->graph; }
private:
    gpx::Graph graph;
    std::vector<Vertex> vertices;
};

#endif
