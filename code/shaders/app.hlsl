#include "shader_header.h"

struct vs_input {
    vec3 P : POSITION0;
};

struct vs_output {
    vec4 P : SV_Position;
};

vs_output VS_Main(vs_input Input) {
    vs_output Result = (vs_output)0;
    Result.P = vec4(Input.P, 1.0f);
    return Result;
}

vec4 PS_Main(vs_output Input) : SV_Target {
    return vec4(1.0f, 1.0f, 0.0f, 0.0f);
}