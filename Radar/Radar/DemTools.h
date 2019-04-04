//使用该文件请先引用命名空间NAMESPACE_DEMTOOLS
//该文件包含类DemMessage和类DemTools
//DemMessage主要包含单个海图的信息和高程数据
//DemTools包含一个装有DemMessage的容器，并对DemMessage进行管理
//未写深拷贝构造函数，勿将这些类作为函数的值传递参数、=赋值操作、A(a)构造等，浅拷贝可能造成原类对象的空间被其副本的构造函数释放掉。
#pragma once

#include"stdafx.h"
#include"QuadTree.h"
#include<cstdlib>
#include<cstdio>
#include<vector>

namespace NAMESPACE_DEMTOOLS{
	class DemMessage
	{
	//属性
	public:
		float* data;
		QuadTree quadTree;
		QuadTree fullTree;
		std::vector<CString> allFile;	//当前树中包含的所有子文件路径
		std::vector<CString> loadedFile;	//当前树中已加载的文件路径
	//方法
	public:
		DemMessage();
		DemMessage(const DemMessage& demMessage);
		~DemMessage();
		bool GetAllFile(CString indexPath);	//获取当前树的所有子文件路径
	};

	

	class DemTools
	{
	//属性
	public:
		std::vector<DemMessage*> m_dem;	//包含所有DEM信息的容器
		int m_pos;	//记录当前所处数据块在容器中的位置
	//方法
	public:
		DemTools();
		DemTools(const DemTools& demTools);
		~DemTools();

		void ReFresh(double lon,double lat);
		void UpdataDem(const MapRect rect);
		
	private:
		bool PtInRect(double dadiX, double dadiY, double minX, double minY, double maxX, double maxY);
		bool PtInRect(double lon, double lat, MapRect rect);
		bool GetIndexPath(CString &indexPath, const double longitude, const double latitude);
		void A_not_in_B(const std::vector<CString> A, const std::vector<CString> B, std::vector<CString>& C);
		void LoadDemFile(CString filePath, DemMessage* dem);;
		void LoadDemFile_memoryMapping(CString filePath, DemMessage* dem);
		void LoadDemFile_freopen(CString filePath, DemMessage* dem);
		void ThreadFunc_LoadDemFile(const std::vector<CString> filePaths, DemMessage* dem, int start, int end);
		void FunA(int a);
		void FunB(int a, int start, int end);		
	};

	


}
