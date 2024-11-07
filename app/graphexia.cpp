#include "graphexia.hpp"
#include "Graphexia/graphrenderer.hpp"
#include "Graphexia/graphtypes.hpp"
#include "Graphexia/graphview.hpp"

Graphexia::Graphexia()
    : view(gpx::CreateKComplete(10), gpx::CircularGraphRenderer({}, 100, 10)), mode(GraphMode::EditVertices), selectedVertex(gpx::Graph::NoVertex), cameraPosition({}), cameraZoom(1) {
}

void Graphexia::Init() {
}

void Graphexia::Update(nk_context* ctx) {
    const gpx::Graph& graph = this->view.GetGraph();

    if(nk_begin(ctx, "Graphexia", nk_rect(0, 0, 300, 450), NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_MINIMIZABLE)) {
        nk_layout_row_static(ctx, 14, 270, 1);
        nk_label_wrap(ctx, "Welcome to Graphexia!");
        nk_layout_row_static(ctx, 60, 270, 1);
        nk_label_wrap(ctx, "To move the camera, drag while clicking the middle mouse button. To zoom in or out, use the scrollwheel");
        nk_label_wrap(ctx, "To add vertices, see their info or move them, use the left mouse button while being in the 'Edit' mode");
        nk_label_wrap(ctx, "To add edges, use the left mouse and click a vertex, a line will follow the mouse which will be the new edge if you click other vertex. If you changed your mind press right click.");
        nk_layout_row_static(ctx, 14, 270, 1);
        nk_label_wrap(ctx, "Built with sokol + nuklear");

        nk_layout_row_dynamic(ctx, 30, 2);
        if(nk_option_label(ctx, "Edit Vertices", this->mode == GraphMode::EditVertices)) {
            this->ChangeMode(GraphMode::EditVertices);
        }

        if(nk_option_label(ctx, "Draw Edges", this->mode == GraphMode::DrawEdges)) {
            this->ChangeMode(GraphMode::DrawEdges);
        }

        if(nk_button_label(ctx, "Reset Graph")) {
            this->view = gpx::GraphView();
            this->selectedVertex = gpx::Graph::NoVertex;
        }

        nk_layout_row_static(ctx, 14, 80, 1);
        if(nk_tree_push(ctx, NK_TREE_TAB, "K Complete Graphs", NK_MINIMIZED)) {
            nk_property_int(ctx, "K", 1, &this->savedSelectedKComplete, 40, 1, 1);
            nk_property_int(ctx, "Radius", 1, &this->savedSelectedRadius, 400, 1, 1);

            if(nk_button_label(ctx, "Render K Complete Graph")) {
                this->view = gpx::GraphView(gpx::CreateKComplete(this->savedSelectedKComplete), gpx::CircularGraphRenderer({}, this->savedSelectedRadius, this->savedSelectedKComplete)); 
                this->selectedVertex = gpx::Graph::NoVertex;
            }

            nk_tree_pop(ctx);
        }

        nk_end(ctx);
    }

    if(nk_begin(ctx, "Graph Info", nk_rect(10, 10, 120, 150), NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_MINIMIZABLE)) {
        nk_layout_row_static(ctx, 16, 80, 1);
        
        nk_labelf(ctx, NK_TEXT_LEFT, "Vertices: %zu", graph.Vertices()); 
        nk_labelf(ctx, NK_TEXT_LEFT, "Edges: %zu", graph.Edges().size());
        if(nk_button_label(ctx, graph.IsDirected() ? "Directed" : "Undirected")) {
            this->view.SetDirected(!graph.IsDirected());
        }

        nk_end(ctx);
    }

    switch (this->mode) {
        case GraphMode::EditVertices: { if(this->selectedVertex != gpx::Graph::NoVertex) {
                if(nk_begin(ctx, "Vertex Info", nk_rect(0, 0, 120, 150), NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_MINIMIZABLE)) {
                    nk_layout_row_static(ctx, 18, 80, 1); 

                    nk_labelf(ctx, NK_TEXT_LEFT, "ID: %zu", this->selectedVertex);
                    nk_labelf(ctx, NK_TEXT_LEFT, "Degree: %zu", graph.EdgesForVertex(this->selectedVertex).size());

                    nk_end(ctx);
                }
            }
            break;
        }
        case GraphMode::DrawEdges: {
            break;
        }
    }
}

