#include "Graphexia.hpp"
#include "Core.hpp"
#include "GraphView.hpp"
#include "GraphViewRenderer.hpp"
#include "Graphexia/Graph.hpp"
#include <Graphexia/GraphTypes.hpp>

#include <algorithm>
#include <sokol/sokol_gfx.h>

Graphexia::Graphexia()
    : view(gpx::CreateKComplete(4), CircularGraphViewRenderer({0,0}, 60, 4)), renderer(), mode(GraphexiaMode::EditVertices), selectedId(GraphView::NoId) {
}

void Graphexia::Init() {
    this->renderer.Init({static_cast<u32>(sapp_width()), static_cast<u32>(sapp_height())});
    this->renderer.ReconstructView(this->view);
}

void Graphexia::Update(nk_context* ctx) {
    this->renderer.SetViewport({static_cast<u32>(sapp_width()), static_cast<u32>(sapp_height())});

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

        nk_labelf(ctx, NK_TEXT_LEFT, "Frametime: %f | FPS: %f", sapp_frame_duration(), 1 / sapp_frame_duration());
        nk_labelf(ctx, NK_TEXT_LEFT, "Camera at %f, %f", this->renderer.GetCameraPosition().x, this->renderer.GetCameraPosition().y);
        nk_layout_row_dynamic(ctx, 30, 2);
        if(nk_option_label(ctx, "Edit Vertices", this->mode == GraphexiaMode::EditVertices)) {
            this->ChangeMode(GraphexiaMode::EditVertices);
        }

        if(nk_option_label(ctx, "Edit Edges", this->mode == GraphexiaMode::EditEdges)) {
            this->ChangeMode(GraphexiaMode::EditEdges);
        }

        nk_layout_row_dynamic(ctx, 30, 1);
        if(nk_button_label(ctx, "Reset Graph")) {
            this->view = GraphView();
            this->selectionType = SelectionType::None;
            this->selectedId = GraphView::NoId;
            this->renderer.ReconstructView(this->view);
        }

        nk_layout_row_static(ctx, 14, 80, 1);
        if(nk_tree_push(ctx, NK_TREE_TAB, "K Complete Graphs", NK_MINIMIZED)) {
            nk_property_int(ctx, "K", 1, &this->savedSelectedKComplete, 400, 1, 1);
            nk_property_int(ctx, "Radius", 1, &this->savedSelectedRadius, 2000, 1, 1);

            if(nk_button_label(ctx, "Render K Complete Graph")) {
                this->view = GraphView(gpx::CreateKComplete(this->savedSelectedKComplete), CircularGraphViewRenderer({}, this->savedSelectedRadius, this->savedSelectedKComplete)); 
                this->renderer.ReconstructView(this->view);
                this->selectedId = GraphView::NoId;
            }

            nk_tree_pop(ctx);
        }
    }
    nk_end(ctx);

    if(nk_begin(ctx, "Graph Info", nk_rect(10, 10, 160, 150), NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_MINIMIZABLE)) {
        nk_layout_row_dynamic(ctx, 16, 1);
        
        nk_labelf(ctx, NK_TEXT_LEFT, "Vertices: %zu", graph.Vertices()); 
        nk_labelf(ctx, NK_TEXT_LEFT, "Edges: %zu", graph.Edges().size());
        if(nk_button_label(ctx, graph.IsDirected() ? "Directed" : "Undirected")) {
            this->view.SetDirected(!graph.IsDirected());
        }
    }
    nk_end(ctx);

    if(nk_begin(ctx, "Selection Info", nk_rect(0, 0, 130, 150), NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_MINIMIZABLE | NK_WINDOW_SCALABLE)) {
        nk_layout_row_dynamic(ctx, 18, 1); 
        switch (this->mode) {
            case GraphexiaMode::EditVertices: {
                if(this->selectedId != GraphView::NoId) {
                    nk_label(ctx, "Vertex", NK_TEXT_LEFT);
                    nk_labelf(ctx, NK_TEXT_LEFT, "ID: %zu", this->selectedId);
                    nk_labelf(ctx, NK_TEXT_LEFT, "Degree: %zu", graph.EdgesForVertex(this->selectedId).size());

                    Vertex& view = this->view.View(this->selectedId);
                    i32 labelSize = view.labelSize;

                    nk_edit_string(ctx, NK_EDIT_FIELD, view.label.data(), &labelSize, Vertex::MaxLabelLength, nk_filter_default);
                    view.labelSize = labelSize;
                }
                break;
            }
            case GraphexiaMode::EditEdges: { 
                if(this->selectedId != GraphView::NoId) {
                    if((this->selectionType & SelectionType::EdgeSelected) == SelectionType::EdgeSelected) {
                        const gpx::Edge& edge = graph.Edges()[this->selectedId];
                        nk_label(ctx, "Edge", NK_TEXT_LEFT);
                        nk_labelf(ctx, NK_TEXT_LEFT, "%zu -> %zu", edge.fromId, edge.toId);
                        nk_labelf(ctx, NK_TEXT_LEFT, "Weight: %f", edge.weight);
                    } else if((this->selectionType & SelectionType::VertexSelected) == SelectionType::VertexSelected) {
                        nk_label(ctx, "Creating edge", NK_TEXT_LEFT);
                        nk_labelf(ctx, NK_TEXT_LEFT, "From ID: %zu", this->selectedId);
                    }
                }
                break;
            }
        }
    }
    nk_end(ctx);
}

