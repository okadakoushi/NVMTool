/// <summary>
/// NavMesh作成用ツール。
/// </summary>

#include "stdafx.h"
#include <iostream>
#include "tkFile/tkmFile.h"
#include "level/TklFile.h"
#include "level/Level.h"

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
	Vector3 pos[3];
	Cell* linkCell[3];
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
				//頂点データを保存する。
				fwrite(&cell.pos[0], sizeof(cell.pos[0]), 1, fp);
				fwrite(&cell.pos[1], sizeof(cell.pos[1]), 1, fp);
				fwrite(&cell.pos[2], sizeof(cell.pos[2]), 1, fp);
				//隣接セルのファイル内相対アドレスを記録していく。
				for (int i = 0; i < 3; i++) {
					std::intptr_t address;
					if (cell.linkCell[i]) {
						//隣接セルがあるので、ファイル内相対アドレスを書き込む。
						address = (std::intptr_t)(&cell.linkCell[i]);
						address -= topAddress;
						fwrite(&address, sizeof(address), 1, fp);

					}
					else {
						//隣接セルがない。
						address = UINT64_MAX;
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
	aabb.v[EnFupperLeft] = { vMin.x, vMin.y, vMax.z };
	aabb.v[EnFlowerLeft] = { vMin.x, vMin.y, vMin.z };
	aabb.v[EnFupperRight] = { vMax.x, vMin.y, vMax.z };
	aabb.v[EnFlowerRight] = { vMax.x, vMin.y, vMin.z };
	aabb.v[EnBupperLeft] = { vMin.x, vMax.y, vMax.z };
	aabb.v[EnBlowerLeft] = { vMin.x, vMax.y, vMin.z };
	aabb.v[EnBupperRight] = { vMax.x, vMax.y, vMax.z };
	aabb.v[EnBlowerRight] = { vMax.x, vMax.y, vMin.z };
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
	Vector3 v0 = cell.pos[0];
	Vector3 v1 = cell.pos[1];
	Vector3 v2 = cell.pos[2];
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
			Vector3 v1v2Vec = epoint - spoint;
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
			//P1
			Vector3 P1toP2 = v2 - v1;
			Vector3 P1toH = hitPos - v1;
			P1toP2.Cross(P0toH);
			//P2
			Vector3 P2toP0 = v0 - v2;
			Vector3 P2toH = hitPos - v2;
			P2toP0.Cross(P0toH);
			//内積を計算。
			float P0toP1Dot = P0toP1.Dot(P1toP2);
			float P0toP2Dot = P0toP1.Dot(P2toP0);
			//衝突判定。
			if (P0toP1Dot * P0toP2Dot > 0) {
				//衝突してた。
				return true;
			}
		}
	}

	return false;

	//平面の方程式のd(法線の長さ？)も求める。
	/*float d = nom.Length();
	//nom.Normalize();

	//平面上の点Pを求める。あってるかなぁ？
	Vector3 P = { nom.x * d, nom.y * d, nom.z * d };

	//PA,PBを求めていく。
	Vector3 PA, PB;
	PA = P - spoint;
	PB = P - epoint;

	//PA PB それぞれの平面法線と内積
	float dot_PA, dot_PB;
	dot_PA = PA.Dot(nom);
	dot_PB = PB.Dot(nom);

	//誤差調整。todo:調整しなおす。
	if (abs(dot_PA) < 0.000001) { dot_PA = 0.0; }
	if (abs(dot_PB) < 0.000001) { dot_PB = 0.0; }

	//交差を判定していく。
	if (dot_PA == 0.0f && dot_PB == 0.0f) {
		//両端が平面上なので交点計算不可能。
		return false;
	}
	else {
		if ((dot_PA >= 0.0f && dot_PB <= 0.0f) ||
			(dot_PA <= 0.0f && dot_PB >= 0.0f)) 
		{
			//内積片方が正,片方が負なので交差している。
			return true;
		}
		else {
			//交差していない。
			return false;
		}
	}*/
}

