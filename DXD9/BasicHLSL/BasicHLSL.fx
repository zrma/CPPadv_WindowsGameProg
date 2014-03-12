//////////////////////////////////////////////////////////////////////////
// 쉐이더
//////////////////////////////////////////////////////////////////////////

// 맨 위는 전역 변수

float4 g_MaterialAmbientColor;      // Material's ambient color
float4 g_MaterialDiffuseColor;      // Material's diffuse color
int g_nNumLights;

float3 g_LightDir[3];               // Light's direction in world space
float4 g_LightDiffuse[3];           // Light's diffuse color
float4 g_LightAmbient;              // Light's ambient color

texture g_MeshTexture;              // Color texture for mesh

float    g_fTime;                   // App's time in seconds
float4x4 g_mWorld;                  // World matrix for object
float4x4 g_mWorldViewProjection;    // World * View * Projection matrix



//--------------------------------------------------------------------------------------
// Texture 샘플러
//
// 텍스쳐 매핑 규칙을 정해준다.
//--------------------------------------------------------------------------------------
sampler MeshTextureSampler = 
sampler_state
{
    Texture = <g_MeshTexture>;
    MipFilter = LINEAR;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
	// 선형 보간
};


// CUSTOM_VERTEX 역할을 하는 구조체
// 좌표, 색상, UV 텍스쳐 좌표 지정
struct VS_OUTPUT
{
    float4 Position   : POSITION;   // vertex position 
    float4 Diffuse    : COLOR0;     // vertex diffuse color (note that COLOR0 is clamped from 0..1)
    float2 TextureUV  : TEXCOORD0;  // vertex texture coords 
};


//--------------------------------------------------------------------------------------
// 버텍스 실제 함수
//--------------------------------------------------------------------------------------
VS_OUTPUT RenderSceneVS( float4 vPos : POSITION, float3 vNormal : NORMAL, float2 vTexCoord0 : TEXCOORD0,
                         uniform int nNumLights, uniform bool bTexture, uniform bool bAnimate )
{
    VS_OUTPUT Output;
    float3 vNormalWorldSpace;
    float4 vAnimatedPos = vPos;
    
    // 움직임 플래그가 true일 경우
	// C++ 진영에서 받아온 g_fTime 시간을 갖고 계산 함
	if ( bAnimate )
	{
		vAnimatedPos += float4( vNormal, 0 ) * ( sin( g_fTime + 5.5 ) + 0.5 ) * 5;
	}

	// 시간 주고, 행렬 주고(그러면 락은 안 걸어도 됨),
	// 쉐이더에게 계산을 시키는 과정으로 성능 향상을 꾀함
    
    // 쉐이더 쪽에서 결과물 행렬과 현재 움직인 결과물 좌표를 곱셈함
    Output.Position = mul(vAnimatedPos, g_mWorldViewProjection);
    
    // 월드 좌표값 노멀라이즈 함
    vNormalWorldSpace = normalize(mul(vNormal, (float3x3)g_mWorld));
    
    // 조명 초기화
    float3 vTotalLightDiffuse = float3(0,0,0);

	for ( int i = 0; i < nNumLights; i++ )
	{
		vTotalLightDiffuse += g_LightDiffuse[i] * max( 0, dot( vNormalWorldSpace, g_LightDir[i] ) );
		// 빛 밝기 셋팅 연산
	}
        
    Output.Diffuse.rgb = g_MaterialDiffuseColor * vTotalLightDiffuse + g_MaterialAmbientColor * g_LightAmbient;   
    Output.Diffuse.a = 1.0f; 
    
    // 텍스쳐가 있으면 UV 좌표 적용, 아니면 0으로
	if ( bTexture )
	{
		Output.TextureUV = vTexCoord0;
	}
	else
	{
		Output.TextureUV = 0;
	}
    
    return Output;    
}


//--------------------------------------------------------------------------------------
// 픽셀 구조체
//--------------------------------------------------------------------------------------
struct PS_OUTPUT
{
    float4 RGBColor : COLOR0;  // Pixel color    
};


//--------------------------------------------------------------------------------------
// 픽셀 실제 함수
//--------------------------------------------------------------------------------------
PS_OUTPUT RenderScenePS( VS_OUTPUT In, uniform bool bTexture ) 
{ 
    PS_OUTPUT Output;

    // 텍스쳐가 있으면 앞서 버텍스 쉐이더에서 계산한 Diffuse 값을
	// 샘플러 적용 되어 생성 된 텍스쳐 값과 곱셈
	if ( bTexture )
	{
		Output.RGBColor = tex2D( MeshTextureSampler, In.TextureUV ) * In.Diffuse;
	}
	else
	{
		Output.RGBColor = In.Diffuse;
	}

    return Output;
}


// 테크닉 정의
technique RenderSceneWithTexture1Light
{
    pass P0
    {          
        VertexShader = compile vs_2_0 RenderSceneVS( 1, true, true );
        PixelShader  = compile ps_2_0 RenderScenePS( true );
    }
}

technique RenderSceneWithTexture2Light
{
    pass P0
    {          
        VertexShader = compile vs_2_0 RenderSceneVS( 2, true, true );
        PixelShader  = compile ps_2_0 RenderScenePS( true );
    }
}

technique RenderSceneWithTexture3Light
{
    pass P0
    {          
        VertexShader = compile vs_2_0 RenderSceneVS( 3, true, true );
        PixelShader  = compile ps_2_0 RenderScenePS( true );
    }
}

technique RenderSceneNoTexture
{
    pass P0
    {          
        VertexShader = compile vs_2_0 RenderSceneVS( 1, false, false );
        PixelShader  = compile ps_2_0 RenderScenePS( false );
    }
}