void Graphexia::Render() {
    this->renderer.Render(this->view, this->selectionType, this->selectedId, this->currentMouseWorldPosition); 
}

void Graphexia::Event(const sapp_event* event) {
    switch (event->type) {
        case SAPP_EVENTTYPE_MOUSE_SCROLL: {
            f32x2 lastWorldSpaceMouse = this->renderer.ScreenToWorld({event->mouse_x, event->mouse_y});
            bool zoomingIn = event->scroll_y > 0;
            f32 newZoom = std::clamp(this->renderer.GetCameraZoom() * (zoomingIn ? 1.1f : 1/1.1f), .8f, 15.f);

            this->renderer.SetCameraZoom(newZoom);
            f32x2 newWorldSpaceMouse = this->renderer.ScreenToWorld({event->mouse_x, event->mouse_y});

            f32x2 cameraPosition = this->renderer.GetCameraPosition();
            cameraPosition.x -= (lastWorldSpaceMouse.x - newWorldSpaceMouse.x) * newZoom;
            cameraPosition.y -= (lastWorldSpaceMouse.y - newWorldSpaceMouse.y) * newZoom;
            this->renderer.SetCameraPosition(cameraPosition);
            break;
        }
        case SAPP_EVENTTYPE_MOUSE_DOWN: {
            switch (event->mouse_button) {
                case SAPP_MOUSEBUTTON_LEFT: {
                    f32x2 worldPosition = this->renderer.ScreenToWorld({event->mouse_x, event->mouse_y});
                    usize newSelection = this->view.FindVertex(worldPosition, this->selectedId);

                    switch (this->mode) {
                        case GraphexiaMode::EditVertices: {
                            usize lastSelection = this->selectedId;

                            if(newSelection == GraphView::NoId) {
                                newSelection = this->view.FindVertex(worldPosition);

                                if(this->selectedId == newSelection && newSelection == GraphView::NoId) {
                                    this->view.AddVertex(worldPosition);
                                    this->renderer.AddVertex(this->view.Views().back());
                                }
                            }

                            this->selectedId = newSelection;
                            this->selectionType = SelectionType::VertexSelected; 

                            if(lastSelection != GraphView::NoId) {
                                this->renderer.UpdateVertexColor(lastSelection, 0xFFFFFFFF);
                            }

                            if(this->selectedId != GraphView::NoId) {
                                const Vertex& vertex = this->view.Views()[this->selectedId];
                                this->selectedVertexMouseOffset = { vertex.position.x - worldPosition.x, vertex.position.y - worldPosition.y };
                                this->currentlyDraggingVertex = true;

                                this->renderer.UpdateVertexColor(this->selectedId, 0x00FF00FF);
                            }

                            break;
                        }
                        case GraphexiaMode::EditEdges: {
                            if(newSelection == GraphView::NoId) {
                                newSelection = this->view.FindVertex(worldPosition);

                                if(newSelection == GraphView::NoId) {
                                    if(this->selectedId != GraphView::NoId && (this->selectionType & SelectionType::VertexSelected) == SelectionType::VertexSelected) {
                                        break;
                                    }

                                    newSelection = this->view.FindEdge(worldPosition);

                                    this->selectedId = newSelection;
                                    this->selectionType = SelectionType::EdgeSelected;
                                    break;
                                }
                            }

                            if(this->selectedId == GraphView::NoId) {
                                this->selectedId = newSelection;
                                this->selectionType = SelectionType::VertexDrawingEdge;
                                this->renderer.UpdateVertexColor(this->selectedId, 0x00FF00FF);
                                break;
                            }

                            this->renderer.UpdateVertexColor(this->selectedId, 0xFFFFFFFF);
                            this->view.AddEdge(this->selectedId, newSelection); 
                            this->renderer.AddEdge(this->view.GetGraph().Edges().back());
                            this->selectionType = SelectionType::None;
                            this->selectedId = GraphView::NoId;
                            break;
                        }
                    }
                    break;
                }
                case SAPP_MOUSEBUTTON_RIGHT: {
                    f32x2 worldPosition = this->renderer.ScreenToWorld({event->mouse_x, event->mouse_y});

                    switch (this->mode) {
                        case GraphexiaMode::EditVertices: {
                            if((this->selectionType & SelectionType::VertexSelected) == SelectionType::VertexSelected && this->selectedId != GraphView::NoId) {
                                const Vertex& vView = this->view.Views()[this->selectedId];
                                
                                if(vView.Collides(worldPosition)) {
                                    if((this->selectionType & SelectionType::DeletionRequest) != SelectionType::DeletionRequest) {
                                        this->selectionType = SelectionType::VertexDeletionRequest;
                                        this->renderer.UpdateVertexColor(this->selectedId, 0xFF0000FF);
                                        break;
                                    }

                                    this->view.EraseVertex(this->selectedId);
                                    this->renderer.ReconstructEdges(this->view.GetGraph().Edges());
                                    this->renderer.DeleteVertex(this->selectedId);
                                    this->selectionType = SelectionType::None;
                                    this->selectedId = GraphView::NoId;
                                }
                            }
                            break;
                        }
                        case GraphexiaMode::EditEdges: {
                            if(this->selectedId != GraphView::NoId) {
                                this->renderer.UpdateVertexColor(this->selectedId, 0xFFFFFFFF);
                            }
                            this->selectedId = GraphView::NoId;
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
                f32x2 cameraPosition = this->renderer.GetCameraPosition();
                cameraPosition.x += event->mouse_dx;
                cameraPosition.y += event->mouse_dy;
                this->renderer.SetCameraPosition(cameraPosition);
                break;
            }
            
            this->currentMouseWorldPosition = this->renderer.ScreenToWorld({event->mouse_x, event->mouse_y});
            switch (this->mode) {
                case GraphexiaMode::EditVertices: {
                    if(this->currentlyDraggingVertex) {
                        f32x2 newPosition = {(this->currentMouseWorldPosition.x + this->selectedVertexMouseOffset.x), (this->currentMouseWorldPosition.y + this->selectedVertexMouseOffset.y)};
                        this->view.MoveVertex(this->selectedId, newPosition); 

                        this->renderer.UpdateVertexPosition(this->selectedId, newPosition);
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

void Graphexia::ChangeMode(const GraphexiaMode mode) {
    if(this->mode == mode)
        return;

    this->mode = mode;
    this->selectedId = GraphView::NoId;
}
