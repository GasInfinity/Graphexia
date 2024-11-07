#ifndef _GRAPHEXIA_GRAPHVIEW__HPP_
#define _GRAPHEXIA_GRAPHVIEW__HPP_

#include <Graphexia/graph.hpp>
#include <Graphexia/graphrenderer.hpp>
#include <vector>

namespace gpx {
    struct VertexView {
        Point position;
        i32 size;

        bool collides(Point p) const {
            i16 dX = this->position.x - p.x;
            i16 dY = this->position.y - p.y;

            return static_cast<i32>(dX * dX) + static_cast<i32>(dY * dY) <= this->size * this->size;
        }
    };

    struct GraphView final {
        constexpr explicit GraphView()
            : graph(), views() {}

        template<GraphRenderer Renderer>
        constexpr explicit GraphView(const Graph& graph, Renderer renderer)
            : graph(graph), views(std::vector<VertexView>(graph.Vertices())) {
            for (usize i = 0; i < graph.Vertices(); ++i) {
                this->views[i] = {renderer.Render(i), 5};
            }
        }

        void SetDirected(bool directed) { this->graph.SetDirected(directed); }

        usize AddVertex(Point position, i32 size = 6) {
            usize vertexId = this->graph.AddVertex();

            this->views.push_back(VertexView{position, size});
            return vertexId;
        }
        void AddEdge(usize from, usize to, f32 weight = 0) { return this->graph.AddEdge(from, to, weight); }

        usize FindVertex(Point position) const;
        void MoveVertex(usize id, Point position) {
            this->views[id].position = position;
        }

        const std::vector<VertexView>& Views() const { return this->views; }
        const Graph& GetGraph() const { return this->graph; }
    private:
        Graph graph;
        std::vector<VertexView> views;
    };
} // namespace gpx

#endif
