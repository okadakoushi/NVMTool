/// <summary>
/// NavMesh作成用ツール。
/// </summary>

#include "stdafx.h"
#include <iostream>
#include "tkFile/tkmFile.h"
#include "level/TklFile.h"
#include "level/Level.h"
#include <comdef.h>

//直方体を構成する８頂点。
enum Rectangular {
	//ほんとはEnRectangular~~って書かないといけないけど長い・・・。
	//ツールだからそんなに変数ないし大丈夫かな。todo:EnRectangular??
	//直方体を構成する頂点番号の規則とかあるのかな？
	EnFupperLeft,			//手前左上。
	EnFlowerLeft,			//手前左下。
	EnFupperRight,			//手前右上。
	EnFlowerRight,			//手前右下。
	EnBupperLeft,			//奥左上。
	EnBlowerLeft,			//奥左下。
	EnBupperRight,			//奥右上。
	EnBlowerRight,			//奥右下。
	EnRectangular_Num
};

/// <summary>
/// セル。
/// </summary>
struct Cell {
	int linkCellNumber[3] = { INT_MAX, INT_MAX, INT_MAX };		//隣接セルの番号。
	Vector3 pos[3];						//36byte
	std::int32_t pad;					//4byte
	Cell* linkCell[3] = { nullptr };	//24byte
};
/// <summary>
/// ナビゲーションメッシュ。
/// </summary>
class NaviMesh {
public:
	std::vector<Cell>	m_cell;
	void Save(const char* filePath)
	{
		//ファイルを開く
		FILE* fp = fopen(filePath, "wb");
		if (fp) {
			std::intptr_t topAddress = (std::intptr_t)(&m_cell[0]);
			//セルの数を書き込む。
			int numCell = m_cell.size();
			printf("Saveするセルの数は%dです。", numCell);
			fwrite(&numCell, sizeof(numCell), 1, fp);
			for (auto& cell : m_cell) {
				for (int i = 0; i < 3; i++) {
					//セル番号も書き出し。
					fwrite(&cell.linkCellNumber[i], sizeof(cell.linkCellNumber[i]), 1, fp);
				}
				//頂点データを保存する。
				fwrite(&cell.pos[0], sizeof(cell.pos[0]), 1, fp);
				fwrite(&cell.pos[1], sizeof(cell.pos[1]), 1, fp);
				fwrite(&cell.pos[2], sizeof(cell.pos[2]), 1, fp);
				fwrite(&cell.pad, sizeof(cell.pad), 1, fp);

				//隣接セルのファイル内相対アドレスを記録していく。
				for (int i = 0; i < 3; i++) {
					std::intptr_t address;
					if (cell.linkCell[i]) {
						//隣接セルがあるので、ファイル内相対アドレスを書き込む。
						address = (std::intptr_t)(cell.linkCell[i]);
						address -= topAddress;
						fwrite(&address, sizeof(address), 1, fp);
					}
					else {
						//隣接セルがない。
						address = UINT64_MAX;
						fwrite(&address, sizeof(address), 1, fp);
					}
				}
			}
			fclose(fp);
		}
		
	}
};
/// <summary>
/// 障害物。
/// </summary>
struct Obstacle {
	Vector3 v[EnRectangular_Num];	//8頂点。
	int ObjNo = 0;					//一応オブジェクトナンバー。
};
/// <summary>
/// 線分。
/// </summary>
struct Line {
	Vector3 SPoint;	//始点。
	Vector3 EPoint;	//終点。
};
/// <summary>
/// AABB。
/// </summary>
struct AABB {
	Vector3 v[8];	//AABBを構成する８点。
	//線分の名前。
	enum LineName {
		EnFrontLeft,
		EnBuckLeft,
		EnFrontRight,
		EnBuckRight,
		EnLineCount
	};
	Line line[EnLineCount];	//AABBを構成する線分4本。
};

