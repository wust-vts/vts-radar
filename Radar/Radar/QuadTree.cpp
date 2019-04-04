#include "stdafx.h"
#include"QuadTree.h"
#include <vector>
#include<fstream>
#include<mutex>
using namespace std;

mutex mtx_quadtree;
//向上取整
int QuadTree::_roundUp(int a, int b)
{
	int result = a / b;
	if (a%b)
		result++;
	return result;
}

//向下取整
int QuadTree::_roundDown(int a, int b)
{
	return (a / b);
}

//初始化根节点，载入配置文件信息。false为配置文件载入错误
//bool QuadTree::Init()
//{
//	if (m_root != NULL)
//	{
//		Destory();
//	}
//	m_root = InitQuadNode();
//
//	CStdioFile file;
//	if (file.Open(_T("../Radar/CONFIGURATION.txt"), CFile::modeRead))
//	{
//		CString str;
//		double minX, minY, maxX, maxY;
//		file.ReadString(str);
//		minX = atof(_CStringToCharArray(str));
//		file.ReadString(str);
//		minY = atof(_CStringToCharArray(str));
//		file.ReadString(str);
//		maxX = atof(_CStringToCharArray(str));
//		file.ReadString(str);
//		maxY = atof(_CStringToCharArray(str));
//		file.ReadString(str);
//		m_minCol = atol(_CStringToCharArray(str));
//		file.ReadString(str);
//		m_minRow = atol(_CStringToCharArray(str));
//		file.ReadString(str);
//		m_depth = atoi(_CStringToCharArray(str));
//		m_root->Box.minX = minX;
//		m_root->Box.minY = minY;
//		m_root->Box.maxX = maxX;
//		m_root->Box.maxY = maxY;
//		file.Close();
//	}
//	else
//	{
//		return false;
//	}
//	return true;
//}

//深拷贝构造
QuadTree::QuadTree(const QuadTree& tree)
{
	if (tree.m_root == NULL)
		m_root = NULL;
	else
	{
		if (Init_unlock(tree.indexFilePath))
		{
			if (tree.m_isFullTree)
				CreateFullQuadTree_unlock(tree.indexFilePath);
			else
				BuildQuadTree_unlock(tree.indexFilePath);
		}

	}
	m_depth = tree.m_depth;
	m_row = tree.m_row;
	m_col = tree.m_col;
	m_minX = tree.m_minX;
	m_minY = tree.m_minY;
	m_maxX = tree.m_maxX;
	m_maxY = tree.m_maxY;
	m_stepX = tree.m_stepX;
	m_stepY = tree.m_stepY;
	m_minRow = tree.m_minRow;
	m_minCol = tree.m_minCol;
	m_isBuildOK = tree.m_isBuildOK;
	m_isFullTree = tree.m_isFullTree;
	indexFilePath = tree.indexFilePath;
	

}

//初始化类信息,线程安全
bool QuadTree::Init(const CString indexPath)
{
//	lock_guard<mutex> locker(mtx_quadtree);
	mtx_quadtree.lock();
	bool flag = Init_unlock(indexPath);
	mtx_quadtree.unlock();
	return(flag);
}


//根据指定的index文件初始化类
bool QuadTree::Init_unlock(const CString indexPath)
{
	ifstream indexFile(indexPath);
	if (!indexFile.is_open())
	{
		return false;
	}
	if (m_root != NULL)
	{
		Destory_unlock();
	}	
	indexFile >> m_minX >> m_minY >> m_maxX >> m_maxY >> m_row >> m_col >> m_stepX >> m_stepY >> m_minRow >> m_minCol >> m_depth;
	indexFile.close();
	indexFilePath = indexPath;
	m_root = InitQuadNode();
	m_root->Box.minX = m_minX;
	m_root->Box.minY = m_minY;
	m_root->Box.maxX = m_maxX;
	m_root->Box.maxY = m_maxY;
	m_root->filePos.left = 0;
	m_root->filePos.top = 0;
	m_root->filePos.right = m_col - 1;
	m_root->filePos.bottom = m_row - 1;
	return true;
}

