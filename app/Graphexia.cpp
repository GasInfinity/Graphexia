#include "Graphexia.hpp"
#include <Graphexia/GraphViewRenderer.hpp>
#include <Graphexia/GraphTypes.hpp>
#include <Graphexia/GraphView.hpp>

#include <sokol/sokol_gfx.h>
#include "Core.hpp"
#include "sokol/sokol_gp.h"

Graphexia::Graphexia()
    : view(gpx::CreateKComplete(10), gpx::CircularGraphViewRenderer({}, 100, 10)), renderer(), mode(GraphexiaMode::EditVertices), selectedId(gpx::Graph::NoId), cameraPosition({}), cameraZoom(1) {
}

void Graphexia::Init() {
    this->renderer.Init();
}

void Graphexia::Update(nk_context* ctx) {
    const gpx::Graph& graph = this->view.GetGraph();

    nk_style_hide_cursor(ctx);

    if(nk_begin(ctx, "Graphexia", nk_rect(0, 0, 300, 450), NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_MINIMIZABLE)) {
        nk_layout_row_dynamic(ctx, 14, 1);
        nk_label_wrap(ctx, "Welcome to Graphexia!");

        nk_layout_row_dynamic(ctx, 60, 1);
        nk_label_wrap(ctx, "To move the camera, drag while clicking the middle mouse button. To zoom in or out, use the scrollwheel");
        nk_label_wrap(ctx, "To add vertices, see their info or move them, use the left mouse button while being in the 'Edit' mode");
        nk_label_wrap(ctx, "To delete vertices, select a vertex and press right click. The first one will mark it for deletion and the second one will delete it");
        nk_label_wrap(ctx, "To add edges, use the left mouse and click a vertex, a line will follow the mouse which will be the new edge if you click other vertex. If you changed your mind press right click.");
        nk_layout_row_dynamic(ctx, 14, 1);
        nk_label_wrap(ctx, "Built with sokol + nuklear");

        nk_layout_row_dynamic(ctx, 30, 2);
        if(nk_option_label(ctx, "Edit Vertices", this->mode == GraphexiaMode::EditVertices)) {
            this->ChangeMode(GraphexiaMode::EditVertices);
        }

        if(nk_option_label(ctx, "Edit Edges", this->mode == GraphexiaMode::EditEdges)) {
            this->ChangeMode(GraphexiaMode::EditEdges);
        }

        nk_layout_row_dynamic(ctx, 30, 1);
        if(nk_button_label(ctx, "Reset Graph")) {
            this->view = gpx::GraphView();
            this->selectedId = gpx::Graph::NoId;
        }

        nk_layout_row_static(ctx, 14, 80, 1);
        if(nk_tree_push(ctx, NK_TREE_TAB, "K Complete Graphs", NK_MINIMIZED)) {
            nk_property_int(ctx, "K", 1, &this->savedSelectedKComplete, 40, 1, 1);
            nk_property_int(ctx, "Radius", 1, &this->savedSelectedRadius, 400, 1, 1);

            if(nk_button_label(ctx, "Render K Complete Graph")) {
                this->view = gpx::GraphView(gpx::CreateKComplete(this->savedSelectedKComplete), gpx::CircularGraphViewRenderer({}, this->savedSelectedRadius, this->savedSelectedKComplete)); 
                this->selectedId = gpx::Graph::NoId;
            }

            nk_tree_pop(ctx);
        }
    }
    nk_end(ctx);

    if(nk_begin(ctx, "Graph Info", nk_rect(10, 10, 120, 150), NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_MINIMIZABLE)) {
        nk_layout_row_static(ctx, 16, 80, 1);
        
        nk_labelf(ctx, NK_TEXT_LEFT, "Vertices: %zu", graph.Vertices()); 
        nk_labelf(ctx, NK_TEXT_LEFT, "Edges: %zu", graph.Edges().size());
        if(nk_button_label(ctx, graph.IsDirected() ? "Directed" : "Undirected")) {
            this->view.SetDirected(!graph.IsDirected());
        }
    }
    nk_end(ctx);

    if(nk_begin(ctx, "Selection Info", nk_rect(0, 0, 130, 150), NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_MINIMIZABLE)) {
        switch (this->mode) {
            case GraphexiaMode::EditVertices: {
                if(this->selectedId != gpx::Graph::NoId) {
                    nk_layout_row_static(ctx, 18, 80, 1); 
                    
                    nk_label(ctx, "Vertex", NK_TEXT_LEFT);
                    nk_labelf(ctx, NK_TEXT_LEFT, "ID: %zu", this->selectedId);
                    nk_labelf(ctx, NK_TEXT_LEFT, "Degree: %zu", graph.EdgesForVertex(this->selectedId).size());
                }
                break;
            }
            case GraphexiaMode::EditEdges: { 
                break;
            }
        }
    }
    nk_end(ctx);
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

        this->renderer.RenderView(this->view, this->selectionType, this->selectedId, this->currentMouseWorldPosition); 
    sgp_pop_transform();
}