/// <summary>
/// 一つのメッシュからナビゲーションメッシュのセルを作成。
/// </summary>
/// <returns></returns>
template<class TIndexBuffer> 
void BuildCellsFromOneMesh(
	NaviMesh& naviMesh,
	const TIndexBuffer& indexBuffer, 
	const TkmFile::SMesh& mesh
)
{
	for (auto& indexBuffer : indexBuffer) {
		//3つの頂点で1ポリなので、インデックス数 / 3でポリゴン数。
		int numPolygon = static_cast<int>(indexBuffer.indices.size() / 3);
		for (int i = 0; i < numPolygon; i++) {
			int no_0 = i * 3;
			int no_1 = i * 3 + 1;
			int no_2 = i * 3 + 2;
			//頂点番号を取得。
			int vertNo_0 = indexBuffer.indices[no_0];
			int vertNo_1 = indexBuffer.indices[no_1];
			int vertNo_2 = indexBuffer.indices[no_2];
			//頂点からセルを作成。
			Cell c;
			c.pos[0] = mesh.vertexBuffer[vertNo_0].pos;
			c.pos[1] = mesh.vertexBuffer[vertNo_1].pos;
			c.pos[2] = mesh.vertexBuffer[vertNo_2].pos;
			naviMesh.m_cell.push_back(c);
			//printf("naviMesh作成。作成したセル番号は%d、位置は(%f, %f, %f)です。\n", (unsigned int)naviMesh.m_cell.size(), c.pos[0].x, c.pos[0].y, c.pos[0].z);
			//printf("naviMesh作成。作成したセル番号は%d、位置は(%f, %f, %f)です。\n", (unsigned int)naviMesh.m_cell.size(), c.pos[1].x, c.pos[1].y, c.pos[1].z);
			//printf("naviMesh作成。作成したセル番号は%d、位置は(%f, %f, %f)です。\n", (unsigned int)naviMesh.m_cell.size(), c.pos[2].x, c.pos[2].y, c.pos[2].z);
		}
	}
}

/// <summary>
/// AABBを計算。
/// </summary>
/// <param name="aabb">AABB格納用。</param>
/// <param name="vMax">最大頂点。</param>
/// <param name="vMin">最小頂点。</param>
/// <returns>AABB。</returns>
void CalcAABB(AABB& aabb, Vector3& vMax, Vector3& vMin)
{
	//8頂点を計算＆代入していく。
	//ZUPなことに注意！！
	aabb.v[EnFupperLeft] = { vMin.x, vMax.y, vMin.z };
	aabb.v[EnFlowerLeft] = { vMin.x, vMin.y, vMin.z };
	aabb.v[EnFupperRight] = { vMax.x, vMax.y, vMin.z };
	aabb.v[EnFlowerRight] = { vMax.x, vMin.y, vMin.z };
	aabb.v[EnBupperLeft] = { vMin.x, vMax.y, vMax.z };
	aabb.v[EnBlowerLeft] = { vMin.x, vMin.y, vMax.z };
	aabb.v[EnBupperRight] = { vMax.x, vMax.y, vMax.z };
	aabb.v[EnBlowerRight] = { vMax.x, vMin.y, vMax.z };
}