//初始化四叉树节点  
QuadNode* QuadTree::InitQuadNode()
{
	struct QuadNode *node;
//	node = (QuadNode*)malloc(sizeof(struct QuadNode));
	node = new QuadNode;
	node->Box.maxX = 0;
	node->Box.maxY = 0;
	node->Box.minX = 0;
	node->Box.minY = 0;
	node->filePos.left = 0;
	node->filePos.top = 0;
	node->filePos.right = 0;
	node->filePos.bottom = 0;

	for (int i = 0; i < 4; i++)
	{
		node->children[i] = NULL;
	}
	node->nChildCount = 0;
	node->fileName = NULL;
	node->ID = "";
	return node;
}

//获取指定空间对象的ID,只在FileTools中使用
void QuadTree::GetID(const MapRect itemRect, QuadNode* root, CString& id)
{
	if (_equal(itemRect, root->Box))
	{
		return;
	}
	else
	{
		FileRect* fileRect = _splitFile(root->filePos);
		MapRect* box = _splitMap1(fileRect);
		QuadrantEnum quadRant = _getQuadRant(box, itemRect);
		switch (quadRant)
		{
		case UL:
			id += "0";
			break;
		case UR:
			id += "1";
			break;
		case LL:
			id += "2";
			break;
		case LR:
			id += "3";
			break;
		default:
			break;
		}
		//按需创建分支
		if (root->children[quadRant] == NULL)
		{
			root->children[quadRant] = InitQuadNode();
			root->children[quadRant]->Box = box[quadRant];
			root->children[quadRant]->filePos = fileRect[quadRant];
			root->nChildCount++;
		}
		GetID(itemRect, root->children[quadRant], id);
		free(box);
		free(fileRect);
	}
}

//插入节点，线程安全
void QuadTree::Insert(const char* id, QuadNode* root, const CString fileName)
{
//	lock_guard<mutex> locker(mtx_quadtree);
	mtx_quadtree.lock();
	Insert_unlock(id, root, fileName);
	mtx_quadtree.unlock();
}

//插入节点
void QuadTree::Insert_unlock(const char* id, QuadNode* root, const CString fileName)
{
	if (*id == '\0')
	{
		root->fileName = _CStringToCharArray(fileName);
		return;
	}
	else
	{
		FileRect* fileRect = _splitFile(root->filePos);
		MapRect* box = _splitMap1(fileRect);
		CString ID = root->ID;
		switch (*id)
		{
		case '0':
		{
			if (root->children[0] == NULL)
			{
				root->children[0] = InitQuadNode();
				root->children[0]->Box = box[0];
				root->children[0]->filePos = fileRect[0];
				root->nChildCount++;
				ID += *id;
				root->children[0]->ID = ID;
			}
			Insert_unlock(++id, root->children[0], fileName);
			break;
		}
		case '1':
		{
			if (root->children[1] == NULL)
			{
				root->children[1] = InitQuadNode();
				root->children[1]->Box = box[1];
				root->children[1]->filePos = fileRect[1];
				root->nChildCount++;
				ID += *id;
				root->children[1]->ID = ID;
			}
			Insert_unlock(++id, root->children[1], fileName);
			break;
		}
		case '2':
		{
			if (root->children[2] == NULL)
			{
				root->children[2] = InitQuadNode();
				root->children[2]->Box = box[2];
				root->children[2]->filePos = fileRect[2];
				root->nChildCount++;
				ID += *id;
				root->children[2]->ID = ID;
			}
			Insert_unlock(++id, root->children[2], fileName);
			break;
		}
		case '3':
		{
			if (root->children[3] == NULL)
			{
				root->children[3] = InitQuadNode();
				root->children[3]->Box = box[3];
				root->children[3]->filePos = fileRect[3];
				root->nChildCount++;
				ID += *id;
				root->children[3]->ID = ID;
			}
			Insert_unlock(++id, root->children[3], fileName);
			break;
		}
		default:
			break;
		}
		free(box);
		free(fileRect);
	}
}

//判断两个矩形区域是否相等(大致相等)
bool QuadTree::_equal(const MapRect rect1, const MapRect rect2)
{
	if (rect1.minX <= rect2.minX + 10 && rect1.minX >= rect2.minX - 10 && rect1.minY <= rect2.minY + 10 && rect1.minY >= rect2.minY - 10
		&& rect1.maxX <= rect2.maxX + 10 && rect1.maxX >= rect2.maxX - 10 && rect1.maxY <= rect2.maxY + 10 && rect1.maxY >= rect2.maxY - 10)
	{
		return true;
	}
	return false;
}

