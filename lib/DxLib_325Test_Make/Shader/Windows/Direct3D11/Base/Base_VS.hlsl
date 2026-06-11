#include "../VertexShader.h"

// 頂点シェーダーの入力
struct VS_INPUT
{
	float4 Position        : POSITION ;			// 座標( ローカル空間 )
	float4 Diffuse         : COLOR0 ;			// ディフューズカラー
	float4 Specular        : COLOR1 ;			// スペキュラカラー
	float2 TexCoords0      : TEXCOORD0 ;		// テクスチャ座標
	float2 TexCoords1      : TEXCOORD1 ;
	float2 TexCoords2      : TEXCOORD2 ;
} ;

// 頂点シェーダーの出力
struct VS_OUTPUT
{
	float4 Position        : POSITION ;
	float4 Diffuse         : COLOR0 ;
	float4 Specular        : COLOR1 ;
	float2 TexCoords0      : TEXCOORD0 ;
	float2 TexCoords1      : TEXCOORD1 ;
	float2 TexCoords2      : TEXCOORD2 ;
} ;

// 2D用
VS_OUTPUT VS2D_Normal( VS_INPUT VSInput )
{
	VS_OUTPUT VSOutput ;
	float4 Position;
	
	// 座標変換
	Position   = VSInput.Position;
	Position.w = 1.0f;
	VSOutput.Position.x = dot( Position, g_ProjectionMatrix[ 0 ] ) ;
	VSOutput.Position.y = dot( Position, g_ProjectionMatrix[ 1 ] ) ;
	VSOutput.Position.z = dot( Position, g_ProjectionMatrix[ 2 ] ) ;
	VSOutput.Position.w = dot( Position, g_ProjectionMatrix[ 3 ] ) ;

	// パラメータセット
	VSOutput.Diffuse    = VSInput.Diffuse ;
	VSOutput.Specular   = VSInput.Specular ;
	VSOutput.TexCoords0 = VSInput.TexCoords0 ;
	VSOutput.TexCoords1 = VSInput.TexCoords1 ;
	VSOutput.TexCoords2 = VSInput.TexCoords2 ;
	
	return VSOutput ;
}

// 3D用
VS_OUTPUT VS3D_Normal( VS_INPUT VSInput )
{
	VS_OUTPUT VSOutput ;
	float4 WorldPosition;
	float4 ViewPosition;
	
	// 座標変換
	WorldPosition.x = dot( VSInput.Position, g_LocalWorldMatrix[ 0 ] ) ;
	WorldPosition.y = dot( VSInput.Position, g_LocalWorldMatrix[ 1 ] ) ;
	WorldPosition.z = dot( VSInput.Position, g_LocalWorldMatrix[ 2 ] ) ;
	WorldPosition.w = 1.0f;
	
	ViewPosition.x = dot( WorldPosition, g_ViewMatrix[ 0 ] ) ;
	ViewPosition.y = dot( WorldPosition, g_ViewMatrix[ 1 ] ) ;
	ViewPosition.z = dot( WorldPosition, g_ViewMatrix[ 2 ] ) ;
	ViewPosition.w = 1.0f;
	
	VSOutput.Position.x = dot( ViewPosition, g_ProjectionMatrix[ 0 ] ) ;
	VSOutput.Position.y = dot( ViewPosition, g_ProjectionMatrix[ 1 ] ) ;
	VSOutput.Position.z = dot( ViewPosition, g_ProjectionMatrix[ 2 ] ) ;
	VSOutput.Position.w = dot( ViewPosition, g_ProjectionMatrix[ 3 ] ) ;

	// パラメータセット
	VSOutput.Diffuse    = VSInput.Diffuse ;
	VSOutput.Specular   = VSInput.Specular ;
	VSOutput.TexCoords0 = VSInput.TexCoords0 ;
	VSOutput.TexCoords1 = VSInput.TexCoords1 ;
	VSOutput.TexCoords2 = VSInput.TexCoords2 ;
	
	return VSOutput ;
}
