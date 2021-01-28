#pragma once

#include "Resource/IResource.h"
class TklFile : public IResource
{
public:
	struct SObject {
		std::unique_ptr<char[]> name;	//モデル名。
		int parentNo;					//親の番号。
		float bindPose[4][3];			//バインドポーズ。
		float invBindPose[4][3];		//バインドポーズの逆行列。
		int no;							//オブジェクト番号。
		bool isShadowCaster;			//シャドウキャスターフラグ。
		bool isShadowReceiver;			//シャドウレシーバーフラグ。
		std::vector<int> intDatas;		//intパラメータ。
		std::vector<float> floatDatas;	//floatパラメータ。
		std::vector<std::unique_ptr<char[]>> charsDatas;	//文字列。
		std::vector<Vector3> vec3Datas;	//vec3パラメーター。
	};
	/// <summary>
	/// 読み込み処理。
	/// </summary>
	/// <param name="filePath"></param>
	void LoadImplement(const char* filePath) override final;
	/// <summary>
	/// ボーンに対してクエリを行う。
	/// <para>クエリを行う、オブジェクトとクエリ処理が引数。</para>
	/// <para>引数の関数オブジェクトを読み込んだオブジェクト分呼び出す。</para>
	/// </summary>
	/// <param name="query">クエリ関数。</param>
	void QueryObject(std::function<void(SObject& obj)> query)
	{
		for (auto& obj : m_objects) {
			query(obj);
		}
	}
	/// <summary>
	/// オブジェクトの数を取得。
	/// </summary>
	/// <returns></returns>
	const int& GetObjectCount() const
	{
		return m_numObject;
	}
	/// <summary>
	/// オブジェクトを取得。
	/// </summary>
	/// <param name="i">何番目のオブジェクトか。</param>
	/// <returns>オブジェクト。</returns>
	const SObject& GetObj(int& i) const
	{
		return m_objects[i];
	}
private:
	int m_tklVersion = 100;			//tklファイルのバージョン。
	int m_numObject = 0;			//オブジェクトの数。
	std::vector<SObject> m_objects;	//オブジェクトのリスト。
};

