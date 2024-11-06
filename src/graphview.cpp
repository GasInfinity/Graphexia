#include <Graphexia/graphview.hpp>

namespace gpx {
    usize GraphView::FindVertex(Point position) const {
        for (usize i = 0; i < this->views.size(); ++i) {
            if(this->views[i].collides(position)) {
                return i;
            }
        }

        return gpx::Graph::NoVertex;
    }
} // namespace gpx