/// <summary>
/// 線分とセルの衝突判定。
/// </summary>
/// <param name="spoint">線分始点。</param>
/// <param name="epoint">線分終点。</param>
/// <param name="cell">セル。</param>
/// <returns>衝突しているならTrue/していないならFalse。</returns>
bool IntersectPlaneAndLine(
	Vector3& spoint,
	Vector3& epoint,
	Cell& cell
)
{
	//セルの法線を求めていく。
#if 0	 //テスト
	Vector3 v0 = { 0.0f, 0.0f, 0.0f };
	Vector3 v1 = { 20.0f, 0.0f, 0.0f };
	Vector3 v2 = { 0.0f, 20.0f, 0.0f };
#else
	Vector3 v0 = cell.pos[0];//{ cell.pos[0].x, cell.pos[0].z, cell.pos[0].y };
	Vector3 v1 = cell.pos[1];//{ cell.pos[1].x, cell.pos[1].z, cell.pos[1].y };
	Vector3 v2 = cell.pos[2];//{ cell.pos[2].x, cell.pos[2].z, cell.pos[2].y };
#endif
	//v0からv1。
	Vector3 nom = v1 - v0;
	//法線。
	Vector3 v2tov1 = v2 - v1;
	nom.Cross(v2tov1);
	nom.Normalize();

	//セルを含む無限平面と線分の交差判定。
	//三角形のどこか一頂点から線分の終点に伸びる線分。
	Vector3 v2ToEnd = epoint - v2;
	//三角形のどこか一頂点から線分の始点に伸びる線分。
	Vector3 v2ToStart = spoint - v2;
	//法線と内積をとることで正射影ベクトルが取れる。
	float EndDotN = v2ToEnd.Dot(nom);
	float StartDonN= v2ToStart.Dot(nom);
	//2つの射影ベクトルの内積を求める。
	//交差している場合は射影ベクトルが違う方向を向いているはず。
	float v1v2 = EndDotN * StartDonN;
	if (v1v2 < 0.0f) {
		//無限平面と線分が交差している。
		//交点を求める。
		//辺の絶対値求める。
		float EndLen = fabsf(EndDotN);
		float StartLen = fabsf(StartDonN);
		//辺の割合を求める。
		if ((StartLen + EndLen) > 0.0f) {
			float t = EndLen / (StartLen + EndLen);
			//交点を求める。
			Vector3 v1v2Vec = spoint - epoint;
			v1v2Vec *= t;
			Vector3 hitPos = epoint + v1v2Vec;

			//交点がポリゴン内にあるかどうか。
			//各頂点から法線に向かうベクトルと、次の頂点に向かうベクトルとの外積を計算して
			//外積結果から内積をとって正の場合は衝突している。
			//外積の特徴としてv1.Cross(v2)とあった場合にv2がv1の時計回りの位置にあれば正の値。逆は負の値になる。
			//P0
			Vector3 P0toP1 = v1 - v0;
			Vector3 P0toH = hitPos - v0;
			//外積。
			P0toP1.Cross(P0toH);
			P0toP1.Normalize();
			//P1
			Vector3 P1toP2 = v2 - v1;
			Vector3 P1toH = hitPos - v1;
			P1toP2.Cross(P1toH);
			P1toP2.Normalize();
			//P2
			Vector3 P2toP0 = v0 - v2;
			Vector3 P2toH = hitPos - v2;
			P2toP0.Cross(P2toH);
			P2toP0.Normalize();			//内積を計算。
			float P0toP1Dot = P0toP1.Dot(P1toP2);
			float P0toP2Dot = P0toP1.Dot(P2toP0);
			//衝突判定。
			if (P0toP1Dot > 0 && P0toP2Dot > 0) {
				//衝突してた。
				return true;
			}
		}
		else {
			//test
			int i = 0;
		}
	}

	return false;
}

