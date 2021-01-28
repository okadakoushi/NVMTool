//ランバート拡散反射サンプル00。
//拡散反射光のみを確認するためのサンプルです。


//変えたら変える。
static const int NUM_DIRECTIONAL_LIGHT = 4;	//ディレクションライトの本数。
static const int NUM_SHADOW_MAP = 3;		//シャドウマップの数。

//モデル用の定数バッファ
//変えたらMeshPartsの送ってる処理も変えてね！
cbuffer ModelCb : register(b0){
	float4x4 mWorld;
	float4x4 mView;
	float4x4 mProj;
	int mShadowReciever;
};

//ディレクションライト。
struct DirectionalLight{
	float3 direction;	//ライトの方向。
	float4 color;		//ライトの色。
};
//ライト用の定数バッファ。
cbuffer LightCb : register(b1){
	DirectionalLight directionalLight[NUM_DIRECTIONAL_LIGHT];
	float3 eyePos;					//カメラの視点。
	float specPow;					//スペキュラの絞り。
	float3 ambinentLight;			//環境光。
};
//頂点シェーダーへの入力。
struct SVSIn{
	float4	pos 		: POSITION;		//モデルの頂点座標。
	float3	normal		: NORMAL;		//法線。
	float3	Tangent		: TANGENT;		
	float3	BiNormal	: BINORMAL;
	float2	uv 			: TEXCOORD0;	//UV座標。
	uint4	Indices		: BLENDINDICES0;//インデックスのサイズ。
	float4	Weights		: BLENDWEIGHT0;	//重み。
};
//スキンなし頂点シェーダー。
struct SVSInNonSkin {
	float4 pos		: POSITION;				//頂点座標。
	float3 normal	: NORMAL;		//法線。
	float2 uv 		: TEXCOORD0;	//uv座標。
};
//ピクセルシェーダーへの入力。
struct SPSIn{
	float4 pos 			: SV_POSITION;	//スクリーン空間でのピクセルの座標。
	float3 normal		: NORMAL;		//法線。
	float2 uv 			: TEXCOORD0;	//uv座標。
	float3 worldPos		: TEXCOORD1;	//ワールド空間でのピクセルの座標。
	float4 posInWorld	: TEXCOORD2;	//ワールド座標。
	float4 posInview	: TEXCOORD3;	//ビュー座標
};

cbuffer ShadowCB : register(b2) {
	float4x4 mLVP[NUM_SHADOW_MAP];						//ライトビュープロジェクション。
	float3 shadowAreaDepthInViewSpace;					//カメラ空間での影を落とすエリアの深度テーブル。
}
//シャドウ用ピクセルシェーダー入力。
struct SPSInShadow {
	float4 pos	: SV_POSITION;	//座標。
};

//変更したらMeshPartsのディスクリプタヒープのレジスタも変更すること。
//モデルテクスチャ。
Texture2D<float4> g_texture : register(t0);	
Texture2D<float4> g_normalMap : register(t1);
Texture2D<float4> g_specularMap : register(t2);
StructuredBuffer<float4x4> boneMatrix : register(t3); //ボーン行列 
Texture2D<float4> ShadowMap0 : register(t4);	//1枚目
Texture2D<float4> ShadowMap1 : register(t5);	//2枚目
Texture2D<float4> ShadowMap2 : register(t6);	//3枚目

//サンプラステート。
sampler g_sampler : register(s0);

