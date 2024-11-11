#include "GraphView.hpp"
#include <vector>

usize GraphView::FindVertex(f32x2 position, usize startingVertex) const {
    for (usize i = startingVertex + 1; i < this->vertices.size(); ++i) {
        if(this->vertices[i].Collides(position)) {
            return i;
        }
    }

    return NoId;
}

usize GraphView::FindEdge(f32x2 position) const {
    constexpr f32 MinimumDistance = 1;

    const std::vector<gpx::Edge>& edges = this->graph.Edges();

    for (usize i = 0; i < edges.size(); ++i) {
        const gpx::Edge& edge = edges[i]; 

        const Vertex& starting = this->vertices[edge.fromId];
        const Vertex& ending = this->vertices[edge.toId];

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

    return NoId;
}
