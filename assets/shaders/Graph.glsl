@module Graphexia 

@block QuadVertices
const vec2 QuadVertices[6] = vec2[6](
    vec2(-1, -1),
    vec2(-1, 1),
    vec2(1, 1),

    vec2(-1, -1),
    vec2(1, 1),
    vec2(1, -1)
);
@end

@block UnpackVec4
vec4 UnpackVec4(uint v) {
    return vec4(((v >> 24) / 255), (((v >> 16) & 0xFFu) / 255), (((v >> 8) & 0xFFu) / 255), ((v & 0xFFu) / 255));
}
@end

@block SharedData
@sampler_type BatchDataSmp nonfiltering
layout(binding=0) uniform sampler BatchDataSmp;

// Layout:
// X | Y | Size | Color
layout(binding=0) uniform utexture2D VtxBatchDataTex;

layout(binding=0) uniform GlobalData {
    vec4 MVP1;
    vec4 MVP2WithUnused;
};

const int VtxPerLine = 32;
const int EdgesPerLine = 1024;

ivec2 GetVtxPosition(int vtxId) { return ivec2(vtxId % VtxPerLine, vtxId / VtxPerLine); }
ivec2 GetEdgePosition(int edgeId) { return ivec2(edgeId % EdgesPerLine, edgeId / EdgesPerLine); }
@end

@vs vtxVS
layout(location=0) out vec2 fVtx;
layout(location=1) out vec4 fColor;

@include_block QuadVertices
@include_block SharedData
@include_block UnpackVec4

void main() {
    vec2 vtxVertex = QuadVertices[gl_VertexIndex % 6];
    int vtxId = gl_VertexIndex / 6;

    uvec4 vtxData = texelFetch(usampler2D(VtxBatchDataTex, BatchDataSmp), GetVtxPosition(vtxId), 0);
    vec2 position = vec2(uintBitsToFloat(vtxData.x), uintBitsToFloat(vtxData.y));
    float size = uintBitsToFloat(vtxData.z);
    vec4 color = UnpackVec4(vtxData.w);

    mat3x2 mvp = mat3x2(MVP1.xy, MVP1.zw, MVP2WithUnused.xy);
    vec2 finalVertex = mvp * vec3((vtxVertex.xy * size) + position.xy, 1.0);

    gl_Position = vec4(finalVertex.xy, 0.0, 1.0);
    fVtx = vtxVertex.xy;
    fColor = color;
}
@end

@fs vtxFS
layout(location=0) in vec2 fVtx;
layout(location=1) in vec4 fColor;
layout(location=0) out vec4 oFragColor;

void main() {
    float dst = 1.0 - dot(fVtx, fVtx);
    float t = smoothstep(0.0, fwidth(dst), dst);

    oFragColor = fColor * t;
}

@end

// Reference: https://wwwtyro.net/2019/11/18/instanced-lines.html
@vs edgeVS
layout(location=0) out vec4 fColor;

const vec2 LineVertices[6] = vec2[6](
    vec2(0, -0.5),
    vec2(1, -0.5),
    vec2(1, 0.5),

    vec2(0, -0.5),
    vec2(1, 0.5),
    vec2(0, 0.5)
); 

@include_block SharedData
@include_block UnpackVec4

// Layout:
// VtxFrom | VtxTo | Size | Color
layout(binding=1) uniform utexture2D EdgeBatchDataTex;

void main() {
    int edgeVertexIndex = gl_VertexIndex % 6;

    vec2 rawEdgeVertex = LineVertices[edgeVertexIndex];
    int edgeId = gl_VertexIndex / 6;

    uvec4 edgeData = texelFetch(usampler2D(EdgeBatchDataTex, BatchDataSmp), GetEdgePosition(edgeId), 0);
    float size = uintBitsToFloat(edgeData.z);
    vec4 color = UnpackVec4(edgeData.w);

    uvec4 fromVtxData = texelFetch(usampler2D(VtxBatchDataTex, BatchDataSmp), GetVtxPosition(int(edgeData.x)), 0);
    uvec4 toVtxData = texelFetch(usampler2D(VtxBatchDataTex, BatchDataSmp), GetVtxPosition(int(edgeData.y)), 0);

    vec2 fromVtxPosition = vec2(uintBitsToFloat(fromVtxData.x), uintBitsToFloat(fromVtxData.y));
    vec2 toVtxPosition = vec2(uintBitsToFloat(toVtxData.x), uintBitsToFloat(toVtxData.y));

    vec2 director = toVtxPosition - fromVtxPosition;
    vec2 normal = normalize(vec2(-director.y, director.x));
    
    vec2 edgeVertex = fromVtxPosition + director * rawEdgeVertex.x + normal * size * rawEdgeVertex.y;

    mat3x2 mvp = mat3x2(MVP1.xy, MVP1.zw, MVP2WithUnused.xy);
    vec2 finalVertex = mvp * vec3(edgeVertex, 1.0);

    gl_Position = vec4(finalVertex.xy, 0.0, 1.0);
    fColor = color;
}

@end

@fs edgeFS
layout(location=0) in vec4 fColor;
layout(location=0) out vec4 oFragColor;

void main() { oFragColor = fColor; }
@end

@program GraphVtx vtxVS vtxFS
@program GraphEdge edgeVS edgeFS
