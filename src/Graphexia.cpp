#include "Graphexia.hpp"
#include "Core.hpp"
#include <Graphexia/GraphTypes.hpp>
#include <Graphexia/Algo/Hakimi.hpp>

#include <sokol/sokol_gfx.h>

#include <optional>
#include <limits>
#include <algorithm>
#include <random>

Graphexia::Graphexia()
    : view(gpx::CreateKComplete(4), CircularGraphViewRenderer({0,0}, 60, 4)), renderer(), mode(GraphexiaMode::EditVertices), selectedId(GraphView::NoId), savedHavelHakimiSequenceLength(), havelHakimiSequence() {
}

void Graphexia::Init() {
    this->renderer.Init({static_cast<u32>(sapp_width()), static_cast<u32>(sapp_height())});
    this->renderer.ReconstructView(this->view);
}

nk_bool FilterSequence(const nk_text_edit*, const nk_rune unicode) {
    return (unicode >= '0' && unicode <= '9') || unicode == ' ';
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

        nk_labelf(ctx, NK_TEXT_LEFT, "Frametime: %.2f | FPS: %.2f", sapp_frame_duration(), 1 / sapp_frame_duration());
        nk_labelf(ctx, NK_TEXT_LEFT, "Camera at %.2f, %.2f", this->renderer.GetCameraPosition().x, this->renderer.GetCameraPosition().y);
        nk_labelf(ctx, NK_TEXT_LEFT, "Zoom: %.2f", this->renderer.GetCameraZoom());

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

        // HACK
        if(nk_button_label(ctx, "Clear Graph Color")) {
            for (usize i = 0; i < graph.Vertices(); ++i) {
                this->renderer.UpdateVertexColor(i, Rgba8(0xFFFFFFFF)); 
            }

            for (usize i = 0; i < graph.Edges().size(); ++i) {
                this->renderer.UpdateEdgeColor(i, Rgba8(0xFFFFFFFF)); 
            }
        }

        nk_layout_row_dynamic(ctx, 14, 1);
        if(nk_tree_push(ctx, NK_TREE_TAB, "K Complete Graphs", NK_MINIMIZED)) {
            nk_property_int(ctx, "K", 1, &this->savedSelectedKComplete, 400, 1, 1);
            nk_property_int(ctx, "Radius", 1, &this->savedSelectedRadius, 2000, 1, 1);

            if(nk_button_label(ctx, "Render K Complete Graph")) {
                this->view = GraphView(gpx::CreateKComplete(this->savedSelectedKComplete), CircularGraphViewRenderer({}, this->savedSelectedRadius, this->savedSelectedKComplete)); 
                this->renderer.ReconstructView(this->view);
                this->selectionType = SelectionType::None;
                this->selectedId = GraphView::NoId;
            }

            nk_tree_pop(ctx);
        }

        nk_layout_row_dynamic(ctx, 14, 1);
        if(nk_tree_push(ctx, NK_TREE_TAB, "Havel Hakimi", NK_MINIMIZED)) {
            nk_edit_string(ctx, NK_EDIT_FIELD, this->savedHavelHakimiSequence, &this->savedHavelHakimiSequenceLength, 256, FilterSequence);
            
            this->havelHakimiSequence.clear();
            usize i = 0;
            while (i < static_cast<u32>(this->savedHavelHakimiSequenceLength)) {
                if(this->savedHavelHakimiSequence[i] == ' ') {
                    ++i;
                    continue;
                }

                const char* start = this->savedHavelHakimiSequence + i;
                char* end;
                this->havelHakimiSequence.push_back(std::strtoull(start, &end, 10));
                i += end - start;
            }


            if(gpx::IsGraphicSequence(this->havelHakimiSequence)) {
                nk_label(ctx, "Is a graphic sequence", NK_TEXT_LEFT);
                nk_label(ctx, "Render mode", NK_TEXT_LEFT);

                nk_layout_row_dynamic(ctx, 30, 2);
                if(nk_option_label(ctx, "Circular", this->renderHakimiRandom == false)) {
                    this->renderHakimiRandom = false;
                }

                if(nk_option_label(ctx, "Random", this->renderHakimiRandom == true)) {
                    this->renderHakimiRandom = true;
                }

                nk_layout_row_dynamic(ctx, 14, 1);
                if(nk_button_label(ctx, "Render")) {
                    if(this->renderHakimiRandom) {
                        this->view = GraphView(gpx::CreateFromGraphicSequence(this->havelHakimiSequence), RandomGraphViewRenderer({}, this->havelHakimiSequence.size() * 10, std::default_random_engine{static_cast<u32>(sapp_frame_count())})); 
                    } else {
                        this->view = GraphView(gpx::CreateFromGraphicSequence(this->havelHakimiSequence), CircularGraphViewRenderer({}, this->havelHakimiSequence.size() * 10, this->havelHakimiSequence.size())); 

                    }
                    this->renderer.ReconstructView(this->view);
                    this->selectionType = SelectionType::None;
                    this->selectedId = GraphView::NoId;
                }
            } else {
                nk_label(ctx, "Is not a graphic sequence", NK_TEXT_LEFT);
            }
            nk_tree_pop(ctx);
        }

        nk_layout_row_dynamic(ctx, 14, 1);
        if(nk_tree_push(ctx, NK_TREE_TAB, "Kruskal", NK_MINIMIZED)) {
            if(nk_button_label(ctx, "Setup")) {
                this->ClearLastSelection();
                this->kruskalState = gpx::SetupKruskal(this->view.GetGraph());
            }

            if(nk_button_label(ctx, "Iterate")) {
                this->ClearLastSelection(); 

                gpx::IterateKruskal(this->view.GetGraph(), this->kruskalState);

                for (const usize edgeId : this->kruskalState.result) {
                    const gpx::Edge& edge = graph.Edges()[edgeId];
                    this->renderer.UpdateVertexColor(edge.fromId, Rgba8(0x0000FFFF));
                    this->renderer.UpdateVertexColor(edge.toId, Rgba8(0x0000FFFF));
                    this->renderer.UpdateEdgeColor(edgeId, Rgba8(0x0000FFFF)); 
                }
            }

            nk_tree_pop(ctx);
        }

        if(nk_tree_push(ctx, NK_TREE_TAB, "Search", NK_MINIMIZED)) {
            if(nk_button_label(ctx, "Setup BFS")) {
                this->ClearLastSelection();
                this->bfsState = gpx::SetupBFS(this->view.GetGraph(), 0, std::nullopt);
            }

            if(nk_button_label(ctx, "Iterate BFS")) {
                this->ClearLastSelection(); 

                gpx::IterateBFS(this->view.GetGraph(), this->bfsState);

                for (const usize edgeId : this->bfsState.result) {
                    const gpx::Edge& edge = graph.Edges()[edgeId];
                    this->renderer.UpdateVertexColor(edge.fromId, Rgba8(0x0000FFFF));
                    this->renderer.UpdateVertexColor(edge.toId, Rgba8(0x0000FFFF));
                    this->renderer.UpdateEdgeColor(edgeId, Rgba8(0x0000FFFF)); 
                }

                this->renderer.UpdateVertexColor(this->bfsState.visiting[this->bfsState.current - 1], Rgba8(0xFF0000FF));
            }

            nk_spacer(ctx);

            if(nk_button_label(ctx, "Setup DFS")) {
                this->ClearLastSelection();
                this->dfsState = gpx::SetupDFS(this->view.GetGraph(), 0, std::nullopt);
            }

            if(nk_button_label(ctx, "Iterate DFS")) {
                this->ClearLastSelection(); 

                gpx::IterateDFS(this->view.GetGraph(), this->dfsState);

                for (const usize edgeId : this->dfsState.result) {
                    const gpx::Edge& edge = graph.Edges()[edgeId];
                    this->renderer.UpdateVertexColor(edge.fromId, Rgba8(0x0000FFFF));
                    this->renderer.UpdateVertexColor(edge.toId, Rgba8(0x0000FFFF));
                    this->renderer.UpdateEdgeColor(edgeId, Rgba8(0x0000FFFF)); 
                }

                this->renderer.UpdateVertexColor(this->dfsState.last, Rgba8(0xFF0000FF));
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
                        f32& edgeWeight = this->view.EdgeWeight(this->selectedId); 
                        f32 before = edgeWeight;

                        nk_label(ctx, "Edge", NK_TEXT_LEFT);
                        nk_labelf(ctx, NK_TEXT_LEFT, "%zu -> %zu", edge.fromId, edge.toId);
                        nk_property_float(ctx, "Weight", 0, &edgeWeight, std::numeric_limits<f32>::infinity(), 1.f, .1f);

                        if(std::abs(before - edgeWeight) < 0.001) {
                            this->renderer.UpdateWeights();
                        }
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

    this->renderer.SetViewport({static_cast<u32>(sapp_width()), static_cast<u32>(sapp_height())});
    this->renderer.Update(view);
}

void Graphexia::Render() {
    this->renderer.Render();
}

void Graphexia::Event(const sapp_event* event) {
    switch (event->type) {
        case SAPP_EVENTTYPE_MOUSE_SCROLL: {
            f32x2 lastWorldSpaceMouse = this->renderer.ScreenToWorld({event->mouse_x, event->mouse_y});
            bool zoomingIn = event->scroll_y > 0;
            f32 newZoom = std::clamp(this->renderer.GetCameraZoom() * (zoomingIn ? 1.1f : 1/1.1f), .4f, 15.f);

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
                            if(newSelection == GraphView::NoId) {
                                newSelection = this->view.FindVertex(worldPosition);

                                if(this->selectedId == newSelection && newSelection == GraphView::NoId) {
                                    this->AddVertex(worldPosition);
                                }
                            }
                            
                            this->Select(SelectionType::VertexSelected, newSelection);

                            if(this->selectedId != GraphView::NoId) {
                                const Vertex& vertex = this->view.Vertices()[this->selectedId];
                                this->selectedVertexMouseOffset = { vertex.position.x - worldPosition.x, vertex.position.y - worldPosition.y };
                                this->currentlyDraggingVertex = true;
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

                                    this->Select(SelectionType::EdgeSelected, newSelection);
                                    break;
                                }
                            }

                            if(this->selectedId == GraphView::NoId) {
                                this->Select(SelectionType::VertexDrawingEdge, newSelection);
                                break;
                            }

                            this->AddEdge(this->selectedId, newSelection);
                            this->Select(SelectionType::None, GraphView::NoId);
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
                                const Vertex& vView = this->view.Vertices()[this->selectedId];
                                
                                if(vView.Collides(worldPosition)) {
                                    if((this->selectionType & SelectionType::DeletionRequest) != SelectionType::DeletionRequest) {
                                        this->RequestSelectedDeletion();
                                        break;
                                    }

                                    this->EraseVertex(this->selectedId);
                                }
                            }
                            break;
                        }
                        case GraphexiaMode::EditEdges: {
                            if((this->selectionType & SelectionType::VertexSelected) == SelectionType::VertexSelected) {
                                this->Select(SelectionType::None, GraphView::NoId);
                                break;
                            }
                            
                            usize selectedEdge = this->view.FindEdge(worldPosition);
                            
                            if(this->selectedId == selectedEdge && this->selectedId != GraphView::NoId) {
                                if((this->selectionType & SelectionType::DeletionRequest) != SelectionType::DeletionRequest) {
                                    this->RequestSelectedDeletion();
                                    break;
                                }

                                this->EraseEdge(this->selectedId);
                            }
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

void Graphexia::AddVertex(f32x2 position) {
    this->view.AddVertex(position);
    this->renderer.AddVertex(this->view.Vertices().back());
}

void Graphexia::AddEdge(usize from, usize to) {
    this->view.AddEdge(from, to); 
    this->renderer.AddEdge(this->view.GetGraph().Edges().back());
}

void Graphexia::EraseVertex(usize id) {
    this->view.EraseVertex(id);
    this->renderer.EraseVertex(id);
    this->renderer.ReconstructEdges(this->view.GetGraph().Edges());
    this->selectionType = SelectionType::None;
    this->selectedId = GraphView::NoId;
    this->currentlyDraggingVertex = false;
}

void Graphexia::EraseEdge(usize id) {
    this->view.EraseEdge(id);
    this->renderer.EraseEdge(id);
    this->selectionType = SelectionType::None;
    this->selectedId = GraphView::NoId;
}

void Graphexia::Select(const SelectionType type, const usize id) {
    this->ClearLastSelection();
    this->selectionType = type;
    this->selectedId = id;

    if(this->selectedId != GraphView::NoId) {
        if((this->selectionType & SelectionType::EdgeSelected) == SelectionType::EdgeSelected) {
            this->renderer.UpdateEdgeColor(this->selectedId, Rgba8(0x00FF00FF));
        } else if((this->selectionType & SelectionType::VertexSelected) == SelectionType::VertexSelected) {
            this->renderer.UpdateVertexColor(this->selectedId, Rgba8(0x00FF00FF));
        }
    }
}

void Graphexia::RequestSelectedDeletion() {
    this->selectionType = this->selectionType | SelectionType::DeletionRequest;
    
    if((this->selectionType & SelectionType::EdgeSelected) == SelectionType::EdgeSelected) {
        this->renderer.UpdateEdgeColor(this->selectedId, Rgba8(0xFF0000FF));
    } else if((this->selectionType & SelectionType::VertexSelected) == SelectionType::VertexSelected) {
        this->renderer.UpdateVertexColor(this->selectedId, Rgba8(0xFF0000FF));
    }
}

void Graphexia::ClearLastSelection() {
    if(this->selectedId != GraphView::NoId) {
        if((this->selectionType & SelectionType::EdgeSelected) == SelectionType::EdgeSelected) {
            this->renderer.UpdateEdgeColor(this->selectedId, Rgba8(0xFFFFFFFF));
        } else if((this->selectionType & SelectionType::VertexSelected) == SelectionType::VertexSelected) {
            this->renderer.UpdateVertexColor(this->selectedId, Rgba8(0xFFFFFFFF));
        }

        this->selectionType = SelectionType::None;
        this->selectedId = GraphView::NoId;
    }
}

void Graphexia::ChangeMode(const GraphexiaMode mode) {
    if(this->mode == mode)
        return;

    this->ClearLastSelection();
    this->mode = mode;
    this->selectionType = SelectionType::None;
    this->selectedId = GraphView::NoId;
}
