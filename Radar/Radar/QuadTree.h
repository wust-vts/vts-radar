#ifndef QUADTREE_H
#define QUADTREE_H

#include <vector>
#include<mutex>


/* 一个矩形区域的象限划分：:

UL(0)   |    UR(1)
--------|-----------
LL(2)   |    LR(3)
以下对该象限类型的枚举
*/
typedef enum
{
	UL = 0,
	UR = 1,
	LL = 2,
	LR = 3
}QuadrantEnum;

typedef struct MapRect	//大地坐标
{
	double minX = 0;
	double minY = 0;
	double maxX = 0;
	double maxY = 0;
	MapRect(){};
	MapRect(double minx, double miny, double maxx, double maxy)
	{
		minX = minx;
		minY = miny;
		maxX = maxx;
		maxY = maxy;
	}
	bool equal(const MapRect rect)
	{
		if (rect.minX >= (minX - 1) && rect.minX <= (minX + 1) && rect.minY >= (minY - 1) && rect.minY <= (minY + 1) && rect.maxX >= (maxX - 1)
			&& rect.maxX <= (maxX + 1) && rect.maxY >= (maxY - 1) && rect.maxY <= (maxY + 1))
			return true;
		return false;
	}
}MapRect;

typedef struct FileRect	//新文件在原文件中的位置
{
	int left = 0;
	int right = 0;
	int top = 0;
	int bottom = 0;
	FileRect(){};
	FileRect(int l, int t, int r, int b)
	{
		left = l;
		top = t;
		right = r;
		bottom = b;
	}
}FileRect;

//typedef struct MapRect	//经纬度
//{
//	double left;
//	double top;
//	double right;
//	double bottom;
//}MapRect;

/*空间对象MBR信息*/
//typedef struct SHPMBRInfo
//{
//	int nID;        //空间对象ID号  
//	MapRect Box;    //空间对象MBR范围坐标  
//}SHPMBRInfo;

/* 四叉树节点类型结构 */
typedef struct QuadNode
{
	MapRect     Box;            //节点所代表的矩形区域  
	FileRect	filePos;		//子文件在根文件中的位置范围
	int     nChildCount;        //子节点个数  
	QuadNode  *children[4];     //指向节点的四个孩子   
	CString ID;			//ID号 定位节点位置
	char* fileName;	//该节点所对应文件名（叶节点）
}QuadNode;




class QuadTree
{
//属性
public:
	QuadNode * m_root = NULL;//根节点
	int m_depth = 0;	//四叉树深度
	double m_minX = 0;
	double m_minY = 0;
	double m_maxX = 0;
	double m_maxY = 0;
	int m_row = 0;
	int m_col = 0;
	double m_stepX = 0.0;
	double m_stepY = 0.0;
	int m_minCol = 0;	//最小文件宽度
	int m_minRow = 0;	//最小文件高度
	bool m_isBuildOK = false;//标志树是否构建完成，用于多线程检查
	bool m_isFullTree = false;//是否为满四叉树
	CString indexFilePath;	//用于构造该树的索引文件
//操作
public:
	QuadTree(){}
	QuadTree(const QuadTree& tree);
	QuadTree(CString indexPath){ Init(indexPath); }
	~QuadTree(){ Destory(); }

	
//	bool Init();
	//初始化类信息,线程安全
	bool Init(const CString indexPath);

	//初始化四叉树节点  
	QuadNode *InitQuadNode();

	//构建满四叉树（叶节点不包含文件信息）
	void CreateFullQuadTree(CString indexPath);

	//构建四叉树（只包含有信息的叶节点）
	void BuildQuadTree(const CString indexPath);

	//获取目标范围的文件信息
	void GetFileName(const MapRect radarArea, std::vector<CString>& fileName);

	//获取指定空间对象的ID  
	void GetID(const MapRect itemRect, QuadNode* root, CString& id);

	//获取包含当前rect的文件范围
	bool GetBoxRect(MapRect &box, const MapRect rect);

	//判断小Box是否位于大Box中
	bool _isSmallInBig(const MapRect bigBox, const MapRect smallBox);

	//销毁四叉树，释放指针
	void Destory();

private:
	//获取子Box位于父Box的象限
	QuadrantEnum _getQuadRant(const MapRect Box[4], const MapRect smallBox);



	//CString转char*
	char* _CStringToCharArray(CString str);

	//将一块矩形分割成4块等大小矩形
	MapRect* _splitMap(const MapRect rect);

	MapRect* _splitMap1(const FileRect rect[4]);

	//将一个FileRect分成四个
	FileRect* _splitFile(const FileRect rect);

	//判断两个矩形区域是否相等(大致相等)
	bool _equal(const MapRect rect1, const MapRect rect2);

	//判断两个矩形区域是否有交集
	bool _hasIntersection(const MapRect rect1, const MapRect rect2);

	//递归释放指针
	void _destory(QuadNode* root);

	//插入节点，线程安全
	void Insert(const char* id, QuadNode* root, const CString fileName);

	//递归构建满四叉树（叶节点不包含文件信息）
	void _createFullQuadTree(QuadNode* root, const int depth);

	//获取包含当前rect的文件范围
	void _getBoxRect(MapRect &box, const MapRect rect,const QuadNode* root, const int depth,bool flag);

	int _roundUp(int a, int b = 2);
	int _roundDown(int a, int b = 2);





	bool Init_unlock(const CString indexPath);
	void Insert_unlock(const char* id, QuadNode* root, const CString fileName);
	void BuildQuadTree_unlock(const CString indexPath);
	void GetFileName_unlock(const MapRect radarArea, std::vector<CString>& fileName, const QuadNode* root);
	void Destory_unlock();
	void CreateFullQuadTree_unlock(CString indexPath);
	bool GetBoxRect_unlock(MapRect &box, const MapRect rect);
};






#endif