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

        f32 directorX = ending.position.x - starting.position.x;
        f32 directorY = ending.position.y - starting.position.y;

        f32 c = directorY * starting.position.x - directorX * starting.position.y;

        f32 numerator = directorX * position.y - (directorY * position.x) + c;
        f32 distanceSqr = (numerator * numerator) / (directorX * directorX + directorY * directorY);

        if(distanceSqr <= MinimumDistance * MinimumDistance) {
            return i;
        }
    }

    return NoId;
}
