@module graphexia 

@vs vs
layout(location=0) in vec4 coord;
layout(location=1) in vec4 color;
layout(location=0) out vec2 texUV;
layout(location=1) out vec4 iColor;
void main() {
    gl_Position = vec4(coord.xy, 0.0, 1.0);
    texUV = coord.zw;
    iColor = color;
}
@end

@fs fs
layout(location=0) in vec2 texUV;
layout(location=1) in vec4 iColor;
layout(location=0) out vec4 fragColor;

void main() {
    vec2 uv = texUV * 2.0 - vec2(1.0);
    float dst = 1.0 - dot(uv, uv);
    float t = smoothstep(0.0, fwidth(dst), dst);

    fragColor = iColor * t;
}

@end

@program graphvtx vs fs
