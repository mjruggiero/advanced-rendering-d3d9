// Constant registers 
// c0 - c3 world space transposed
// c8 - c11 world * view * proj
// c12 - Light Direction (In World Space)
// c24 - eye vector
// c33 - four constants with 0.5f
//
// Input registers
// dcl_position v0
// dcl_normal0 v3
// dcl_normal1 v4
// dcl_texcoord v7
// dcl_tangent0 v8
// dcl_tangent1 v9


float4x4 matWorldViewProj	:register(c8);	
float4x4 matWorld			:register(c0);	
float4 LightDir				:register(c12);
float4 vecEye				:register(c24);



// -------------------------------------------------------------
// Output channels
// -------------------------------------------------------------
struct VS_OUTPUT
{
    float4 Pos  : POSITION;
    float2 Tex : TEXCOORD0;
    float3 View : TEXCOORD1;
    float3 Light : TEXCOORD2;
    float3 Half : TEXCOORD3;
};



VS_OUTPUT VS(float4 Pos : POSITION : register(v0), 
			float3 Normal : NORMAL : register(v3),
			float3 Tex : TEXCOORD0 : register(v7),						
			float3 Tangent : TANGENT : register(v8))						
{
    VS_OUTPUT Out = (VS_OUTPUT)0;      
    Out.Pos = mul(Pos, matWorldViewProj);	// transform Position

   	// compute the 3x3 tranform matrix 
    // to transform from world space to tangent space
    float3x3 worldToTangentSpace;
    worldToTangentSpace[0] = mul(Tangent, matWorld);
    worldToTangentSpace[1] = mul(cross(Tangent, Normal), matWorld);
    worldToTangentSpace[2] = mul(Normal, matWorld);
	
    Out.Tex = Tex.xy;

    float3 PosWorld = normalize(mul(Pos, matWorld));

	float3 Light =  -(LightDir - PosWorld);
//	float3 Light =  Pos - LightDir;
    Out.Light.xyz = mul(worldToTangentSpace, Light); 	// L
    float3 Viewer = vecEye - PosWorld;						// V
    Out.View = mul(worldToTangentSpace, Viewer);

    Out.Half = mul(worldToTangentSpace,(Light + Viewer) / 2); // V
    
   return Out;
}

