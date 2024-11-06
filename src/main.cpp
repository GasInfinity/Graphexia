// TODO: Use SDL3 then available

#include <Graphexia/graphview.hpp>
#include <Graphexia/graphmatrix.hpp>
#include <iostream>
#include <raylib.h>
#include <vector>
#include <string>

enum class GraphMode : u8 {
    EditVertices,
    DrawEdges,
};

void RenderGraphView(const gpx::GraphView& graphView) {
    const gpx::Graph& graph = graphView.GetGraph();
    const std::vector<gpx::VertexView>& vertexViews = graphView.Views();
    const std::vector<gpx::Edge>& edges = graph.Edges();

    for (const gpx::Edge& edge : edges) {
        const gpx::VertexView& from = vertexViews[edge.fromId]; 
        const gpx::VertexView& to = vertexViews[edge.toId]; 

        if(edge.fromId != edge.toId) {
            DrawLineEx({static_cast<f32>(from.position.x), static_cast<f32>(from.position.y)}, {static_cast<f32>(to.position.x), static_cast<f32>(to.position.y)}, 1, Color{255,255,255,255});
            continue;
        }

        DrawCircleLines(from.position.x - 3, from.position.y - 3, 7, {255, 255, 255, 255});
    }

    for (const gpx::VertexView& vertex : vertexViews) {
        DrawCircle(vertex.position.x, vertex.position.y, vertex.size, Color{255, 255, 255, 255}); 
    }
}

#include <Graphexia/graphtypes.hpp>

