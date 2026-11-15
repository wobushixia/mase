struct VertexIn {
  float3 pos;
}

struct VertexOutput {
  float3 color;
  float4 sv_position : SV_Position;
};

[shader("vertex")]
VertexOutput vertMain(VertexIn in) {
  VertexOutput output;
  output.sv_position = float4(in.pos, 1.0);
  output.color = float3(0.223, 0.772, 0.733);
  return output;
}

[shader("fragment")]
float4 fragMain(VertexOutput inVert) : SV_Target {
  float3 color = inVert.color;
  return float4(color, 1.0);
}