//获取子Box位于父Box的象限
QuadrantEnum QuadTree::_getQuadRant(const MapRect box[4], const MapRect smallBox)
{
	if (_isSmallInBig(box[UL], smallBox))
	{
		return UL;
	}
	else if (_isSmallInBig(box[UR], smallBox))
	{
		return UR;
	}
	else if (_isSmallInBig(box[LL], smallBox))
	{
		return LL;
	}
	else if (_isSmallInBig(box[LR], smallBox))
	{
		return LR;
	}
}

//判断小Box是否位于大Box中
bool QuadTree::_isSmallInBig(const MapRect bigBox, const MapRect smallBox)
{
	if (smallBox.minX >= bigBox.minX-1&&smallBox.minY >= bigBox.minY-1&&smallBox.maxX <= bigBox.maxX+1&&smallBox.maxY <= bigBox.maxY+1)
		return true;
	else
		return false;
}

//将一块矩形分割成4块等大小矩形,用完需free掉
MapRect* QuadTree::_splitMap(const MapRect rect)
{
	struct MapRect* box;
	box = (MapRect*)malloc(sizeof(struct MapRect) * 4);
	box[LL].minX = rect.minX;
	box[LL].minY = rect.minY;
	box[LL].maxX = rect.minX + (rect.maxX - rect.minX) / 2;
	box[LL].maxY = rect.minY + (rect.maxY - rect.minY) / 2;
	box[LR].minX = box[LL].maxX;
	box[LR].maxX = rect.maxX;
	box[LR].minY = rect.minY;
	box[LR].maxY = box[LL].maxY;
	box[UL].minX = rect.minX;
	box[UL].maxX = box[LL].maxX;
	box[UL].minY = box[LL].maxY;
	box[UL].maxY = rect.maxY;
	box[UR].minX = box[UL].maxX;
	box[UR].maxX = rect.maxX;
	box[UR].minY = box[UL].minY;
	box[UR].maxY = rect.maxY;
	return box;
}

//根据分割的4个文件在原文件中的位置算出4个文件的坐标范围
MapRect* QuadTree::_splitMap1(const FileRect rect[4])
{
	struct MapRect* box = (MapRect*)malloc(4 * sizeof(MapRect));
	box[0].minX = m_minX + rect[0].left*m_stepX;
	box[0].maxX = m_minX + rect[0].right*m_stepX;
	box[0].maxY = m_maxY - rect[0].top*m_stepY;
	box[0].minY = m_maxY - rect[0].bottom*m_stepY;
	box[1].minX = m_minX + rect[1].left*m_stepX;
	box[1].maxX = m_minX + rect[1].right*m_stepX;
	box[1].maxY = m_maxY - rect[1].top*m_stepY;
	box[1].minY = m_maxY - rect[1].bottom*m_stepY;
	box[2].minX = m_minX + rect[2].left*m_stepX;
	box[2].maxX = m_minX + rect[2].right*m_stepX;
	box[2].maxY = m_maxY - rect[2].top*m_stepY;
	box[2].minY = m_maxY - rect[2].bottom*m_stepY;
	box[3].minX = m_minX + rect[3].left*m_stepX;
	box[3].maxX = m_minX + rect[3].right*m_stepX;
	box[3].maxY = m_maxY - rect[3].top*m_stepY;
	box[3].minY = m_maxY - rect[3].bottom*m_stepY;
	return box;
}



//将一个FileRect分成四个,用完需free
FileRect* QuadTree::_splitFile(const FileRect rect)
{
	struct FileRect * box;
	box = (FileRect *)malloc(4 * sizeof(FileRect));
	int child_right = rect.left + _roundDown(rect.right - rect.left);
	int child_left = rect.left + _roundUp(rect.right - rect.left);
	int child_bottom = rect.top + _roundDown(rect.bottom - rect.top);
	int child_top = rect.top + _roundUp(rect.bottom - rect.top);
	box[0].left = rect.left;
	box[0].right = child_right;
	box[0].top = rect.top;
	box[0].bottom = child_bottom;
	box[1].left = child_left;
	box[1].right = rect.right;
	box[1].top = rect.top;
	box[1].bottom = child_bottom;
	box[2].left = rect.left;
	box[2].right = child_right;
	box[2].top = child_top;
	box[2].bottom = rect.bottom;
	box[3].left = child_left;
	box[3].right = rect.right;
	box[3].top = child_top;
	box[3].bottom = rect.bottom;
	return box;
}

