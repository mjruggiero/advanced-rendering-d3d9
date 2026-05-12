// Constant registers 
// c8 - c11 world * view * proj
//
// Input registers
// dcl_position v0
// dcl_texcoord v7


float4x4 matWorldViewProj	:register(c8);	
float4x4 matLightViewProj		:register(c16);	
float4x4 matLightViewProjTexAdj	:register(c20);	
float4 fFarNear					:register(c34);


// -------------------------------------------------------------
// Output channels
// -------------------------------------------------------------
struct VS_OUTPUT
{
    float4 Pos  : POSITION;
    float2 Tex : TEXCOORD0;
   	float4 ShadowMapUV	: TEXCOORD1;
	float Depth			: TEXCOORD2;
};



VS_OUTPUT VS(float4 Pos : POSITION : register(v0), 
			float3 Tex : TEXCOORD0 : register(v7))						
{
    VS_OUTPUT Out = (VS_OUTPUT)0;      
    Out.Pos = mul(Pos, matWorldViewProj);	// transform Position

    Out.Tex = Tex.xy;

	// transform projected textures
	// == mLightViewProj * texture adjustment matrix
	// mScale * mTransform * mRotate * mView * mProj * mScaleBias
	Out.ShadowMapUV = mul(Pos, matLightViewProjTexAdj);
	
	// measure distance between light and point
	// transform light source
	// mScale * mTransform * mRotate * mView * mProj
	float4 Position = mul(Pos, matLightViewProj);
	Out.Depth = -(((Position.z + fFarNear.y) / (fFarNear.x - fFarNear.y)) * Position.w) + 2.079f;
    
   return Out;
}

