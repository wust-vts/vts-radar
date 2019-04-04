//ʹ�ø��ļ��������������ռ�NAMESPACE_DEMTOOLS
//���ļ�������DemMessage����DemTools
//DemMessage��Ҫ����������ͼ����Ϣ�͸߳�����
//DemTools����һ��װ��DemMessage������������DemMessage���й���
//δд������캯��������Щ����Ϊ������ֵ���ݲ�����=��ֵ������A(a)����ȣ�ǳ�����������ԭ�����Ŀռ䱻�丱���Ĺ��캯���ͷŵ���
#pragma once

#include"stdafx.h"
#include"QuadTree.h"
#include<cstdlib>
#include<cstdio>
#include<vector>

namespace NAMESPACE_DEMTOOLS{
	class DemMessage
	{
	//����
	public:
		float* data;
		QuadTree quadTree;
		QuadTree fullTree;
		std::vector<CString> allFile;	//��ǰ���а������������ļ�·��
		std::vector<CString> loadedFile;	//��ǰ�����Ѽ��ص��ļ�·��
	//����
	public:
		DemMessage();
		DemMessage(const DemMessage& demMessage);
		~DemMessage();
		bool GetAllFile(CString indexPath);	//��ȡ��ǰ�����������ļ�·��
	};

	

	class DemTools
	{
	//����
	public:
		std::vector<DemMessage*> m_dem;	//��������DEM��Ϣ������
		int m_pos;	//��¼��ǰ�������ݿ��������е�λ��
	//����
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