void Graphexia::Render() {
    i32 width = sapp_width();
    i32 height = sapp_height();
    sgp_viewport(0, 0, width, height);
    sgp_project(0, width, 0, height);

    sgp_set_color(0.1, 0.1, 0.1, 1);
    sgp_clear();

    sgp_push_transform();
        sgp_translate(width/2.0, height/2.0);
        sgp_translate(this->cameraPosition.x, this->cameraPosition.y);
        sgp_scale(this->cameraZoom, this->cameraZoom);

        const gpx::Graph& graph = this->view.GetGraph();
        const std::vector<gpx::VertexView>& vertexViews = this->view.Views();
        const std::vector<gpx::Edge>& edges = graph.Edges();

        sgp_set_color(1, 1, 1, 1);
        for (const gpx::Edge& edge : edges) {
            const gpx::VertexView& from = vertexViews[edge.fromId]; 
            const gpx::VertexView& to = vertexViews[edge.toId]; 

            if(edge.fromId != edge.toId) {
                sgp_draw_line(from.position.x, from.position.y, to.position.x, to.position.y);
                continue;
            }

            //DrawCircleLines(from.position.x - 3, from.position.y - 3, 7, {255, 255, 255, 255});
        }

        sgp_set_color(.1, .9, .5, 1);
        if(this->mode == GraphMode::DrawEdges && this->selectedVertex != gpx::Graph::NoVertex) {
            const gpx::VertexView& view = this->view.Views()[selectedVertex];
            sgp_draw_line(view.position.x, view.position.y, currentMouseWorldPosition.x, currentMouseWorldPosition.y);
        }

        sgp_set_color(1, 1, 1, 1);
        for (const gpx::VertexView& vertex : vertexViews) {
            f32 halfSize = vertex.size / 2.0;
            sgp_draw_filled_rect(vertex.position.x - halfSize, vertex.position.y - halfSize, vertex.size, vertex.size);
        }
    sgp_pop_transform();
}

void Graphexia::Event(const sapp_event* event) {
    switch (event->type) {
        case SAPP_EVENTTYPE_MOUSE_SCROLL: {
            this->cameraZoom += event->scroll_y * 0.1;
            break;
        }
        case SAPP_EVENTTYPE_MOUSE_DOWN: {
            switch (event->mouse_button) {
                case SAPP_MOUSEBUTTON_LEFT: {
                    Vec2 worldPosition = ScreenToWorld({event->mouse_x, event->mouse_y});
                    gpx::Point viewPosition = {static_cast<i16>(worldPosition.x), static_cast<i16>(worldPosition.y)};
                    usize newSelectedVertex = this->view.FindVertex(viewPosition);

                    switch (this->mode) {
                        case GraphMode::EditVertices: {
                            if(this->selectedVertex == gpx::Graph::NoVertex && newSelectedVertex == gpx::Graph::NoVertex) {
                                this->view.AddVertex(viewPosition);
                            }

                            this->selectedVertex = newSelectedVertex;

                            if(this->selectedVertex != gpx::Graph::NoVertex) {
                                const gpx::VertexView& vertex = this->view.Views()[this->selectedVertex];
                                this->selectedVertexMouseOffset = { vertex.position.x - worldPosition.x, vertex.position.y - worldPosition.y };
                                this->currentlyDraggingVertex = true;
                            }
                            break;
                        }
                        case GraphMode::DrawEdges: { 
                            if(this->selectedVertex == gpx::Graph::NoVertex) {
                                this->selectedVertex = newSelectedVertex;
                                break;
                            }

                            this->view.AddEdge(this->selectedVertex, newSelectedVertex); 
                            this->selectedVertex = gpx::Graph::NoVertex;
                            break;
                        }
                    }
                    break;
                }
                case SAPP_MOUSEBUTTON_RIGHT: {
                    switch (this->mode) {
                        case GraphMode::DrawEdges: {
                            this->selectedVertex = gpx::Graph::NoVertex;
                            break;
                        }
                        default: break;
                    }
                    break;
                }
                case SAPP_MOUSEBUTTON_MIDDLE: this->movingCamera = true; break;
                default: break;
            }
            break;
        }
        case SAPP_EVENTTYPE_MOUSE_UP: {
            switch (event->mouse_button) {
                case SAPP_MOUSEBUTTON_LEFT: {
                    switch (this->mode) {
                        case GraphMode::EditVertices: {
                            this->currentlyDraggingVertex = false;
                            break;
                        }
                        default: break;
                    }
                    break;
                }
                case SAPP_MOUSEBUTTON_MIDDLE: this->movingCamera = false; break;
                default: break;
            }
            break;
        }
        case SAPP_EVENTTYPE_MOUSE_MOVE: {
            if(this->movingCamera) {
                this->cameraPosition.x += event->mouse_dx;
                this->cameraPosition.y += event->mouse_dy;
                break;
            }
            
            this->currentMouseWorldPosition = ScreenToWorld({event->mouse_x, event->mouse_y});
            switch (this->mode) {
                case GraphMode::EditVertices: {
                    if(this->currentlyDraggingVertex) {
                        this->view.MoveVertex(this->selectedVertex, {static_cast<i16>(this->currentMouseWorldPosition.x + this->selectedVertexMouseOffset.x), static_cast<i16>(this->currentMouseWorldPosition.y + this->selectedVertexMouseOffset.y)}); 
                    }
                    break;
                }
                default: break;
            }
            break;
        }
        default: break;
    }
}

Vec2 Graphexia::ScreenToWorld(Vec2 position) {
    f32 hW = sapp_width()/2., hH = sapp_height()/2.;
    return { (position.x - hW - this->cameraPosition.x) / this->cameraZoom, (position.y - hH - this->cameraPosition.y) / this->cameraZoom };
}

void Graphexia::ChangeMode(const GraphMode mode) {
    if(this->mode == mode)
        return;

    this->mode = mode;
    this->selectedVertex = gpx::Graph::NoVertex;
}
