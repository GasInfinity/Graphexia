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

usize GraphView::FindEdge(f32x2 position, f32 minimumDistance) const {
    f32 halfMinimumDistance = minimumDistance / 2.f;

    const std::vector<gpx::Edge>& edges = this->graph.Edges();
    for (usize i = 0; i < edges.size(); ++i) {
        const gpx::Edge& edge = edges[i]; 

        const Vertex& starting = this->vertices[edge.fromId];
        const Vertex& ending = this->vertices[edge.toId];

        f32 minX = std::min(starting.position.x, ending.position.x) - halfMinimumDistance;
        f32 minY = std::min(starting.position.y, ending.position.y) - halfMinimumDistance;
        f32 maxX = std::max(starting.position.x, ending.position.x) + halfMinimumDistance;
        f32 maxY = std::max(starting.position.y, ending.position.y) + halfMinimumDistance;

        if(position.x < minX || position.x > maxX || position.y < minY || position.y > maxY) {
            continue;
        }

        f32 directorX = ending.position.x - starting.position.x;
        f32 directorY = ending.position.y - starting.position.y;

        f32 c = directorY * starting.position.x - directorX * starting.position.y;

        f32 numerator = directorX * position.y - (directorY * position.x) + c;
        f32 distanceSqr = (numerator * numerator) / (directorX * directorX + directorY * directorY);

        if(distanceSqr <= minimumDistance * minimumDistance) {
            return i;
        }
    }

    return NoId;
}