/// <summary>
/// 線分とセルの当たり判定。
/// <para>一線でも当たっていた場合、該当セルを削除してreturn。</para>
/// </summary>
/// <param name="vMax">AABBの最大頂点。</param>
/// <param name="vMin">AABBの最小頂点。</param>
/// <param name="aabb">AABB。</param>
/// <param name="naviMesh">ナビメッシュ。</param>
/// <param name="cellCount">セル番号。</param>
void hantei(Vector3& vMax, Vector3& vMin, AABB& aabb, NaviMesh& naviMesh, int cellCount) 
{
	//printf("メッシュ番号は%dです。\n", cellCount);
	//ここから当たり判定が抜けないように、AABB内部に規則的な線分を飛ばして
	//当たり判定を抜けないようにする。
	//AABBの辺からとばす頂点の間隔。
	const int stride = 5;
	//各軸の線分を飛ばす数。
	int xCount = (vMax.x - vMin.x) / stride;
	int zCount = (vMax.z - vMin.z) / stride;	//こいつZ

	//基点とする頂点。
	const Vector3 baseVertex = aabb.v[EnFupperLeft];

	////基盤のXYZ軸の頂点座標を求める。
	////基盤頂点座標リスト。普通の配列でやると、サイズ確保量が決め打ちになって気持ち悪いのでvectorでやろう。
	vector<float> Base_xValue, Base_zValue;
	for (int vX = 0; vX < xCount; vX++) {
		float X = baseVertex.x + vX * stride;
		Base_xValue.push_back(X);
	}
	for (int vZ = 0; vZ < zCount; vZ++) {
		float Z = baseVertex.z + vZ * stride;
		Base_zValue.push_back(Z);
	}

	
	Vector3 Start, End;

#if 0 //テスト
	Start.x = 10.0f;
	Start.y = 5.0f;
	Start.z = 1000.0f;
	End.x = 10.0f;
	End.y = 0.0f;
	End.z = -1000.0f;
	bool CD = IntersectPlaneAndLine(Start, End, naviMesh.m_cell[cellCount]);
	if (CD == true) {
		//printf("削除おおおおおおおおおお！！\n");
		//IntersectPlaneAndLine(Start, End, naviMesh.m_cell[cellCount]);
		naviMesh.m_cell.erase(naviMesh.m_cell.begin() + cellCount);
		return;
	}
#else
	for (int x = 0; x < xCount; x++) {
		for (int z = 0; z < zCount; z++) {
			//XY平面にある点(始点)から真下に線分を飛ばす。
			Start = { Base_xValue[x],baseVertex.y , Base_zValue[z] };
			End = { Base_xValue[x], baseVertex.y - 1000, Base_zValue[z] };
			bool CD = IntersectPlaneAndLine(Start, End, naviMesh.m_cell[cellCount]);
			if (CD == true) {
				naviMesh.m_cell.erase(naviMesh.m_cell.begin() + cellCount);
				return;
			}
		}
	}
#endif
}
/// <summary>
/// ナビゲーション作成ツール。
/// </summary>
/// <param name="argc">引数の数。</param>
/// <param name="argv">入力ファイル。</param>
/// <returns></returns>
int main(int argc, char* argv[])
{
	if (argc < 2) {
		//引数が足りないのでヘルプを表示する。
		std::cout << "ナビゲーションメッシュ生成ツール\n";
		std::cout << "mknvm.exe tkmFilePath nvmFilePath\n";
		std::cout << "tklFilePath・・・ナビゲーションメッシュの生成元のtklファイル\n";
		std::cout << "nvmFilePath・・・生成したナビゲーションメッシュのファイルパス\n";
		return 0;
	}

	//レベル初期化。tklもロードされてる。
	Level level;
	level.Init(argv[1]);
	//ナビメッシュ
	NaviMesh naviMesh;

	int eraseCount = 0;
	int numMaxObject = level.GetTklFile().GetObjectCount();
	//tklファイルの情報をもとにtkmファイルを読み込む。
	for (int i = 1; i < level.GetTklFile().GetObjectCount(); i++) {
		printf("endObjNo = %d/%d\n", i, numMaxObject);
		//一個目はレベルのルートボーンなので除外。
		//オブジェクト分回す。
		TkmFile tkmFile;
		//ファイルパス形成。
		char filePath[256];
		sprintf_s(filePath, "Assets/modelData/NavSample/%s.tkm", level.GetTklFile().GetObj(i).name.get());
		//tkmのロード。
		//ロードすることで頂点バッファが生成される。
		tkmFile.Load(filePath);

		//メッシュの形成をしていく。
		//メッシュを形成するのはNaviMeshを作成する床とかのみでいいはず。
		//todo:いま強制的に１番目のオブジェクトにNavを作成するようにしてるので直す。
		if (level.GetTklFile().GetObj(i).no == 1) {
			tkmFile.QueryMeshParts([&](const TkmFile::SMesh& mesh) {
				//16ビット版
				//NavMesh作る上では,
				BuildCellsFromOneMesh(naviMesh, mesh.indexBuffer16Array, mesh);
				BuildCellsFromOneMesh(naviMesh, mesh.indexBuffer32Array, mesh);
				});
			for (auto& cell : naviMesh.m_cell) {
				//軸をYUPに変換しておく。
				std::swap(cell.pos[0].y, cell.pos[0].z);
				std::swap(cell.pos[1].y, cell.pos[1].z);
				std::swap(cell.pos[2].y, cell.pos[2].z);
			}
			//NavMeshを作成したオブジェクトとは当たり判定を取る必要はないので処理をスキップ。
			continue;
		}
		//頂点の最大。
		Vector3 vMax = tkmFile.GetMaxVertex();
		//頂点の最小。
		Vector3 vMin = tkmFile.GetMinVertex();
		//ローカルAABBを求める。
		AABB aabb;
		CalcAABB(aabb, vMax, vMin);

		//AABBをワールド座標系にするために、
		//レベルから変換座標を持ってくる。
		Vector3 levelPos = level.GetLevelObj(i).position;
		Quaternion levelRot = level.GetLevelObj(i).rotatatin;
		Vector3 scale = level.GetLevelObj(i).scale;
		//変換座標を行列化していく。
		Matrix mBias, mTrans, mRot, mScale;
		mTrans.MakeTranslation(levelPos);
		mRot.MakeRotationFromQuaternion(levelRot);
		mScale.MakeScaling(scale);
		//ワールド行列を求める。
		Matrix world = mScale * mRot * mTrans;

		vMax = { -FLT_MAX, -FLT_MAX , -FLT_MAX };
		vMin = { FLT_MAX, FLT_MAX , FLT_MAX };
		//ここからローカルAABBにワールド行列を乗算していく。
		for (int vCount = 0; vCount < EnRectangular_Num; vCount++) {
			//ワールド座標軸に変換。
			world.Mul(aabb.v[vCount]);
			vMax.Max(aabb.v[vCount]);
			vMin.Min(aabb.v[vCount]);
		}
		//補正。
		vMax.z *= -1;
		vMin.z *= -1;
		Vector3 min = vMin;
		vMin.z = vMax.z;
		vMax.z = min.z;
		CalcAABB(aabb, vMax, vMin);
		
		//ここから衝突判定。
		//meshすべてのセルとAABBとの当たり判定を取って、
		//セルとAABBが衝突していたら、該当セルを削除。リストは降順から回さないと削除したときにおかしくなるはず。
		for (int cellCount = naviMesh.m_cell.size() - 1; cellCount >= 0; cellCount--) {
			hantei(vMax, vMin, aabb, naviMesh, cellCount);
		}
	}

	//ここからセル1つ1つの隣接セルを求める。
	printf("隣接セル情報の調査を開始。\n");
	for (int serchCell = naviMesh.m_cell.size() - 1; serchCell >= 0; serchCell--) {
		//隣接セル番号。
		int linkNum = 0;
		for (int isSerchedCell = naviMesh.m_cell.size() - 1; isSerchedCell >= 0; isSerchedCell--) {
			if (serchCell == isSerchedCell) {
				//同じセルの場合はスキップ。
				continue;
			}
			//同じ頂点の発見数。
			int findSameVertex = 0;
			//セルを構成する3頂点のうち、2頂点が一緒ならばそれは隣接セル。
			for (int posCount = 0; posCount < 3; posCount++) {
				//3頂点分
				if (naviMesh.m_cell[serchCell].pos[0] == naviMesh.m_cell[isSerchedCell].pos[posCount]) {
					//同じ頂点！！
					findSameVertex++;
				}
				if (naviMesh.m_cell[serchCell].pos[1] == naviMesh.m_cell[isSerchedCell].pos[posCount]) {
					//同じ頂点！！
					findSameVertex++;
				}
				if (naviMesh.m_cell[serchCell].pos[2] == naviMesh.m_cell[isSerchedCell].pos[posCount]) {
					//同じ頂点！！
					findSameVertex++;
				}
			}

			if (findSameVertex == 2) {
				//同じ頂点が2つ見つかったので、こいつは隣接セル。
				naviMesh.m_cell[serchCell].linkCell[linkNum] = &naviMesh.m_cell[isSerchedCell];
				//隣接セル番号も保存しておく。
				naviMesh.m_cell[serchCell].linkCellNumber[linkNum] = isSerchedCell;
				//リンクセル番号を次のやつにしとく。
				linkNum++;
			}


		}
	}
	naviMesh.Save(argv[2]);
}
