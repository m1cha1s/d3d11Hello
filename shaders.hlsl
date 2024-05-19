struct vs_in {
    float3 pos : POS;
};

struct vs_out {
    float4 screenPos : SV_POSITION;
    float4 uv : LOLIMASEMANTIC;
};

vs_out vs_main(vs_in input) {
    vs_out output = (vs_out)0;
    output.screenPos = float4(input.pos, 1.0);
    output.uv = float4(input.pos, 1.0);
    return output;
}

// Returns RGBA
float4 ps_main(vs_out input) : SV_TARGET {
    // return float4( 1.0, 0.0, 1.0, 1.0 );
    float u = (1.0+input.uv.x)/2.0;
    float v = (1.0+input.uv.y)/2.0;
    
    return float4(
        u, 
        v, 
        // 0.1*u+0.7*v, 
        0.0,
        1.0 
    );
    // return input.uv;
}