void hantei(Vector3& vMax, Vector3& vMin, AABB& aabb, NaviMesh& naviMesh, int cellCount) 
{
	printf("メッシュ番号は%dです。\n", cellCount);
	//ここから当たり判定が抜けないように、AABB内部に規則的な線分を飛ばして
	//当たり判定を抜けないようにする。
	//AABBの辺からとばす頂点の間隔。
	const int stride = 5;
	//各軸の線分を飛ばす数。
	int xCount = vMax.x - vMin.x / stride;
	int yCount = vMax.y - vMin.y / stride;	//こいつZ
	int zCount = vMax.z - vMin.z / stride;	//こいつY

	//基点とする頂点。
	const Vector3 baseVertex = aabb.v[EnFlowerLeft];
	//基盤のXYZ軸の頂点座標を求める。
	//基盤頂点座標リスト。普通の配列でやると、サイズ確保量が決め打ちになって気持ち悪いのでvectorでやろう。
	vector<float> Base_xValue, Base_yValue, Base_zValue;
	////初期化しとく。
	//Base_xValue.resize(xCount);
	//Base_yValue.resize(xCount);
	//Base_zValue.resize(xCount);
	for (int vX = 0; vX < xCount; vX++) {
		float X = baseVertex.x + vX * stride;
		Base_xValue.push_back(X);
	}
	for (int vY = 0; vY < yCount; vY++) {
		float Y = baseVertex.y + vY * stride;
		Base_yValue.push_back(Y);
	}
	for (int vZ = 0; vZ < zCount; vZ++) {
		float Z = baseVertex.z + vZ * stride;
		Base_zValue.push_back(Z);
	}
	//始点と終点。
	Vector3 Start, End;
	End = baseVertex;
	//全線分計算。
	for (int y = 0; y < yCount; y++) {
		for (int x = 0; x < xCount; x++) {
			for (int z = 0; z < zCount; z++) {
				//始点は前の終点。
				Start = End;
				End = { Base_xValue[x] * stride, Base_yValue[y] * stride , Base_zValue[z] * stride };
				bool CD = IntersectPlaneAndLine(Start, End, naviMesh.m_cell[cellCount]);
				if (CD == true) {
					printf("削除おおおおおおおおおお！！\n");
					naviMesh.m_cell.erase(naviMesh.m_cell.begin() + cellCount);
					return;
				}
			}
		}
	}
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
		std::cout << "tkmFilePath・・・ナビゲーションメッシュの生成元のtkファイル\n";
		std::cout << "nvmFilePath・・・生成したナビゲーションメッシュのファイルパス\n";
		return 0;
	}

	//レベル初期化。tklもロードされてる。
	Level level;
	level.Init(argv[1]);
	//ナビメッシュ
	NaviMesh naviMesh;

	int eraseCount = 0;

	//tklファイルの情報をもとにtkmファイルを読み込む。
	for (int i = 1; i < level.GetTklFile().GetObjectCount(); i++) {
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
		Matrix mTrans, mRot, mScale;
		mTrans.MakeTranslation(levelPos);
		mRot.MakeRotationFromQuaternion(levelRot);
		mScale.MakeScaling(scale);
		//ワールド行列を求める。
		Matrix world = mScale * mRot * mTrans;

		//ここからローカルAABBにワールド行列を乗算していく。
		for (int vCount = 0; vCount < EnRectangular_Num; vCount++) {
			//ワールド座標軸に変換。
			aabb.v[vCount].TransformCoord(world);
		}

		//AABBの線分１２本を求めていく。
		//todo:このコードに救済を。
		//手前左。
		//aabb.line[0] = { aabb.v[EnFupperLeft] ,aabb.v[EnFlowerLeft] };
		////手前上。
		//aabb.line[1] = { aabb.v[EnFupperLeft] ,aabb.v[EnFupperRight] };
		////奥左。
		//aabb.line[2] = { aabb.v[EnFupperLeft] ,aabb.v[EnBupperLeft] };
		////手前下。
		//aabb.line[3] = { aabb.v[EnFlowerLeft] , aabb.v[EnFlowerRight] };
		////手前右。
		//aabb.line[4] = { aabb.v[EnFupperRight] , aabb.v[EnFlowerRight] };
		////奥上。
		//aabb.line[5] = { aabb.v[EnBupperLeft] , aabb.v[EnBupperRight] };
		////奥右
		//aabb.line[6] = { aabb.v[EnFupperRight] , aabb.v[EnBupperRight] };
		////
		//aabb.line[7] = { aabb.v[EnBupperRight] , aabb.v[EnBlowerRight] };
		////
		//aabb.line[8] = { aabb.v[EnFlowerRight] , aabb.v[EnBlowerRight] };

		//aabb.line[9] = { aabb.v[EnFlowerLeft] , aabb.v[EnBlowerLeft] };

		//aabb.line[10] = { aabb.v[EnBlowerLeft] , aabb.v[EnBlowerRight] };

		//aabb.line[11] = { aabb.v[EnBupperLeft] , aabb.v[EnBlowerLeft] };

		//aabb.line[AABB::EnFrontLeft] = { aabb.v[EnFlowerLeft], aabb.v[EnFupperLeft] };
		//aabb.line[AABB::EnFrontLeft] = { aabb.v[EnBlowerLeft], aabb.v[EnBupperLeft] };
		//aabb.line[AABB::EnFrontLeft] = { aabb.v[EnFlowerRight], aabb.v[EnFupperRight] };
		//aabb.line[AABB::EnFrontLeft] = { aabb.v[EnBlowerRight], aabb.v[EnBupperRight] };

		for (int cellCount = naviMesh.m_cell.size(); cellCount > 0; cellCount--) {
			hantei(vMax, vMin, aabb, naviMesh, cellCount);
		}

	//	for (int cellCount = 0; cellCount < naviMesh.m_cell.size(); cellCount++) {
	//		//セルの数分ループ。
	//		for (int lineCount = 0; lineCount < AABB::EnLineCount; lineCount++) {
	//			//線分本数分、衝突判定を行う。
	//			//衝突判定(collision detection)。
	//			bool CD = IntersectPlaneAndLine(aabb.line[lineCount].SPoint, aabb.line[lineCount].EPoint, naviMesh.m_cell[cellCount]);
	//			if (CD == true) {
	//				//衝突してたから、そのセルは削除する。
	//				naviMesh.m_cell.erase(naviMesh.m_cell.begin() + cellCount);
	//				printf("セルが削除されました。セル番号は%dです。\n", cellCount);
	//				eraseCount++;
	//			}
	//		}
	//	}
	//}

		//////////////////////////////////////////////////////////////////////////////
		//①高速にするために・・・
		//②ナビゲーションメッシュをノードとするBVH構築する。
		//③線分と三角形との当たり判定との前に、BVHを利用した大幅な足切りを行う。
		//////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////
		//配置されているオブジェクトとセルの当たり判定を行って、
		//衝突しているセルは除去する。
		//①配置されているオブジェクトのローカルAABBをして8頂点を求める。
		//②ローカルAABBの8頂点にワールド行列を乗算する。
		//③8頂点のエッジ(線分)と三角形との衝突判定を行って、ぶつかっているセルを除去。
		//④線分と三角形の衝突判定は、平面の方程式をなんとかしたらできます。
		//////////////////////////////////////////////////////////////////////////////

	}
	naviMesh.Save(argv[2]);
}