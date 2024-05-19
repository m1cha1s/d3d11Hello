struct vsIn {
    float3 Position : POSITION;
    float2 TextureUV : UV;
};

struct vsOut {
    float2 TextureUV : UV;
    float4 Position  : SV_POSITION;
};

vsOut vsMain(vsIn input) {
    vsOut output = (vsOut)0;
    
    output.Position = float4(input.Position, 1.0);
    output.TextureUV = input.TextureUV;
    
    return output;
}

Texture2D MeshTexture;
SamplerState MeshTextureSampler;

struct psIn {
    float2 TextureUV : UV;
};

// Returns RGBA
float4 psMain(psIn input) : SV_TARGET {
    return MeshTexture.Sample(MeshTextureSampler, input.TextureUV);
}