int main(void)
{
    gpx::GraphView view;

    InitWindow(1280, 720, "Graphexia");
    SetTargetFPS(60);

    GraphMode mode = GraphMode::EditVertices;
    usize selectedVertex = gpx::Graph::NoVertex;
    Vector2 movingVertexDifference = {};
    Camera2D camera = {{640, 360}, {}, 0, 1};
    while (!WindowShouldClose())
    {
        Vector2 mDt = GetMouseDelta();

        if(IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)) {
            camera.offset.x += mDt.x;
            camera.offset.y += mDt.y;
        }

        camera.zoom += GetMouseWheelMove() * 0.04;

        if(IsKeyPressed(KEY_R)) {
            view = gpx::GraphView();
            selectedVertex = gpx::Graph::NoVertex;
        } else if(IsKeyPressed(KEY_V)) {
            selectedVertex = gpx::Graph::NoVertex;
            mode = GraphMode::EditVertices;
        } else if(IsKeyPressed(KEY_C)) {
            selectedVertex = gpx::Graph::NoVertex;
            mode = GraphMode::DrawEdges;
        } else if(IsKeyPressed(KEY_A)) {
            usize verticesSize = view.GetGraph().Vertices();
            std::vector<usize> adjacencyMatrix = gpx::AdjacencyMatrix(view.GetGraph());

            std::cout << "Adjacency matrix -----" << std::endl;
            for (usize i = 0; i < verticesSize; ++i) {
                usize currentRow = i * verticesSize;
                for (usize j = 0; j < verticesSize; ++j) {
                    std::cout << adjacencyMatrix[currentRow + j] << ' ';
                } 

                std::cout << '\n';
            }
            std::cout << "-----" << std::endl;
        } else if(IsKeyPressed(KEY_Q)) {
            usize verticesSize = view.GetGraph().Vertices();
            usize edgesSize = view.GetGraph().Edges().size();
            std::vector<gpx::IncidenceState> incidenceMatrix = gpx::IncidenceMatrix(view.GetGraph());

            std::cout << "Incidence matrix -----" << std::endl;
            for (usize i = 0; i < verticesSize; ++i) {
                usize currentRow = i * edgesSize;
                for (usize j = 0; j < edgesSize; ++j) {
                    std::cout << static_cast<i16>(incidenceMatrix[currentRow + j]) << ' ';
                } 

                std::cout << '\n';
            }
            std::cout << "-----" << std::endl;
        } else if(IsKeyPressed(KEY_ONE)) {
            gpx::CircularGraphRenderer renderer = gpx::CircularGraphRenderer({}, 100, 10);
            view = gpx::GraphView(gpx::CreateKComplete(10), renderer);
        } else if(IsKeyPressed(KEY_TWO)) {
            gpx::CircularGraphRenderer renderer = gpx::CircularGraphRenderer({}, 100, 14);
            view = gpx::GraphView(gpx::CreateKComplete(14), renderer);
        } else if(IsKeyPressed(KEY_THREE)) {
            gpx::CircularGraphRenderer renderer = gpx::CircularGraphRenderer({}, 100, 18);
            view = gpx::GraphView(gpx::CreateKComplete(18), renderer);
        } else if(IsKeyPressed(KEY_FOUR)) {
            gpx::CircularGraphRenderer renderer = gpx::CircularGraphRenderer({}, 100, 22);
            view = gpx::GraphView(gpx::CreateKComplete(22), renderer);
        }
        
        Vector2 worldMousePosition = GetScreenToWorld2D(GetMousePosition(), camera);
        switch (mode) {
            case GraphMode::EditVertices: {
                if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    usize newSelected = view.FindVertex({static_cast<i16>(worldMousePosition.x), static_cast<i16>(worldMousePosition.y)});

                    if(newSelected == gpx::Graph::NoVertex && selectedVertex == gpx::Graph::NoVertex) {
                        view.AddVertex({static_cast<i16>(worldMousePosition.x), static_cast<i16>(worldMousePosition.y)});
                    }

                    selectedVertex = newSelected;

                    if(selectedVertex != gpx::Graph::NoVertex) {
                        const gpx::VertexView& vertexView = view.Views()[selectedVertex];

                        movingVertexDifference = {vertexView.position.x - worldMousePosition.x, vertexView.position.y - worldMousePosition.y};
                    }
                } else if(IsMouseButtonDown(MOUSE_BUTTON_LEFT) && selectedVertex != gpx::Graph::NoVertex) {
                    view.MoveVertex(selectedVertex, {static_cast<i16>(worldMousePosition.x + movingVertexDifference.x), static_cast<i16>(worldMousePosition.y + movingVertexDifference.y)});
                }
                break;
            } 
            case GraphMode::DrawEdges: {
                if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    if(selectedVertex == gpx::Graph::NoVertex) {
                        selectedVertex = view.FindVertex({static_cast<i16>(worldMousePosition.x), static_cast<i16>(worldMousePosition.y)}); 
                    } else {
                        usize nextSelectedNode = view.FindVertex({static_cast<i16>(worldMousePosition.x), static_cast<i16>(worldMousePosition.y)});

                        if(nextSelectedNode != gpx::Graph::NoVertex) {
                            view.AddEdge(selectedVertex, nextSelectedNode); 
                            selectedVertex = nextSelectedNode;
                        }
                    }
                } else if(IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) && selectedVertex != gpx::Graph::NoVertex) {
                    selectedVertex = gpx::Graph::NoVertex; 
                }
                break;
            }
        }

        BeginDrawing();
            ClearBackground(BLACK);

            BeginMode2D(camera);
                if(mode == GraphMode::DrawEdges && selectedVertex != gpx::Graph::NoVertex) {
                    const gpx::VertexView& v = view.Views()[selectedVertex];
                    DrawLineEx({static_cast<f32>(v.position.x), static_cast<f32>(v.position.y)}, {static_cast<f32>(worldMousePosition.x), static_cast<f32>(worldMousePosition.y)}, 2.5, Color{255,255,255,255});
                }

                RenderGraphView(view);

                if(mode == GraphMode::EditVertices && selectedVertex != gpx::Graph::NoVertex) {
                    const gpx::VertexView& v = view.Views()[selectedVertex];

                    std::string vertexInfo = "Vertex [" + std::to_string(selectedVertex) + "]: degree [" + std::to_string(view.GetGraph().DegreeOf(selectedVertex)) + "]";
                    DrawText(vertexInfo.c_str(), v.position.x, v.position.y, 8, {200, 200, 200, 255});
                }
            EndMode2D();

            DrawText(mode == GraphMode::EditVertices ? "EditVertices" : "DrawEdges", 0, 0, 14, {0, 255, 0, 255});

            std::string verticesEdges = "Vertices: " + std::to_string(view.GetGraph().Vertices()) + '\n' + "Edges: " + std::to_string(view.GetGraph().Edges().size());
            DrawText(verticesEdges.c_str(), 0, 14, 14, {0, 255, 0, 255});
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
