// -------------------------------------------------------------
// Ahsikhmin Reflection Model
// 
// Copyright (c) 2003 - 2004 Wolfgang F. Engel (wolf@gameversity.com)
// All rights reserved.
// -------------------------------------------------------------

// -------------------------------------------------------------
// variables that are provided by the application
// -------------------------------------------------------------
float4x4 matWorldViewProj;
float4x4 matWorld;
float4   MaterialColor;		
float4	 vLightPos;	
float4	 vCamPos;	
float	fIteration;
float4  pixelSize;
float    mx;
float    my;
float    A;
float    kd;
float    ExposureLevel;


texture RenderMap;
sampler RenderMapSampler = sampler_state
{
   Texture = <RenderMap>;
   MinFilter = LINEAR;
   MagFilter = LINEAR;
   MipFilter = LINEAR;   
   AddressU  = Clamp;
   AddressV  = Clamp;
};

texture FullResMap;
sampler FullResMapSampler = sampler_state
{
   Texture = <FullResMap>;
   MinFilter = LINEAR;
   MagFilter = LINEAR;
   MipFilter = LINEAR;   
   AddressU  = Clamp;
   AddressV  = Clamp;
};


// More accurate encoding
#define EXP_BASE    (1.04)
#define EXP_OFFSET  (64.0)
// a^n = b
#define LOG(a, b)   ( log((b)) / log((a)) )


float4 EncodeRGBE8(in float3 rgb)
{
	float4 rgbe;

    // determine the largest color component
	float fMax = max(max(rgb.r, rgb.g), rgb.b);
	
	float fExp = floor( LOG(EXP_BASE, fMax) );

	rgbe.a = clamp( (fExp + EXP_OFFSET) / 256, 0.0, 1.0 );
	rgbe.rgb = rgb / pow(EXP_BASE, rgbe.a * 256 - EXP_OFFSET);

	return rgbe;
}

float3 DecodeRGBE8(in float4 rgbe)
{
    // retrieved the shared exponent
	float fExp = rgbe.a * 256 - EXP_OFFSET;
	float fScaler = pow(EXP_BASE, fExp);

	return (rgbe.rgb * fScaler);
}

// -------------------------------------------------------------
// Output channels
// -------------------------------------------------------------
struct VS_OUTPUT 
{
   float4 Pos      : POSITION;
   float4 Color    : COLOR0;
   float3 Normal   : TEXCOORD0;
   float3 Binormal : TEXCOORD1;
   float3 Tangent  : TEXCOORD2;
   float3 View     : TEXCOORD3;
   float3 Light    : TEXCOORD4;
};

// -------------------------------------------------------------
// vertex shader function (input channels)
// -------------------------------------------------------------
VS_OUTPUT VS (float4 Pos      : POSITION, 
              float3 Normal   : NORMAL,
              float3 Binormal : BINORMAL,
              float3 Tangent  : TANGENT)
{
   VS_OUTPUT Out = (VS_OUTPUT)0;

   Out.Pos = mul(Pos, matWorldViewProj);
   Out.Normal = mul(Normal, matWorld);
   Out.Binormal = mul(Binormal, matWorld);
   Out.Tangent = mul(Tangent, matWorld);
   Out.Color = MaterialColor;

   float4 PosWorld = mul(Pos, matWorld);
   Out.View  = vLightPos.xyz - PosWorld.xyz;
   Out.Light = vCamPos.xyz - PosWorld.xyz;

   return Out;
}

