#include <Graphexia/GraphView.hpp>
#include <vector>

namespace gpx {
    usize GraphView::FindVertex(i16x2 position, usize startingVertex) const {
        for (usize i = startingVertex + 1; i < this->views.size(); ++i) {
            if(this->views[i].collides(position)) {
                return i;
            }
        }

        return gpx::Graph::NoId;
    }

    usize GraphView::FindEdge(i16x2 position) const {
        constexpr i32 MinimumDistance = 1;

        const std::vector<Edge>& edges = this->graph.Edges();

        for (usize i = 0; i < edges.size(); ++i) {
            const Edge& edge = edges[i]; 

            const VertexView& starting = this->views[edge.fromId];
            const VertexView& ending = this->views[edge.toId];

            i32 directorX = ending.position.x - starting.position.x;
            i32 directorY = ending.position.y - starting.position.y;
            
            f32 t = static_cast<f32>(position.x - starting.position.x) / (directorX + directorY);

            f32 crossX = starting.position.x + directorX * t;
            f32 crossY = starting.position.y + directorY * t;

            i32 dX = crossX - position.x;
            i32 dY = crossY - position.y;

            i32 distanceSqr = dX * dX + dY * dY;

            if(distanceSqr <= MinimumDistance * MinimumDistance) {
                return i;
            }
        }

        return gpx::Graph::NoId;
    }
} // namespace gpx