/// <summary>
/// モデル用の頂点シェーダーのエントリーポイント。
/// アニメーションサンプル用にスキニングしてる。
/// </summary>
SPSIn VSMain(SVSIn vsIn, uniform bool hasSkin)
{
	SPSIn psIn;
	//スキン行列の計算。
	float4x4 skinning = 0;
	float w = 0.0f;
	for (int i = 0; i < 3; i++) {
		skinning += boneMatrix[vsIn.Indices[i]] * vsIn.Weights[i];
		w += vsIn.Weights[i];
	}
	skinning += boneMatrix[vsIn.Indices[3]] * (1.0f - w);

	psIn.pos = mul(skinning, vsIn.pos);						//モデルの頂点をワールド座標系に変換。
	psIn.posInWorld = psIn.pos;								//ワールド座標を保存しておく。
	psIn.pos = mul(mView, psIn.pos);						//ワールド座標系からカメラ座標系に変換。
	psIn.posInview = psIn.pos;								//ビュー座標を保存しておく。
	psIn.pos = mul(mProj, psIn.pos);						//カメラ座標系からスクリーン座標系に変換。
	psIn.normal = normalize(mul(mWorld, vsIn.normal));		//法線をワールド座標系に変換。
	psIn.uv = vsIn.uv;

	return psIn;
}

/// <summary>
/// モデル用スキンなしシェーダー。
/// </summary>
SPSIn VSMainNonSkin(SVSInNonSkin vsIn)
{
	SPSIn psIn;
	psIn.worldPos = 0;					//なんやこれは！todo: delete
	psIn.pos = mul(mWorld, vsIn.pos);	//１：モデルの頂点をワールド座標系に変換。	
	psIn.posInWorld = psIn.pos;			//ワールド座標。
	psIn.pos = mul(mView, psIn.pos);	//２：ワールド座標系からカメラ座標系に変換。
	psIn.posInview = psIn.pos;			//ビュー座標
	psIn.pos = mul(mProj, psIn.pos);	//３：カメラ座標系からスクリーン座標系に変換。
	psIn.normal = vsIn.normal;
	psIn.uv = vsIn.uv;
	return psIn;
}

int GetCascadeIndex(float zInView)
{
	for (int i = 0; i < NUM_SHADOW_MAP; i++) {
		if (zInView < shadowAreaDepthInViewSpace[i]) {
			return i;
		}
	}
	return 0;
}

float CalcShadowPercent(Texture2D<float4> tex, float2 uv, float depth)
{
	//シャドウマップの深度情報
	float shadow_val = tex.Sample(g_sampler, uv).r;
	//return shadow_val;
	//深度テスト
	if (depth > shadow_val.r + 0.01f) {
		//手前にあるのでシャドウを落とす。
		return 1.0f;
	}
	return 0.0f;	
}

