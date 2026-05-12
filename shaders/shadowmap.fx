// -------------------------------------------------------------
// Shadow Mapping
// 
// Copyright (c) 2003 - 2004 Wolfgang F. Engel (wolf@shaderx.com)
// All rights reserved.
// -------------------------------------------------------------

// transform object
// mScale * mTransform * mRotate * m_matWorld * m_matView * m_matProj
float4x4 mWorldViewProj;		

// transform light source
// mScale * mTransform * mRotate * mView * mProj
float4x4 mLightViewProj;		

// transform projected textures
// == mLightViewProj * texture adjustment matrix
// mScale * mTransform * mRotate * mView * mProj * mScaleBias
float4x4 mLightViewProjTexAdj;		

// Material color
float4   MaterialColor;	

// Light vector
float4	 vLightDir;	

// near and far plane of the viewing frustum
float fFar, fNear;

// hardcoded for 1k shadow map
const float2 TexelSize = {1.0f/1024.0f, 1.0f/1024.0f};

// -------------------------------------------------------------
// shadow map texture stage states
// we try linear filtering here ... in case of fp textures it 
// is not supported on some graphics hardware
// -------------------------------------------------------------
texture ShadowMap;
sampler ShadowMapSamp = sampler_state
{
    Texture = <ShadowMap>;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = NONE;

    AddressU = Clamp;
    AddressV = Clamp;
};

struct VS_OUTPUTCREATESHADOWMAP
{
    float4 Pos		: POSITION;
	float Depth		: TEXCOORD0;
};

VS_OUTPUTCREATESHADOWMAP VSCreateShadowMap(float4 Pos    : POSITION,
										float3 Normal : NORMAL)
{
    VS_OUTPUTCREATESHADOWMAP Out = (VS_OUTPUTCREATESHADOWMAP)0; 

	// transform light source
	// mScale * mTransform * mRotate * mView * mProj
	float4 Position = mul( Pos, mLightViewProj);	
	
    // output position
    Out.Pos = Position;
    
    // depth value
    Out.Depth = -((Position.z + fNear) / (fFar - fNear)) * Position.w;

//    Out.Depth = Position.z / Position.w;
    

    return Out;
}

// -------------------------------------------------------------
// Pixel shader that creates shadow map
// -------------------------------------------------------------
float4 PSCreateShadowMap(float Depth : TEXCOORD0) : COLOR
{   
	return Depth;
}



struct VS_OUTPUT
{
    float4 Pos			: POSITION;
	float4 Diffuse		: COLOR0;
	float4 Ambient		: COLOR1;
	float4 ShadowMapUV	: TEXCOORD0;
	float Depth			: TEXCOORD1;
};

// -------------------------------------------------------------
// vertex shader that creates scene
// -------------------------------------------------------------
VS_OUTPUT VSScene(float4 Pos    : POSITION,
					float3 Normal : NORMAL)
{
    VS_OUTPUT Out = (VS_OUTPUT)0;        
	
	// transform object
	// mScale * mTransform * mRotate * m_matWorld * m_matView * m_matProj
    Out.Pos = mul(Pos, mWorldViewProj);
    
	// diffuse per-vertex light and ambient
	Out.Diffuse = MaterialColor * saturate(dot(vLightDir, Normal));
	Out.Ambient = MaterialColor * 0.3f;                     
	
	// transform projected textures
	// == mLightViewProj * texture adjustment matrix
	// mScale * mTransform * mRotate * mView * mProj * mScaleBias
	Out.ShadowMapUV = mul(Pos, mLightViewProjTexAdj);
	
	// measure distance between light and point
	// transform light source
	// mScale * mTransform * mRotate * mView * mProj
	float4 Position = mul(Pos, mLightViewProj);
	Out.Depth = -(((Position.z + fNear) / (fFar - fNear)) * Position.w) + 0.079f;
	
//	float4 uv = mul(Pos, mLightViewProj);
//	Out.Depth = uv.z / uv.w - 0.003;
	
		
    return Out;
}

float OffsetLookup(sampler2D map, float4 loc, float2 offset)
{
	float2 TexcoordProj = (loc.xy / loc.w) + TexelSize;

	return tex2D(map, float2(TexcoordProj + (offset * TexelSize)));
}

// -------------------------------------------------------------
// pixel shader that creates scene
// -------------------------------------------------------------
float4 PSScene(VS_OUTPUT In) : COLOR
{   
 //   float4 Color = In.Ambient;
    float4 Color = 0.0f;
    
//	float ShadowMap = tex2Dproj( ShadowMapSamp, In.ShadowMapUV ).x;
	float ShadowMap = OffsetLookup(ShadowMapSamp, In.ShadowMapUV, float2(0.0f, 0.0f));
    
    // value of shadow_map bigger or equal than depth value: return diffuse
    //Color += (In.Depth - ShadowMap < 0.0f ) ? 0.0f : In.Diffuse;
    Color += (In.Depth - ShadowMap < 0.0f ) ? 0.0f : 1.0f;
    
    return Color;
}  

// -------------------------------------------------------------
// Compiler directives
// -------------------------------------------------------------
technique TShader
{
    pass P0
    {
        VertexShader = compile vs_1_1 VSCreateShadowMap();
        PixelShader = compile ps_2_0 PSCreateShadowMap();
    }
    
    pass P1
    {
		Sampler[0] = (ShadowMapSamp);

        VertexShader = compile vs_1_1 VSScene();
        PixelShader  = compile ps_2_0 PSScene();
        
    }
}
