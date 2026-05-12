// Constant registers 
// c0 - c3 world space transposed
// c8 - c11 world * view * proj
// c12 - Light Direction (In World Space)
// c33 - four constants with 0.5f
//
// Input registers
// dcl_position v0
// dcl_normal0 v3
// dcl_normal1 v4
// dcl_texcoord v7
// dcl_tangent0 v8
// dcl_tangent1 v9


float4x4 matWorldViewProj		:register(c8);	
float4x4 matWorld				:register(c0);	
float4 LightDir					:register(c12);
float4x4 matLightViewProj		:register(c16);	
float4x4 matLightViewProjTexAdj	:register(c20);	
float4 fFarNear					:register(c34);

// -------------------------------------------------------------
// Output channels
// -------------------------------------------------------------
struct VS_OUTPUT
{
    float4 Pos  : POSITION;
    float3 Light : COLOR0;
    float2 Tex : TEXCOORD0;
    float2 Tex1 : TEXCOORD1;
   	float4 ShadowMapUV	: TEXCOORD2;
	float Depth			: TEXCOORD3;
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

    float3 PosWorld = normalize(mul(Pos, matWorld));
	
	// transform the light vector with U, V, W
	float3 Light =  -(LightDir - PosWorld);
	
//	float3 Light = Pos - LightDir;
    Out.Light.xyz = ((normalize(mul(worldToTangentSpace, Light))) * 0.5) + 0.5; 	// L
    
    Out.Tex = Tex.xy;
    Out.Tex1 = Tex.xy;    

	// transform projected textures
	// == mLightViewProj * texture adjustment matrix
	// mScale * mTransform * mRotate * mView * mProj * mScaleBias
	Out.ShadowMapUV = mul(Pos, matLightViewProjTexAdj);
	
	// measure distance between light and point
	// transform light source
	// mScale * mTransform * mRotate * mView * mProj
	float4 Position = mul(Pos, matLightViewProj);
	Out.Depth = -(((Position.z + fFarNear.y) / (fFarNear.x - fFarNear.y)) * Position.w) + 5.079f;
    
   return Out;
}