void Graphexia::Event(const sapp_event* event) {
    switch (event->type) {
        case SAPP_EVENTTYPE_MOUSE_SCROLL: {
            this->cameraZoom += event->scroll_y * (event->modifiers & SAPP_MODIFIER_SHIFT ? 0.25 : 0.1);

            if(this->cameraZoom < 0.1) {
                this->cameraZoom = 0.1;
            }
            break;
        }
        case SAPP_EVENTTYPE_MOUSE_DOWN: {
            switch (event->mouse_button) {
                case SAPP_MOUSEBUTTON_LEFT: {
                    f32x2 worldPosition = ScreenToWorld({event->mouse_x, event->mouse_y});
                    gpx::i16x2 viewPosition = {static_cast<i16>(worldPosition.x), static_cast<i16>(worldPosition.y)};
                    usize newSelection = this->view.FindVertex(viewPosition, this->selectedId);

                    switch (this->mode) {
                        case GraphexiaMode::EditVertices: {
                            if(newSelection == gpx::Graph::NoId) {
                                newSelection = this->view.FindVertex(viewPosition);

                                if(this->selectedId == newSelection && newSelection == gpx::Graph::NoId) {
                                    this->view.AddVertex(viewPosition);
                                }
                            }

                            this->selectedId = newSelection;
                            this->selectionType = SelectionType::VertexSelected; 

                            if(this->selectedId != gpx::Graph::NoId) {
                                const gpx::VertexView& vertex = this->view.Views()[this->selectedId];
                                this->selectedVertexMouseOffset = { vertex.position.x - worldPosition.x, vertex.position.y - worldPosition.y };
                                this->currentlyDraggingVertex = true;
                            }
                            break;
                        }
                        case GraphexiaMode::EditEdges: {
                            if(newSelection == gpx::Graph::NoId) {
                                newSelection = this->view.FindVertex(viewPosition);

                                if(newSelection == gpx::Graph::NoId) {
                                    break;
                                }
                            }

                            if(this->selectedId == gpx::Graph::NoId) {
                                this->selectedId = newSelection;
                                this->selectionType = SelectionType::VertexDrawingEdge;
                                break;
                            }

                            this->view.AddEdge(this->selectedId, newSelection); 
                            this->selectionType = SelectionType::None;
                            this->selectedId = gpx::Graph::NoId;
                            break;
                        }
                    }
                    break;
                }
                case SAPP_MOUSEBUTTON_RIGHT: {
                    f32x2 worldPosition = ScreenToWorld({event->mouse_x, event->mouse_y});
                    gpx::i16x2 viewPosition = {static_cast<i16>(worldPosition.x), static_cast<i16>(worldPosition.y)};

                    switch (this->mode) {
                        case GraphexiaMode::EditVertices: {
                            if((this->selectionType & SelectionType::VertexSelected) == SelectionType::VertexSelected && this->selectedId != gpx::Graph::NoId) {
                                const gpx::VertexView& vView = this->view.Views()[this->selectedId];
                                
                                if(vView.collides(viewPosition)) {
                                    if((this->selectionType & SelectionType::DeletionRequest) != SelectionType::DeletionRequest) {
                                        this->selectionType = SelectionType::VertexDeletionRequest;
                                        break;
                                    }

                                    this->view.EraseVertex(this->selectedId);
                                    this->selectionType = SelectionType::None;
                                    this->selectedId = gpx::Graph::NoId;
                                }
                            }
                        }
                        case GraphexiaMode::EditEdges: {
                            this->selectedId = gpx::Graph::NoId;
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
                        case GraphexiaMode::EditVertices: {
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
                case GraphexiaMode::EditVertices: {
                    if(this->currentlyDraggingVertex) {
                        this->view.MoveVertex(this->selectedId, {static_cast<i16>(this->currentMouseWorldPosition.x + this->selectedVertexMouseOffset.x), static_cast<i16>(this->currentMouseWorldPosition.y + this->selectedVertexMouseOffset.y)}); 
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

f32x2 Graphexia::ScreenToWorld(f32x2 position) {
    f32 hW = sapp_width()/2., hH = sapp_height()/2.;
    return { (position.x - hW - this->cameraPosition.x) / this->cameraZoom, (position.y - hH - this->cameraPosition.y) / this->cameraZoom };
}

void Graphexia::ChangeMode(const GraphexiaMode mode) {
    if(this->mode == mode)
        return;

    this->mode = mode;
    this->selectedId = gpx::Graph::NoId;
}