//构建四叉树,线程安全
void QuadTree::BuildQuadTree(const CString indexPath)
{
//	lock_guard<mutex> locker(mtx_quadtree);
	mtx_quadtree.lock();
	BuildQuadTree_unlock(indexPath);
	mtx_quadtree.unlock();
}

//构建四叉树
void QuadTree::BuildQuadTree_unlock(const CString indexPath)
{
	CStdioFile indexFile;
	indexFile.Open(indexPath, CFile::modeRead);
	CString id, fileName,temp;
	for (int i = 0; i < 11; i++)
		indexFile.ReadString(temp);
	while (indexFile.ReadString(id))
	{
		char* c_id = _CStringToCharArray(id);
		if (indexFile.ReadString(fileName))
		{
			CString ID;
			ID = "";
			Insert_unlock(c_id, m_root, fileName);
		}
		delete c_id;
	}
	indexFile.Close();
	m_isFullTree = false;
	m_isBuildOK = true;
}

//CString转char* 需free
char* QuadTree::_CStringToCharArray(CString str)
{
	char *ptr;

#ifdef _UNICODE
	LONG len;
	len = WideCharToMultiByte(CP_ACP, 0, str, -1, NULL, 0, NULL, NULL);
	ptr = new char[len + 1];
	memset(ptr, 0, len + 1);
	WideCharToMultiByte(CP_ACP, 0, str, -1, ptr, len + 1, NULL, NULL);
#else
	ptr = new char[str.GetAllocLength() + 1];
	sprintf(ptr, _T("%s"), str);
#endif
	return ptr;
}

//判断两个矩形区域是否有交集
bool QuadTree::_hasIntersection(const MapRect rect1, const MapRect rect2)
{
	if (rect1.minX >= rect2.maxX || rect1.maxX <= rect2.minX || rect1.minY >= rect2.maxY || rect1.maxY <= rect2.minY)
		return false;
	else
		return true;
}

//获取目标范围的文件信息,线程安全
void QuadTree::GetFileName(const MapRect radarArea, vector<CString>& fileName)
{
//	lock_guard<mutex> locker(mtx_quadtree);
	mtx_quadtree.lock();
	fileName.clear();
	if (m_root!=NULL)
		GetFileName_unlock(radarArea, fileName, m_root);
	mtx_quadtree.unlock();
}

//获取目标范围的文件信息
void QuadTree::GetFileName_unlock(const MapRect radarArea, vector<CString>& fileName,const QuadNode* root)
{
	
	if (root->nChildCount == 0)
	{
		if (root->fileName != NULL)
		{
			fileName.push_back(CString(root->fileName));
		}
		return;
	}
	else
	{
		if (root->children[UL] != NULL)
		{
			if (_hasIntersection(radarArea, root->children[UL]->Box))
			{
				GetFileName_unlock(radarArea, fileName, root->children[UL]);
			}
		}
		if (root->children[UR] != NULL)
		{
			if (_hasIntersection(radarArea, root->children[UR]->Box))
			{
				GetFileName_unlock(radarArea, fileName, root->children[UR]);
			}
		}
		if (root->children[LL] != NULL)
		{
			if (_hasIntersection(radarArea, root->children[LL]->Box))
			{
				GetFileName_unlock(radarArea, fileName, root->children[LL]);
			}
		}
		if (root->children[LR] != NULL)
		{
			if (_hasIntersection(radarArea, root->children[LR]->Box))
			{
				GetFileName_unlock(radarArea, fileName, root->children[LR]);
			}
		}

	}
}

//销毁四叉数,线程安全
void QuadTree::Destory()
{
//	lock_guard<mutex> locker(mtx_quadtree);
	mtx_quadtree.lock();
	Destory_unlock();
	mtx_quadtree.unlock();
}

