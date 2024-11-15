@module Graphexia 

@block UnpackVec4
// unpackUnorm4x8 is not supported on glsl 300 es...
vec4 UnpackVec4(uint v) {
    return vec4(((v & 0xFFu) / 255), (((v >> 8) & 0xFFu) / 255), (((v >> 16) & 0xFFu) / 255), ((v >> 24) / 255));
}
@end

@vs vs
const vec2 QuadVertices[6] = vec2[6](
    vec2(0, 0),
    vec2(0, 1),
    vec2(1, 1),

    vec2(0, 0),
    vec2(1, 1),
    vec2(1, 0)
);

const int QuadUVs[6] = int[6] (0, 2, 3, 0, 3, 1);

const int PixelsPerChr = 2;
const int ChrPerLine = 1024 / PixelsPerChr;

layout(location = 0) out vec4 fColor;
layout(location = 1) out vec2 fTexUV;

@sampler_type BatchDataSmp nonfiltering
layout(binding = 0) uniform sampler BatchDataSmp;
layout(binding = 0) uniform utexture2D ChrBatchDataTex;

layout(binding = 0) uniform GlobalFontData {
    vec4 MVP1;
    vec4 MVP2WithUnused;
};

@include_block UnpackVec4

ivec2 GetChrPosition(int chrId) { return ivec2((chrId % ChrPerLine) * PixelsPerChr, chrId / ChrPerLine); }

void main() {
    int vertexIndex = gl_VertexIndex % 6;
    vec2 chrVertex = QuadVertices[vertexIndex];
    int chrId = gl_VertexIndex / 6;

    ivec2 chrPosition = GetChrPosition(chrId);
    uvec4 chrData = texelFetch(usampler2D(ChrBatchDataTex, BatchDataSmp), chrPosition, 0);
    vec2 position = vec2(uintBitsToFloat(chrData.x), uintBitsToFloat(chrData.y));
    float fontSize = (chrData.z >> 16) / 65535.0;
    uint height = chrData.z & 0xFFFF;
    vec4 color = UnpackVec4(chrData.w);

    uvec4 chrUVs = texelFetch(usampler2D(ChrBatchDataTex, BatchDataSmp), chrPosition + ivec2(1, 0), 0);
    vec2 topLeftUV = unpackHalf2x16(chrUVs[0]);
    vec2 bottomRightUV = unpackHalf2x16(chrUVs[3]);
    vec2 sizeUV = bottomRightUV - topLeftUV;
    float xAspect = sizeUV.x / sizeUV.y;

    vec2 size = vec2(xAspect * height, height);

    mat3x2 mvp = mat3x2(MVP1.xy, MVP1.zw, MVP2WithUnused.xy);
    vec2 finalVertex = mvp * vec3((chrVertex.xy * size * fontSize) + position.xy, 1.0);

    gl_Position = vec4(finalVertex.xy, 0.0, 1.0);
    fColor = color;
    fTexUV = unpackHalf2x16(chrUVs[QuadUVs[vertexIndex]]);
}

@end

@fs fs
layout(location = 0) in vec4 fColor;
layout(location = 1) in vec2 fTexUV;

layout(location = 0) out vec4 oFragColor;

layout(binding = 1) uniform sampler TexSmp;
layout(binding = 1) uniform texture2D FontTex;

void main() {
    #if SOKOL_GLSL
        oFragColor = fColor * texture(sampler2D(FontTex, TexSmp), fTexUV);
    #else
        oFragColor = fColor * texture(sampler2D(FontTex, TexSmp), fTexUV);
    #endif
}

@end

@program BMFont vs fs
