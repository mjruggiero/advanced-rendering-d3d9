
float4x4 mLightViewProj : register(c16);		
float4 fFarNear : register(c34);

struct VS_OUTPUTCREATESHADOWMAP
{
	float4 Pos		: POSITION;
	float Depth		: TEXCOORD0;
};

VS_OUTPUTCREATESHADOWMAP VS(
	float4 Pos    : POSITION : register(v0),
	float3 Normal : NORMAL : register(v3))
{
	VS_OUTPUTCREATESHADOWMAP Out = (VS_OUTPUTCREATESHADOWMAP)0; 

	// transform light source
	// mScale * mTransform * mRotate * mView * mProj
	float4 Position = mul(Pos, mLightViewProj);	

	// output position
	Out.Pos = Position;

	// depth value
	Out.Depth = -((Position.z + fFarNear.y) / (fFarNear.x - fFarNear.y)) * Position.w;

	return Out;
}