float CalcShadow(float3 wp, float zInView)
{
	//1.0fだった場合シャドウが落ちる。
	float Shadow = 0;
	//シャドウを落とすかどうかの計算。
	if (mShadowReciever == 1) {
		//シャドウマップの番号。
		int MapNum = 0;

		//まず使用するシャドウマップの番号を取得する。
		MapNum = GetCascadeIndex(zInView);

		//モデルの座標をライトのLVPでライトカメラ軸に変換する。
		float4 posInLVP = mul(mLVP[MapNum], float4(wp, 1.0f));
		//ライト座標系での深度値を計算。
		posInLVP.xyz /= posInLVP.w;
		//深度値取得。
		float depth = posInLVP.z;
		//UV座標に変換。
		float2 shadowMapUV = float2(0.5f, -0.5f) * posInLVP.xy + float2(0.5f, 0.5f);
		

		{
			//どのシャドウマップの深度情報をとるのか識別。
			if (MapNum == 0) {
				//0番目の深度情報で計算。
				Shadow = CalcShadowPercent(ShadowMap0, shadowMapUV, depth);
				return Shadow;
			}
			else if (MapNum == 1) {
				//1番目の深度情報で計算。
				Shadow = CalcShadowPercent(ShadowMap1, shadowMapUV, depth);
				return Shadow;
			}
			else if (MapNum == 2) {
				//2番目の深度情報で計算。
				Shadow = CalcShadowPercent(ShadowMap2, shadowMapUV, depth);
				return Shadow;
			}
		}
	}
	return Shadow;

}
/// <summary>
/// モデル用のピクセルシェーダーのエントリーポイント
/// </summary>
float4 PSMain( SPSIn psIn ) : SV_Target0
{
	float3 lig = 0.0f;
	float metaric = g_specularMap.Sample(g_sampler, psIn.uv).a;
	//////////////////////////////////////////////////////
	// 拡散反射を計算
	//////////////////////////////////////////////////////
	{
		for( int i = 0; i < NUM_DIRECTIONAL_LIGHT; i++){
			float NdotL = dot( psIn.normal, -directionalLight[i].direction);	//ライトの逆方向と法線で内積を計算する。
			if( NdotL < 0.0f){	
				//内積の計算結果はマイナスになるので、if文で判定する。
				NdotL = 0.0f;
			}
			float3 diffuse;
			diffuse = directionalLight[i].color.xyz * (1.0f-metaric) * NdotL; //拡散反射光を足し算する。
	//		return float4( diffuse, 1.0f);
	//		//ライトをあてる物体から視点に向かって伸びるベクトルを計算する。
	//		float3 eyeToPixel = eyePos - psIn.worldPos;
	//		eyeToPixel = normalize(eyeToPixel);
	//		
	//		//光の物体に当たって、反射したベクトルを求める。
	//		float3 reflectVector = reflect(directionalLight[i].direction, psIn.normal);
	//		//反射した光が目に飛び込んて来ているかどうかを、内積を使って調べる。
	//		float d = dot(eyeToPixel, reflectVector);
	//		if( d < 0.0f){
	//			d = 0.0f;
	//		}
	//		d = pow(d, specPow) * metaric;
	//		float3 spec = directionalLight[i].color * d * 5.0f;
	//		//スペキュラ反射の光を足し算する。
			lig += diffuse;// + spec;
		}
	}
	
	//////////////////////////////////////////////////////
	// 環境光を計算
	//////////////////////////////////////////////////////
	lig += ambinentLight; //足し算するだけ
	float4 posInLVP = mul(mLVP[0], psIn.posInWorld);
	
	float2 shadowMapUV = float2(0.5f, -0.5f) * posInLVP.xy + float2(0.5f, 0.5f);
	//UV出力。
	//return float4(shadowMapUV, 0.0f, 1.0f);
	
	//Sampler結果出力。
	//return ShadowMap0.Sample(g_sampler, shadowMapUV);

	//影を落とすかどうかの計算をしていく。
	float Shadow = CalcShadow(psIn.posInWorld, psIn.posInview.z);
	//Shadowの値が0.0fなら0.5f, 1.0fなら1.0。
	//GPU処理でのif文削減。
	lig *= lerp(1.0f,0.5f,Shadow);
	//テクスチャカラーをサンプリング。
	float4 texColor = g_texture.Sample(g_sampler, psIn.uv);
	//影を適用させる。
	texColor.xyz *= lig; 
	return float4(texColor.xyz, 1.0f);	
}

/*
	スキンありシャドウマップ生成用の頂点シェーダー。
*/
SPSInShadow VSMain_ShadowMapSkin(SVSIn vsIn)
{
	//どのピクセルシェーダに返すか。
	SPSInShadow psInput = (SPSInShadow)0;

	//スキン行列の計算。
	float4x4 skinning = 0;
	float w = 0.0f;
	float4 pos = 0;
	for (int i = 0; i < 3; i++) {
		skinning += boneMatrix[vsIn.Indices[i]] * vsIn.Weights[i];
		w += vsIn.Weights[i];
	}
	skinning += boneMatrix[vsIn.Indices[3]] * (1.0f - w);

	pos = mul(skinning, vsIn.pos);						//モデルの頂点をワールド座標系に変換。
	pos = mul(mView, pos);						//ワールド座標系からカメラ座標系に変換。
	pos = mul(mProj, pos);						//カメラ座標系からスクリーン座標系に変換。77
	psInput.pos = pos;
	return psInput;
}

/*
	ピクセルシェーダーのエントリ関数。
*/
float4 PSMain_ShadowMap(SPSInShadow In) : SV_Target0
{
	//射影空間でのZ値を返す。
	float z = In.pos.z / In.pos.w;
	return z;
}
