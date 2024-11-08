#ifndef _GRAPHEXIA_GRAPHVIEW__HPP_
#define _GRAPHEXIA_GRAPHVIEW__HPP_

#include <Graphexia/Graph.hpp>
#include <Graphexia/GraphViewRenderer.hpp>
#include <vector>

namespace gpx {
    struct VertexView {
        i16x2 position;
        i32 size;

        bool collides(i16x2 p) const {
            i16 dX = this->position.x - p.x;
            i16 dY = this->position.y - p.y;

            return static_cast<i32>(dX * dX) + static_cast<i32>(dY * dY) <= this->size * this->size;
        }
    };

    struct GraphView final {
        constexpr explicit GraphView()
            : graph(), views() {}

        template<GraphViewRenderer Renderer>
        constexpr explicit GraphView(const Graph& graph, Renderer renderer)
            : graph(graph), views(std::vector<VertexView>(graph.Vertices())) {
            for (usize i = 0; i < graph.Vertices(); ++i) {
                this->views[i] = {renderer.Render(i), 3};
            }
        }

        void SetDirected(bool directed) { this->graph.SetDirected(directed); }

        usize AddVertex(i16x2 position, i32 size = 3) {
            usize vertexId = this->graph.AddVertex();

            this->views.push_back(VertexView{position, size});
            return vertexId;
        }
        void AddEdge(usize from, usize to, f32 weight = 0) { return this->graph.AddEdge(from, to, weight); }
        void EraseEdge(usize id) { this->graph.EraseEdge(id); }
        void EraseVertex(usize id) {
            this->graph.EraseVertex(id);
            this->views.erase(this->views.begin() + id);
        }

        usize FindVertex(i16x2 position, usize startingVertex = gpx::Graph::NoId) const;
        usize FindEdge(i16x2 position) const;

        void MoveVertex(usize id, i16x2 position) { this->views[id].position = position; }

        const std::vector<VertexView>& Views() const { return this->views; }
        const Graph& GetGraph() const { return this->graph; }
    private:
        Graph graph;
        std::vector<VertexView> views;
    };
} // namespace gpx

#endif