void QuadTree::Destory_unlock()
{
	if (m_root != NULL)
		_destory(m_root);
	m_root = NULL;
	m_depth = 0;
	m_minX = 0;
	m_minY = 0;
	m_maxX = 0;
	m_maxY = 0;
	m_row = 0;
	m_col = 0;
	m_stepX = 0.0;
	m_stepY = 0.0;
	m_minCol = 0;
	m_minRow = 0;
	m_isFullTree = false;
	m_isBuildOK = false;
	indexFilePath = "";
}

//递归释放指针
void QuadTree::_destory(QuadNode* root)
{
	if (root->children[0] != NULL)
	{
		_destory(root->children[0]);
	}
	if (root->children[1] != NULL)
	{
		_destory(root->children[1]);
	}
	if (root->children[2] != NULL)
	{
		_destory(root->children[2]);
	}
	if (root->children[3] != NULL)
	{
		_destory(root->children[3]);
	}
	if (root->fileName != NULL)
	{
		delete root->fileName;
		root->fileName = NULL;
	}
	delete root;
	root = NULL;
}

//构建满四叉树（叶节点不包含文件信息）
void QuadTree::_createFullQuadTree(QuadNode* root, const int depth)
{
	if (depth == 0)
		return;
	FileRect* fileRect = _splitFile(root->filePos);
	MapRect* rects = _splitMap1(fileRect);
	root->nChildCount = 4;
	CString ID[4];
	for (int i = 0; i < 4; i++)
	{
		root->children[i] = InitQuadNode();
		root->children[i]->Box = rects[i];
		root->children[i]->filePos = fileRect[i];
		char zero = '0';
		char c_i = zero + i;
		ID[i] = root->ID;
		ID[i] += c_i;
		root->children[i]->ID = ID[i];
		_createFullQuadTree(root->children[i], depth - 1);
	}
	free(rects);
	free(fileRect);
}

//构建满四叉树（叶节点不包含文件信息），线程安全
void QuadTree::CreateFullQuadTree(CString indexPath)
{
//	lock_guard<mutex> locker(mtx_quadtree);
	mtx_quadtree.lock();
	CreateFullQuadTree_unlock(indexPath);
	mtx_quadtree.unlock();
}

//构建满四叉树（叶节点不包含文件信息）
void QuadTree::CreateFullQuadTree_unlock(CString indexPath)
{
	Init_unlock(indexPath);
	_createFullQuadTree(m_root, m_depth);
	m_isFullTree = true;
	m_isBuildOK = true;
}

//获取包含当前rect的文件范围(要先调用CreateFullQuadTree())
void QuadTree::_getBoxRect(MapRect &box, const MapRect rect, const QuadNode* root,const int depth,bool flag)
{
	if (depth == 0)
	{
		if (root->Box.minX < box.minX)
			box.minX = root->Box.minX;
		if (root->Box.minY < box.minY)
			box.minY = root->Box.minY;
		if (root->Box.maxX > box.maxX)
			box.maxX = root->Box.maxX;
		if (root->Box.maxY > box.maxY)
			box.maxY = root->Box.maxY;
		return;
	}
	for (int i = 0; i < 4; i++)
	{
		if (root->children[i] == NULL)
		{
			flag = false;
			return;
		}
		if (_hasIntersection(rect, root->children[i]->Box))
			_getBoxRect(box, rect, root->children[i], depth - 1, flag);
	}
		
}

//获取包含当前rect的文件范围,false为获取失败,线程安全
bool QuadTree::GetBoxRect(MapRect &box, const MapRect rect)
{
//	lock_guard<mutex> locker(mtx_quadtree);
	mtx_quadtree.lock();
	bool flag = GetBoxRect_unlock(box, rect);
	mtx_quadtree.unlock();
	return(flag);
}

//获取包含当前rect的文件范围,false为获取失败
bool QuadTree::GetBoxRect_unlock(MapRect &box, const MapRect rect)
{
	
	if (m_isFullTree&&m_isBuildOK)
	{
		MapRect temp =  rect;
		bool flag = true;
		_getBoxRect(temp, rect, m_root, m_depth,flag);
		if (flag)
		{
			box = temp;
			return true;
		}		
	}
	return false;
}