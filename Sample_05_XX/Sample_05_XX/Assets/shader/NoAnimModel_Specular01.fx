//ランバート拡散反射サンプル00。
//拡散反射光のみを確認するためのサンプルです。



//モデル用の定数バッファ
cbuffer ModelCb : register(b0){
	float4x4 mWorld;
	float4x4 mView;
	float4x4 mProj;
};

//ライト用の定数バッファ。
cbuffer LightCb : register(b1){
	float3 directionalLightDir;		//ディレクションライトの方向。
	float3 directionalLightcolor;	//ディレクションライトのカラー。
	float3 eyePos;					//カメラの視点。
	float specPow;					//スペキュラの絞り。
};

//頂点シェーダーへの入力。
struct SVSIn{
	float4 pos 		: POSITION;		//モデルの頂点座標。
	float3 normal	: NORMAL;		//法線。
	float2 uv 		: TEXCOORD0;	//UV座標。
};
//ピクセルシェーダーへの入力。
struct SPSIn{
	float4 pos 			: SV_POSITION;	//スクリーン空間でのピクセルの座標。
	float3 normal		: NORMAL;		//法線。
	float2 uv 			: TEXCOORD0;	//uv座標。
	float3 worldPos		: TEXCOORD1;	//ワールド空間でのピクセルの座標。
};

//モデルテクスチャ。
Texture2D<float4> g_texture : register(t0);	

//サンプラステート。
sampler g_sampler : register(s0);

/// <summary>
/// モデル用の頂点シェーダーのエントリーポイント。
/// </summary>
SPSIn VSMain(SVSIn vsIn, uniform bool hasSkin)
{
	SPSIn psIn;

	psIn.pos = mul(mWorld, vsIn.pos);			//モデルの頂点をワールド座標系に変換。
	psIn.worldPos = psIn.pos.xyz;
	psIn.pos = mul(mView, psIn.pos);			//ワールド座標系からカメラ座標系に変換。
	psIn.pos = mul(mProj, psIn.pos);			//カメラ座標系からスクリーン座標系に変換。
	psIn.normal = mul(mWorld, vsIn.normal);		//法線をワールド座標系に変換。
	psIn.uv = vsIn.uv;

	return psIn;
}
/// <summary>
/// モデル用のピクセルシェーダーのエントリーポイント
/// </summary>
float4 PSMain( SPSIn psIn ) : SV_Target0
{
	//ライトをあてる物体から視点に向かって伸びるベクトルを計算する。
	float3 eyeToPixel = eyePos - psIn.worldPos;
	eyeToPixel = normalize(eyeToPixel);
	
	//光の物体に当たって、反射したベクトルを求める。
	float3 reflectVector = reflect(directionalLightDir, psIn.normal);
	//反射した光が目に飛び込んて来ているかどうかを、内積を使って調べる。
	float d = dot(eyeToPixel, reflectVector);
	if( d < 0.0f){
		d = 0.0f;
	}
	d = pow(d, specPow);
	float3 lig = directionalLightcolor * d;
	return float4(lig, 1.0f);	//スペキュラ反射光のみを返す。
}
