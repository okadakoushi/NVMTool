/// <summary>
/// 低機能レベル。
/// NavMesh用のセル作成のため、位置情報等のみ算出するクラス。
/// レンダラーとしての能力は持ってない。
/// todo:レベル周りのinclude関係がちょっと適当なので直す。
/// </summary>

#pragma once

class MapChip;
class MapChipRender;
class Bone;

#include "TklFile.h"
#include "Noncopyable.h"

/// <summary>
/// オブジェクトデーター。
/// フックできる範囲。
/// </summary>
struct LevelObjectData {
	Vector3 position;		//座標。
	Quaternion rotatatin;	//回転。
	Vector3 scale;			//拡大。
	const wchar_t* name = nullptr;	//名前。
	bool isShadowCaster;	//シャドウキャスター。
	bool isShadowReceiver;	//シャドウレシーバー。
	/// <summary>
	/// 引数で渡したオブジェクト名のオブジェクトか判定する。
	/// </summary>
	/// <param name="objName">調べるオブジェクト名。</param>
	/// <returns>名前が一致したらtrue。</returns>
	bool EqualObjectName(const wchar_t* objName)
	{
		return wcscmp(objName, name) == 0;
	}
};

/// <summary>
/// レベル。
/// </summary>
class Level : Noncopyable
{
private:
public:
	~Level();
	/// <summary>
	/// レベルを初期化。
	/// <para>フックする場合(MapCihpなし)True。別の形でDrawしてね。</para>
	/// <para>フックしないが描画は走らせる(MapChipあり)false。</para>
	/// <para>フックもしないし描画もしない(MapChipなし)nullptr。</para>
	/// </summary>
	/// <remarks>
	/// フック関数内で、オブジェクトを描画するクラスのインスタンスを作成したときに、falseを指定すると
	/// MapChipとクラスのDrawが走り、2重描画になるので、nullptrとfalseは使い分けること。
	/// </remarks>
	/// <param name="filePath">ファイルパス。</param>
	/// <param name="hookFunc">オブジェクト作成時にフックする関数オブジェクト。</param>
	void Init(const char* filePath, std::function<bool(LevelObjectData& obj)> hookFunc = nullptr);
	/// <summary>
	/// レベルオブジェクトデータを取得。
	/// </summary>
	/// <param name="ObjNo">オブジェクト番号。</param>
	/// <returns>オブジェクト。</returns>
	const LevelObjectData& GetLevelObj(int ObjNo) const
	{
		return objData[ObjNo];
	}
	/// <summary>
	/// tklファイルを取得。
	/// </summary>
	/// <returns></returns>
	const TklFile& GetTklFile() const
	{
		return m_tklFile;
	}
private:
	/// <summary>
	/// マップチップレンダーを作成できたら作成 or 描画すべきオブジェクトのインクリメント。
	/// </summary>
	/// <remarks>
	/// 未登録オブジェクト→レンダーラーを作成。
	/// 登録済みの場合→レンダーラーが描画するオブジェクトをインクリメント。
	/// </remarks>
	/// <param name="objData">オブジェクトデーター。</param>
	/// <returns></returns>
	MapChipRender* CteateMapChipRenderOrAddRenderObject(const LevelObjectData& objData);
	/// <summary>
	/// ボーン行列の構築。
	/// </summary>
	void BuildBoneMatrices();
private:
	TklFile m_tklFile;							//tklファイル。
	static const int BONE_MAX;					//ボーンの最大数。
	using BonePtr = std::unique_ptr<Bone>;		//ボーンのポインタ。
	std::vector<BonePtr> m_bones;				//ボーン配列。
	std::unique_ptr<Matrix[]> m_boneMatirxs;	//ボーン行列。
	bool m_isInited = false;					//初期化した？
	LevelObjectData objData[64];				//オブジェクトデーター。
};