// -------------------------------------------------------------
// Pixel Shader (input channels):output channel
// -------------------------------------------------------------
float4 PS(float4 Color    : COLOR0,
          float3 Normal   : TEXCOORD0,
          float3 Binormal : TEXCOORD1,
          float3 Tangent  : TEXCOORD2,
          float3 View     : TEXCOORD3,
          float3 Light    : TEXCOORD4) : COLOR0
{
	const float PI = 3.141592653589793238462643383279502884197169399375105820974944592f;
	const float RI = 0.7f;

	float ks = 1.0f - kd;

	float3 N = normalize(Normal);
	float3 Bi = normalize(Binormal);
	float3 T = normalize(Tangent);
	float3 V = normalize(View);

	float3 L = normalize(Light);
	float3 H = normalize(L + V);

	float NV = saturate(dot(N, V));
	float NL = saturate(dot(N, L));
	float NH = saturate(dot(N, H));
	float HL = saturate(dot(H, L));
	float T1H = dot(Bi, H);
	float T2H = dot(T, H);

	// Calculate diffuse
	// Rd = 28 * kd (1 - ks) (1 - (1 - N.V)5) * (1 - (1 - N.L)5)
	// 	    23p                       2                  2    
	float Rd = (28 /(23 * PI)) * (1 - pow(1 - (NV / 2), 5.0f)) * (1 - pow(1 - (NL / 2), 5.0f));

	// Calculate specular 
	//                B                               
	// 	 (mx * T1.H^2 + my * T2.H^2) / (1 - (H.N)^2)
	// N.H
	float B = (pow(NH,(mx * T1H * T1H + my * T2H * T2H) / (1 - NH * NH)));   

	//      F
	//RI + (1 - RI)(1 - H.L)^5
	//HL * max(NV, NL)
	float F = (RI + (1 - RI) * pow((1 - HL), 5.0f)) / HL * max(NV, NL);

	//         A               
	// sqrt(mx + 1)(my + 1) 
	// 	   8PI       
	float Rs = A * B * F;

	// Lr = N.L (kd * (1 - ks) * Dc + ks * Sc)
	//   return Color + NL * (kd * (1 - ks) * Rd + ks * Rs);
//	return EncodeRGBE8(NL * (kd * (1 - ks) * Rd + ks * Rs));
	return EncodeRGBE8(Rs);
}



// -------------------------------------------------------------
// Output channels
// -------------------------------------------------------------
struct VS_OUTPUTScaleBuffer
{
    float4 Pos		: POSITION;
	float2 Tex	    : TEXCOORD0;	
};

// -------------------------------------------------------------
// vertex shader function (input channels)
// -------------------------------------------------------------
VS_OUTPUTScaleBuffer VSScaleBuffer(float4 Pos : POSITION)
{
    VS_OUTPUTScaleBuffer Out = (VS_OUTPUTScaleBuffer)0;        
    Out.Pos.xy = Pos.xy + pixelSize.xy;
	Out.Pos.z = 0.5f;
	Out.Pos.w = 1.0f;

	Out.Tex = float2(0.5f, -0.5f) * Pos.xy + 0.5f.xx;

    return Out;
}

// -------------------------------------------------------------
// Pixel Shader (input channels):output channel
// -------------------------------------------------------------
float4 PSScaleBuffer(float2 Tex	: TEXCOORD0) : COLOR			
				
{
	float4 RGBA = float4(DecodeRGBE8(tex2D(RenderMapSampler, Tex)), 1.0f);
    float Luminance = dot(RGBA, float3(0.299f, 0.587f, 0.114f));
	
    return EncodeRGBE8(RGBA * Luminance);
}

// -------------------------------------------------------------
// Output channels
// -------------------------------------------------------------
struct VS_OUTPUTBloom
{
    float4 Pos			: POSITION;
	float2 TopLeft          : TEXCOORD0;
	float2 TopRight          : TEXCOORD1;
	float2 BottomRight          : TEXCOORD2;
	float2 BottomLeft          : TEXCOORD3;
	
};



// -------------------------------------------------------------
// vertex shader function (input channels)
// -------------------------------------------------------------
VS_OUTPUTBloom VSBloom(float4 Pos    : POSITION)
{
    VS_OUTPUTBloom Out = (VS_OUTPUTBloom)0;        
    Out.Pos.xy = Pos.xy + pixelSize.xy;
	Out.Pos.z = 0.5f;
	Out.Pos.w = 1.0f;
	
	float2 Tex = float2(0.5f, -0.5f) * Pos.xy + 0.5f.xx;
	
	float2 halfPixelSize = pixelSize.xy / 2.0f;
	float2 dUV = (pixelSize.xy * fIteration) + halfPixelSize.xy;
	
	// sample top left
	Out.TopLeft = float2(Tex.x - dUV.x, Tex.y + dUV.y); 
	
	// sample top right
	Out.TopRight = float2(Tex.x + dUV.x, Tex.y + dUV.y);
	
	// sample bottom right
	Out.BottomRight = float2(Tex.x + dUV.x, Tex.y - dUV.y);
	
	// sample bottom left
	Out.BottomLeft = float2(Tex.x - dUV.x, Tex.y - dUV.y);

	
    return Out;
}


// -------------------------------------------------------------
// Pixel Shader (input channels):output channel
// -------------------------------------------------------------
float4 PSBloom(float2 TopLeft          : TEXCOORD0,
				float2 TopRight          : TEXCOORD1,
				float2 BottomRight          : TEXCOORD2,
				float2 BottomLeft          : TEXCOORD3) : COLOR0
{
	float3 addedBuffer = 0.0f;
	
	// sample top left
	addedBuffer = DecodeRGBE8(tex2D(RenderMapSampler, TopLeft));
	
	// sample top right
	addedBuffer += DecodeRGBE8(tex2D(RenderMapSampler, TopRight));
	
	// sample bottom right
	addedBuffer += DecodeRGBE8(tex2D(RenderMapSampler, BottomRight));
	
	// sample bottom left
	addedBuffer += DecodeRGBE8(tex2D(RenderMapSampler, BottomLeft));
	
	// average
	addedBuffer *= 0.25f;

	return EncodeRGBE8(addedBuffer);
}

// -------------------------------------------------------------
// Output channels
// -------------------------------------------------------------
struct VS_OUTPUTScreen
{
    float4 Pos			: POSITION;
	float2 Tex          : TEXCOORD0;
};

// -------------------------------------------------------------
// vertex shader function (input channels)
// -------------------------------------------------------------
VS_OUTPUTScreen VSScreen(float4 Pos : POSITION)
{
    VS_OUTPUTScreen Out = (VS_OUTPUTScreen)0;        
    Out.Pos.xy = Pos.xy;// + pixelSize.xy;
	Out.Pos.z = 0.5f;
	Out.Pos.w = 1.0f;
	
	Out.Tex = float2(0.5f, -0.5f) * Pos.xy + 0.5f.xx;
	
    return Out;
}


// -------------------------------------------------------------
// Pixel Shader (input channels):output channel
// -------------------------------------------------------------
float4 PSScreen(float2 Tex : TEXCOORD0) : COLOR0
{
	float3 FullScreenImage = DecodeRGBE8(tex2D(FullResMapSampler, Tex));
	float3 BlurredImage = DecodeRGBE8(tex2D(RenderMapSampler, Tex));
    float4 color;

	color.rgb = lerp(FullScreenImage, BlurredImage, 0.2f);
	color.a = 0;
	
    color *= ExposureLevel; // Apply simple exposure level

    return pow(color, 0.55f); // Apply gamma and return
}



//////////////////////////////////////////////////////////////////////////
technique Ashikhmin
{
    pass P0
    {
        VertexShader = compile vs_2_0 VS();
        PixelShader  = compile ps_2_0 PS();
    }
}

technique ScaleBuffer
{
    pass P0
    {
        VertexShader = compile vs_1_1 VSScaleBuffer();
        PixelShader  = compile ps_2_0 PSScaleBuffer();
    }
}

technique Bloom
{
    pass P0
    {
        VertexShader = compile vs_1_1 VSBloom();
        PixelShader  = compile ps_2_0 PSBloom();
    }
}

technique Screenblit
{
    pass P0
    {
        VertexShader = compile vs_1_1 VSScreen();
        PixelShader  = compile ps_2_0 PSScreen();
    }
}


