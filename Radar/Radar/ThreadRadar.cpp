#include"stdafx.h"
#include"Thread.h"
#include"RadarView.h"
#include"RWLockImpl.h"
#include<mutex>
#include"PublicMethod.h"

using namespace std;

extern HANDLE closeWindowEvent;
extern HANDLE closeThread_loadDemFile_event;
extern HANDLE closeThread_threadCalculateARPARecord;
extern HANDLE closeThread_drawDemBMP;
extern CMyRWLock g_myRWLock;
extern mutex mtx_demUseDataMem;
extern mutex mtx_BMPmem;
//extern float demUseData[DEMBMP_HEIGHT][DEMBMP_WIDTH];	//用于产生高程图像的信息，从demFileData提取
//extern float demUseData_mem[DEMBMP_HEIGHT][DEMBMP_WIDTH];	//demUseData的内存副本，用于缓冲
extern Demdata demData, demData_mem;

//bool dem_check[DEMBMP_HEIGHT][DEMBMP_WIDTH];	//demUseData-->demBMP计算时所用

//UINT ThreadCalculateARPARecord(LPVOID lpParam)
//{
//	CRadarView * pThis = (CRadarView*)lpParam;
//	//计算消耗时间，确保每周期执行一次
//	//	CStdioFile timefile;
//	//	timefile.Open(_T("C:\\Users\\Administor\\Desktop\\timefile.txt"), CFile::modeReadWrite | CFile::modeCreate);
//	while (1)
//	{
//		DWORD sysTime1 = GetTickCount();
//		DWORD dwRetVal;
//		dwRetVal = WaitForSingleObject(pThis->m_hThreadEvent_closeCalculateARPARecord, 100);
//		if (dwRetVal == WAIT_TIMEOUT)
//		{
//			//todo
//
//
//
//			pThis->CalculateARPARecord();
//
//
//			CString str;
//			str.Format(_T("%lu\n"), sysTime1);
//
//			//		timefile.SeekToEnd();
//			//		timefile.WriteString(str);
//
//
//
//			//	Sleep(100);
//		}
//		else
//		{
//			// stop receive text thread.
//			/*DWORD dwExitCode;
//			GetExitCodeThread(pThis->pThreadCalculateARPARecord, &dwExitCode);
//			AfxEndThread(dwExitCode, TRUE);*/
//			break;
//		}
//
//		DWORD sysTime2 = GetTickCount();
//		DWORD diff = sysTime2 - sysTime1;
//
//		Sleep(RADAR_CYCLE * 1000 - diff);
//	}
//
//	//	timefile.Close();
//	SetEvent(closeThread_threadCalculateARPARecord);
//	return 0;
//}



//检查是否需要重新载入DEM文件并载入
//UINT ThreadLoadDemFile(LPVOID lpParam)
//{
//	CRadarView * pThis = (CRadarView*)lpParam;
//	bool loadAllDemFile = false;//是否已读取完DEM
//	while (1)
//	{
//		DWORD dwRetVal = WaitForSingleObject(closeWindowEvent, 100);
//		if (dwRetVal == WAIT_TIMEOUT)
//		{
//			if (!pThis->m_MainShip.aisReceiveDoubleCheck)
//			{
//				continue;
//			}
//			//	DWORD sysTime1 = GetTickCount();
//			g_myRWLock.WriteLock();
//			pThis->demTools.UpdataDem(pThis->m_radarDadiRect);
//			g_myRWLock.Unlock();
//			//	DWORD sysTime2 = GetTickCount();
//			//	DWORD diff1 = sysTime2 - sysTime1;
//
//			//	DWORD sysTime3 = GetTickCount();
//			if (pThis->demTools.m_pos != -1)
//				pThis->FileDataToUseData_1(pThis->demTools.m_dem.at(pThis->demTools.m_pos));
//			/*DWORD sysTime4 = GetTickCount();
//			DWORD diff2 = sysTime4 - sysTime3;*/
//
//		}
//		else
//		{
//			break;
//			/*DWORD dwExitCode;
//			GetExitCodeThread(pThis->pThreadLoadDemFile, &dwExitCode);
//			SetEvent(closeThread_loadDemFile_event);
//			AfxEndThread(dwExitCode, TRUE);			*/
//		}
//		Sleep(100);
//	}
//	SetEvent(closeThread_loadDemFile_event);
//	return 0;
//}

//UINT ThreadDrawDemBMP(LPVOID lpParam)	//矩形绘图,由dem根据Level提取的900*900数组	ThreadDrawMap1改进测试
//{
//	CRadarView * pThis = (CRadarView*)lpParam;
//
//	int locY = DEMBMP_HEIGHT / 2;
//	int locX = DEMBMP_WIDTH / 2;
//	//	long Posx, Posy;
//	//	int count;
//	//	DWORD white = 255;
//	//	DWORD black = 0;
//	//	int R = 255, G = 255, B = 255;
//	double t1 = 0.1;
//	double t2 = 1 - t1;
//	int zoom = 1;
//
//
//	while (1)
//	{
//		DWORD d1 = GetTickCount();
//		CBitmapEx bitmapEx;
//		bitmapEx.Create(DEMBMP_WIDTH, DEMBMP_HEIGHT);
//		bitmapEx.Clear(_RGB(0, 0, 64));
//		mtx_demUseDataMem.lock();
//
//		memset(dem_check, false, DEMBMP_WIDTH * DEMBMP_HEIGHT * sizeof(bool));
//
//		double locH = demUseData_mem[locY][locX]; //@本船所在地势高度
//		double locSumH = locH + RADAR_HIGHT;    //@本船雷达天线所在高度（加地势后）
//		int x = 0, y = 0;	//@当前扫面点坐标
//		double tarH;	//@扫描目标点高度
//		double obsH;  //@遮挡点高度
//		double DX = pThis->m_perPixelM;
//		double DY = pThis->m_perPixelM;
//		for (x = 0; x < DEMBMP_WIDTH; x++)
//		{
//			int w = x>locX ? (x - locX) : (locX - x);	//@直角三角形的横向边
//			int h = y > locY ? (y - locY) : (locY - y);	//@直角三角形的纵向边
//			int maxL = w > h ? w : h;
//			PointF p[DEMBMP_HEIGHT / 2 + 5];
//			double angle[DEMBMP_HEIGHT / 2 + 5];	//@雷达准线角
//			double maxAng = 0;
//			int i;
//			for (i = maxL; i > 0; i--)
//			{
//				if (w > h)
//				{
//					int a = (int)((double)h*i / w + 0.5);
//					if (x > locX)
//					{
//						p[i].X = locX + i;
//					}
//					else
//					{
//						p[i].X = locX - i;
//					}
//					if (y > locY)
//					{
//						p[i].Y = locY + a;
//					}
//					else
//					{
//						p[i].Y = locY - a;
//					}
//				}
//				else
//				{
//					int a = (int)((double)w*i / h + 0.5);
//					if (x > locX)
//					{
//						p[i].X = locX + a;
//					}
//					else
//					{
//						p[i].X = locX - a;
//					}
//					if (y > locY)
//					{
//						p[i].Y = locY + i;
//					}
//					else
//					{
//						p[i].Y = locY - i;
//					}
//				}
//			}
//
//			//开始在一条扫描线上扫描
//			tarH = demUseData_mem[(int)p[1].Y][(int)p[1].X];
//			double R = zoom*sqrt((p[1].X - locX)*(p[1].X - locX)*DX*DX + (p[1].Y - locY)*(p[1].Y - locY)*DY*DY);	//@离雷达物理距离（米）
//
//			if (tarH <= locSumH)
//			{
//				angle[1] = atan(R / (locSumH - tarH));
//			}
//			else
//			{
//				angle[1] = PI / 2 + atan((tarH - locSumH) / R);
//			}
//			maxAng = angle[1];
//			double Rmax = zoom*sqrt((p[maxL].X - locX)*(p[maxL].X - locX)*DX*DX + (p[maxL].Y - locY)*(p[maxL].Y - locY)*DY*DY);	//@扫描线像素长度
//
//			for (i = 2; i <= maxL; i++)
//			{
//				tarH = demUseData_mem[(int)p[i].Y][(int)p[i].X];
//				R = zoom*sqrt((p[i].X - locX)*(p[i].X - locX)*DX*DX + (p[i].Y - locY)*(p[i].Y - locY)*DY*DY);	//@离雷达物理距离（米）
//
//				if (tarH <= locH)	//当目标地势低于本船地势时，默认目标处于水域或无障碍物，不可反射电磁波
//				{
//					continue;
//				}
//				if (tarH <= locSumH)
//				{
//					angle[i] = atan(R / (locSumH - tarH));
//				}
//				else
//				{
//					angle[i] = PI / 2 + atan((tarH - locSumH) / R);
//				}
//				if (angle[i] >= maxAng)
//				{
//					//计算亮度
//					double k1 = (t1*atan(tarH / 10.0f)) / (PI / 2);
//					double slope = atan((tarH - demUseData_mem[(int)p[i - 1].Y][(int)p[i - 1].X]) / (zoom*sqrt((p[i].X - p[i - 1].X)*(p[i].X - p[i - 1].X)*DX*DX + (p[i].Y - p[i - 1].Y)*(p[i].Y - p[i - 1].Y)*DY*DY)));	//@斜角
//					double angleTrans = AngleTrans(angle[i] - slope);
//					double k2 = t2*cos(angleTrans);
//
//					//	double k2 = 10*t2*cos(angle[i] - slope);
//					double t = k1 + k2;
//					double Pb = t*(1 - (R / Rmax)*(R / Rmax)*(R / Rmax)*(R / Rmax));
//
//					if (Pb < 0)Pb = 0;
//					if (Pb > 1)Pb = 1;
//
//					//	Pb = 1;
//
//					int Gray_R = 255 * Pb;
//					DWORD Gray = _RGB(Gray_R, Gray_R, 0);
//
//					maxAng = angle[i];
//					bitmapEx.SetPixel((int)p[i].X, (int)p[i].Y, Gray);
//				}
//			}
//		}
//		y = DEMBMP_HEIGHT - 1;
//		for (x = 0; x < DEMBMP_WIDTH; x++)
//		{
//			int w = x>locX ? (x - locX) : (locX - x);	//@直角三角形的横向边
//			int h = y > locY ? (y - locY) : (locY - y);	//@直角三角形的纵向边
//			int maxL = w > h ? w : h;
//			PointF p[DEMBMP_HEIGHT / 2 + 5];
//			double angle[DEMBMP_HEIGHT / 2 + 5];
//			double maxAng = 0;
//			int i;
//			for (i = maxL; i > 0; i--)
//			{
//				if (w > h)
//				{
//					int a = (int)((double)h*i / w + 0.5);
//					if (x > locX)
//					{
//						p[i].X = locX + i;
//					}
//					else
//					{
//						p[i].X = locX - i;
//					}
//					if (y > locY)
//					{
//						p[i].Y = locY + a;
//					}
//					else
//					{
//						p[i].Y = locY - a;
//					}
//				}
//				else
//				{
//					int a = (int)((double)w*i / h + 0.5);
//					if (x > locX)
//					{
//						p[i].X = locX + a;
//					}
//					else
//					{
//						p[i].X = locX - a;
//					}
//					if (y > locY)
//					{
//						p[i].Y = locY + i;
//					}
//					else
//					{
//						p[i].Y = locY - i;
//					}
//				}
//			}
//
//			//开始在一条扫描线上扫描
//			tarH = demUseData_mem[(int)p[1].Y][(int)p[1].X];
//			double R = zoom*sqrt((p[1].X - locX)*(p[1].X - locX)*DX*DX + (p[1].Y - locY)*(p[1].Y - locY)*DY*DY);	//@离雷达物理距离（米）
//
//			if (tarH <= locSumH)
//			{
//				angle[1] = atan(R / (locSumH - tarH));
//			}
//			else
//			{
//				angle[1] = PI / 2 + atan((tarH - locSumH) / R);
//			}
//			maxAng = angle[1];
//			double Rmax = zoom*sqrt((p[maxL].X - locX)*(p[maxL].X - locX)*DX*DX + (p[maxL].Y - locY)*(p[maxL].Y - locY)*DY*DY);	//@扫描线像素长度
//
//			for (i = 2; i <= maxL; i++)
//			{
//				tarH = demUseData_mem[(int)p[i].Y][(int)p[i].X];
//				R = zoom*sqrt((p[i].X - locX)*(p[i].X - locX)*DX*DX + (p[i].Y - locY)*(p[i].Y - locY)*DY*DY);	//@离雷达物理距离（米）
//
//				if (tarH <= locH)	//当目标地势低于本船地势时，默认目标处于水域或无障碍物，不可反射电磁波
//				{
//					continue;
//				}
//				if (tarH <= locSumH)
//				{
//					angle[i] = atan(R / (locSumH - tarH));
//				}
//				else
//				{
//					angle[i] = PI / 2 + atan((tarH - locSumH) / R);
//				}
//				if (angle[i] >= maxAng)
//				{
//					//计算亮度
//					double k1 = (t1*atan(tarH / 10.0f)) / (PI / 2);
//					double slope = atan((tarH - demUseData_mem[(int)p[i - 1].Y][(int)p[i - 1].X]) / (zoom*sqrt((p[i].X - p[i - 1].X)*(p[i].X - p[i - 1].X)*DX*DX + (p[i].Y - p[i - 1].Y)*(p[i].Y - p[i - 1].Y)*DY*DY)));	//@斜角
//					double angleTrans = AngleTrans(angle[i] - slope);
//					double k2 = t2*cos(angleTrans);
//
//					//	double k2 = 10*t2*cos(angle[i] - slope);
//					double t = k1 + k2;
//					double Pb = t*(1 - (R / Rmax)*(R / Rmax)*(R / Rmax)*(R / Rmax));
//
//					if (Pb < 0)Pb = 0;
//					if (Pb > 1)Pb = 1;
//
//					//	Pb = 1;
//
//					int Gray_R = 255 * Pb;
//					DWORD Gray = _RGB(Gray_R, Gray_R, 0);
//
//					maxAng = angle[i];
//					bitmapEx.SetPixel((int)p[i].X, (int)p[i].Y, Gray);
//				}
//			}
//		}
//		x = 0;
//		for (y = 0; y < DEMBMP_HEIGHT; y++)
//		{
//			int w = x>locX ? (x - locX) : (locX - x);	//@直角三角形的横向边
//			int h = y > locY ? (y - locY) : (locY - y);	//@直角三角形的纵向边
//			int maxL = w > h ? w : h;
//			PointF p[DEMBMP_WIDTH / 2 + 5];
//			double angle[DEMBMP_HEIGHT / 2 + 5];
//			double maxAng = 0;
//			int i;
//			for (i = maxL; i > 0; i--)
//			{
//				if (w > h)
//				{
//					int a = (int)((double)h*i / w + 0.5);
//					if (x > locX)
//					{
//						p[i].X = locX + i;
//					}
//					else
//					{
//						p[i].X = locX - i;
//					}
//					if (y > locY)
//					{
//						p[i].Y = locY + a;
//					}
//					else
//					{
//						p[i].Y = locY - a;
//					}
//				}
//				else
//				{
//					int a = (int)((double)w*i / h + 0.5);
//					if (x > locX)
//					{
//						p[i].X = locX + a;
//					}
//					else
//					{
//						p[i].X = locX - a;
//					}
//					if (y > locY)
//					{
//						p[i].Y = locY + i;
//					}
//					else
//					{
//						p[i].Y = locY - i;
//					}
//				}
//			}
//			//开始在一条扫描线上扫描
//			tarH = demUseData_mem[(int)p[1].Y][(int)p[1].X];
//			double R = zoom*sqrt((p[1].X - locX)*(p[1].X - locX)*DX*DX + (p[1].Y - locY)*(p[1].Y - locY)*DY*DY);	//@离雷达物理距离（米）
//
//			if (tarH <= locSumH)
//			{
//				angle[1] = atan(R / (locSumH - tarH));
//			}
//			else
//			{
//				angle[1] = PI / 2 + atan((tarH - locSumH) / R);
//			}
//			maxAng = angle[1];
//			double Rmax = zoom*sqrt((p[maxL].X - locX)*(p[maxL].X - locX)*DX*DX + (p[maxL].Y - locY)*(p[maxL].Y - locY)*DY*DY);	//@扫描线像素长度
//
//			for (i = 2; i <= maxL; i++)
//			{
//				tarH = demUseData_mem[(int)p[i].Y][(int)p[i].X];
//				R = zoom*sqrt((p[i].X - locX)*(p[i].X - locX)*DX*DX + (p[i].Y - locY)*(p[i].Y - locY)*DY*DY);	//@离雷达物理距离（米）
//
//				if (tarH <= locH)	//当目标地势低于本船地势时，默认目标处于水域或无障碍物，不可反射电磁波
//				{
//					continue;
//				}
//				if (tarH <= locSumH)
//				{
//					angle[i] = atan(R / (locSumH - tarH));
//				}
//				else
//				{
//					angle[i] = PI / 2 + atan((tarH - locSumH) / R);
//				}
//				if (angle[i] >= maxAng)
//				{
//					//计算亮度
//					double k1 = (t1*atan(tarH / 10.0f)) / (PI / 2);
//					double slope = atan((tarH - demUseData_mem[(int)p[i - 1].Y][(int)p[i - 1].X]) / (zoom*sqrt((p[i].X - p[i - 1].X)*(p[i].X - p[i - 1].X)*DX*DX + (p[i].Y - p[i - 1].Y)*(p[i].Y - p[i - 1].Y)*DY*DY)));	//@斜角
//					double angleTrans = AngleTrans(angle[i] - slope);
//					double k2 = t2*cos(angleTrans);
//
//					//	double k2 = 10*t2*cos(angle[i] - slope);
//					double t = k1 + k2;
//					double Pb = t*(1 - (R / Rmax)*(R / Rmax)*(R / Rmax)*(R / Rmax));
//
//					if (Pb < 0)Pb = 0;
//					if (Pb > 1)Pb = 1;
//
//					//		Pb = 1;
//
//					int Gray_R = 255 * Pb;
//					DWORD Gray = _RGB(Gray_R, Gray_R, 0);
//
//					maxAng = angle[i];
//					bitmapEx.SetPixel((int)p[i].X, (int)p[i].Y, Gray);
//				}
//			}
//		}
//		x = DEMBMP_WIDTH - 1;
//		for (y = 0; y < DEMBMP_HEIGHT; y++)
//		{
//			int w = x>locX ? (x - locX) : (locX - x);	//@直角三角形的横向边
//			int h = y > locY ? (y - locY) : (locY - y);	//@直角三角形的纵向边
//			int maxL = w > h ? w : h;
//			PointF p[DEMBMP_WIDTH / 2 + 5];
//			double angle[DEMBMP_HEIGHT / 2 + 5];
//			double maxAng = 0;
//			int i;
//			for (i = maxL; i > 0; i--)
//			{
//				if (w > h)
//				{
//					int a = (int)((double)h*i / w + 0.5);
//					if (x > locX)
//					{
//						p[i].X = locX + i;
//					}
//					else
//					{
//						p[i].X = locX - i;
//					}
//					if (y > locY)
//					{
//						p[i].Y = locY + a;
//					}
//					else
//					{
//						p[i].Y = locY - a;
//					}
//				}
//				else
//				{
//					int a = (int)((double)w*i / h + 0.5);
//					if (x > locX)
//					{
//						p[i].X = locX + a;
//					}
//					else
//					{
//						p[i].X = locX - a;
//					}
//					if (y > locY)
//					{
//						p[i].Y = locY + i;
//					}
//					else
//					{
//						p[i].Y = locY - i;
//					}
//				}
//			}
//			//开始在一条扫描线上扫描
//			tarH = demUseData_mem[(int)p[1].Y][(int)p[1].X];
//			double R = zoom*sqrt((p[1].X - locX)*(p[1].X - locX)*DX*DX + (p[1].Y - locY)*(p[1].Y - locY)*DY*DY);	//@离雷达物理距离（米）
//
//			if (tarH <= locSumH)
//			{
//				angle[1] = atan(R / (locSumH - tarH));
//			}
//			else
//			{
//				angle[1] = PI / 2 + atan((tarH - locSumH) / R);
//			}
//			maxAng = angle[1];
//			double Rmax = zoom*sqrt((p[maxL].X - locX)*(p[maxL].X - locX)*DX*DX + (p[maxL].Y - locY)*(p[maxL].Y - locY)*DY*DY);	//@扫描线像素长度
//
//			for (i = 2; i <= maxL; i++)
//			{
//				tarH = demUseData_mem[(int)p[i].Y][(int)p[i].X];
//				R = zoom*sqrt((p[i].X - locX)*(p[i].X - locX)*DX*DX + (p[i].Y - locY)*(p[i].Y - locY)*DY*DY);	//@离雷达物理距离（米）
//
//				if (tarH <= locH)	//当目标地势低于本船地势时，默认目标处于水域或无障碍物，不可反射电磁波
//				{
//					continue;
//				}
//				if (tarH <= locSumH)
//				{
//					angle[i] = atan(R / (locSumH - tarH));
//				}
//				else
//				{
//					angle[i] = PI / 2 + atan((tarH - locSumH) / R);
//				}
//				if (angle[i] >= maxAng)
//				{
//					//计算亮度
//					double k1 = (t1*atan(tarH / 10.0f)) / (PI / 2);
//					double slope = atan((tarH - demUseData_mem[(int)p[i - 1].Y][(int)p[i - 1].X]) / (zoom*sqrt((p[i].X - p[i - 1].X)*(p[i].X - p[i - 1].X)*DX*DX + (p[i].Y - p[i - 1].Y)*(p[i].Y - p[i - 1].Y)*DY*DY)));	//@斜角
//					double angleTrans = AngleTrans(angle[i] - slope);
//					double k2 = t2*cos(angleTrans);
//
//					//	double k2 = 10*t2*cos(angle[i] - slope);
//					double t = k1 + k2;
//					double Pb = t*(1 - (R / Rmax)*(R / Rmax)*(R / Rmax)*(R / Rmax));
//
//
//
//					if (Pb < 0)Pb = 0;
//					if (Pb > 1)Pb = 1;
//
//					//		Pb = 1;
//
//					int Gray_R = 255 * Pb;
//					DWORD Gray = _RGB(Gray_R, Gray_R, 0);
//
//					maxAng = angle[i];
//					bitmapEx.SetPixel((int)p[i].X, (int)p[i].Y, Gray);
//				}
//			}
//
//		}
//		mtx_demUseDataMem.unlock();
//		//	bitmapEx.FillWhiteKNN();
//		//	bitmapEx.rainClutter(locX, locY, 400);
//		//	bitmapEx.rainClutter(450, 450, 400);
//		//	bitmapEx.BilateralBlur(2);	//平均每加1多耗时1S
//		bitmapEx.Save(_T("../Radar/pic/DEM.bmp"));
//		Sleep(100);
//		DWORD d2 = GetTickCount();
//		DWORD diff1 = d2 - d1;
//
//		mtx_BMPmem.lock();
//		CBitmapEx bmp;
//		bmp.Create(bitmapEx);
//		bmp.Save(_T("../Radar/pic/DEM_mem.bmp"));
//		mtx_BMPmem.unlock();
//
//		DWORD d3 = GetTickCount();
//		DWORD diff2 = d3 - d2;
//
//	}
//	return 0;
//}

//UINT ThreadDrawDemBMP_1(LPVOID lpParam)	//矩形绘图,由dem根据Level提取的900*900数组	ThreadDrawMap1改进测试 正在使用
//{
//	CRadarView * pThis = (CRadarView*)lpParam;
//
//
//	//	long Posx, Posy;
//	//	int count;
//	//	DWORD white = 255;
//	//	DWORD black = 0;
//	//	int R = 255, G = 255, B = 255;
//	double t1 = 0.1;
//	double t2 = 1 - t1;
//	int zoom = 1;
//
//
//	while (1)
//	{
//		DWORD dwRetVal = WaitForSingleObject(closeWindowEvent, 100);
//		if (dwRetVal == WAIT_TIMEOUT)
//		{
//			mtx_demUseDataMem.lock();
//			DWORD d1 = GetTickCount();
//			CBitmapEx bitmapEx;
//			int demBMP_HEIGHT = demData_mem.dem_row;
//			int demBMP_WIDTH = demData_mem.dem_col;
//			int locY = demBMP_HEIGHT / 2;
//			int locX = demBMP_WIDTH / 2;
//			bitmapEx.Create(demBMP_WIDTH, demBMP_HEIGHT);
//			bitmapEx.Clear(_RGB(0, 0, 64));
//			double gain = (double)pThis->m_gainNum / 100 + 1;
//
//			if (demData_mem.pdemUseData == NULL)
//			{
//				mtx_demUseDataMem.unlock();
//				Sleep(500);
//				continue;
//			}
//
//			//--------测试-------------//
//			/*bool * bcheck = (bool*)malloc(demData_mem.dem_row*demData_mem.dem_col*sizeof(bool));
//			memset(bcheck, false, demData_mem.dem_row*demData_mem.dem_col*sizeof(bool));
//			bool * bcheck2 = (bool*)malloc(demData_mem.dem_row*demData_mem.dem_col*sizeof(bool));
//			memset(bcheck2, false, demData_mem.dem_row*demData_mem.dem_col*sizeof(bool));
//			int sumPoint = 0;
//			int sumAngle = 0;
//			int sumHeight[103] = { 0 };
//			int sumAng[93] = { 0 };*/
//			//--------------------------//
//
//			double locH = demData_mem.pdemUseData[locY*demBMP_WIDTH + locX]; //@本船所在地势高度
//			double locSumH = locH + RADAR_HIGHT;    //@本船雷达天线所在高度（加地势后）
//			int x = 0, y = 0;	//@当前扫面点坐标
//			double tarH;	//@扫描目标点高度
//			double obsH;  //@遮挡点高度
//			double DX = pThis->m_perPixelM;
//			double DY = pThis->m_perPixelM;
//			for (x = 0; x < demBMP_WIDTH; x++)
//			{
//				int w = x>locX ? (x - locX) : (locX - x);	//@直角三角形的横向边
//				int h = y > locY ? (y - locY) : (locY - y);	//@直角三角形的纵向边
//				int maxL = w > h ? w : h;
//				//		PointF p[demBMP_HEIGHT / 2 + 5];
//				//		double angle[demBMP_HEIGHT / 2 + 5];	//@雷达准线角
//				PointF *p = new PointF[demBMP_HEIGHT / 2 + 5];
//				double *angle = new double[demBMP_HEIGHT / 2 + 5];
//				double maxAng = 0;
//				int i;
//				for (i = maxL; i > 0; i--)
//				{
//					if (w > h)
//					{
//						int a = (int)((double)h*i / w + 0.5);
//						if (x > locX)
//						{
//							p[i].X = locX + i;
//						}
//						else
//						{
//							p[i].X = locX - i;
//						}
//						if (y > locY)
//						{
//							p[i].Y = locY + a;
//						}
//						else
//						{
//							p[i].Y = locY - a;
//						}
//					}
//					else
//					{
//						int a = (int)((double)w*i / h + 0.5);
//						if (x > locX)
//						{
//							p[i].X = locX + a;
//						}
//						else
//						{
//							p[i].X = locX - a;
//						}
//						if (y > locY)
//						{
//							p[i].Y = locY + i;
//						}
//						else
//						{
//							p[i].Y = locY - i;
//						}
//					}
//				}
//
//				//开始在一条扫描线上扫描
//				tarH = demData_mem.pdemUseData[(int)p[1].Y*demBMP_WIDTH + (int)p[1].X];
//				double R = zoom*sqrt((p[1].X - locX)*(p[1].X - locX)*DX*DX + (p[1].Y - locY)*(p[1].Y - locY)*DY*DY);	//@离雷达物理距离（米）
//
//				//----------------测试-----------------//
//				/*if (bcheck[(int)p[1].Y*demBMP_WIDTH + (int)p[1].X] == false)
//				{
//				sumPoint++;
//				bcheck[(int)p[1].Y*demBMP_WIDTH + (int)p[1].X] = true;
//				if (tarH > 100)
//				sumHeight[102]++;
//				else if (tarH < 0)
//				sumHeight[101]++;
//				else
//				sumHeight[(int)tarH]++;
//				}*/
//				//------------------------------------//	
//
//
//				if (tarH <= locSumH)
//				{
//					angle[1] = atan(R / (locSumH - tarH));
//				}
//				else
//				{
//					angle[1] = PI / 2 + atan((tarH - locSumH) / R);
//				}
//				maxAng = angle[1];
//				double Rmax = zoom*sqrt((p[maxL].X - locX)*(p[maxL].X - locX)*DX*DX + (p[maxL].Y - locY)*(p[maxL].Y - locY)*DY*DY);	//@扫描线像素长度
//
//				for (i = 2; i <= maxL; i++)
//				{
//					tarH = demData_mem.pdemUseData[(int)p[i].Y*demBMP_WIDTH + (int)p[i].X];
//					R = zoom*sqrt((p[i].X - locX)*(p[i].X - locX)*DX*DX + (p[i].Y - locY)*(p[i].Y - locY)*DY*DY);	//@离雷达物理距离（米）
//
//					//----------------测试-----------------//
//					/*if (bcheck[(int)p[i].Y*demBMP_WIDTH + (int)p[i].X] == false)
//					{
//					sumPoint++;
//					bcheck[(int)p[i].Y*demBMP_WIDTH + (int)p[i].X] = true;
//					if (tarH > 100)
//					sumHeight[102]++;
//					else if (tarH < 0)
//					sumHeight[101]++;
//					else
//					sumHeight[(int)tarH]++;
//					}*/
//					//------------------------------------//
//
//					if (tarH <= locH)	//当目标地势低于本船地势时，默认目标处于水域或无障碍物，不可反射电磁波
//					{
//						continue;
//					}
//					if (tarH <= locSumH)
//					{
//						angle[i] = atan(R / (locSumH - tarH));
//					}
//					else
//					{
//						angle[i] = PI / 2 + atan((tarH - locSumH) / R);
//					}
//					if (angle[i] >= maxAng)
//					{
//						//计算亮度
//						double k1 = (t1*atan(tarH / 10.0f)) / (PI / 2);
//						double slope = atan((tarH - demData_mem.pdemUseData[(int)p[i - 1].Y*demBMP_WIDTH + (int)p[i - 1].X]) / (zoom*sqrt((p[i].X - p[i - 1].X)*(p[i].X - p[i - 1].X)*DX*DX + (p[i].Y - p[i - 1].Y)*(p[i].Y - p[i - 1].Y)*DY*DY)));	//@斜角
//						double angleTrans = AngleTrans(angle[i] - slope);
//						double k2 = t2*cos(angleTrans);
//
//						//----------------测试-----------------//
//						/*if (bcheck2[(int)p[i].Y*demBMP_WIDTH + (int)p[i].X] == false)
//						{
//						sumAngle++;
//						bcheck2[(int)p[i].Y*demBMP_WIDTH + (int)p[i].X] = true;
//						double _ang = _ToAng(angle[i] - slope);
//						if (_ang > 90)
//						sumAng[92]++;
//						else if (_ang < 0)
//						sumAng[91]++;
//						else
//						sumAng[(int)_ang]++;
//						}*/
//						//------------------------------------//
//
//						//	double k2 = 10*t2*cos(angle[i] - slope);
//						double t = k1 + k2;
//						double Pb = t*(1 - (R / Rmax)*(R / Rmax)*(R / Rmax)*(R / Rmax));
//
//						if (Pb < 0)Pb = 0;
//						if (Pb > 1)Pb = 1;
//
//						//	Pb = 1;
//
//						int Gray_R = 255 * Pb*gain;
//						if (Gray_R > 255) Gray_R = 255;
//						DWORD Gray = _RGB(Gray_R, Gray_R, 0);
//
//						maxAng = angle[i];
//						bitmapEx.SetPixel((int)p[i].X, (int)p[i].Y, Gray);
//					}
//				}
//				delete[] p;
//				delete[] angle;
//			}
//			y = demBMP_HEIGHT - 1;
//			for (x = 0; x < demBMP_WIDTH; x++)
//			{
//				int w = x>locX ? (x - locX) : (locX - x);	//@直角三角形的横向边
//				int h = y > locY ? (y - locY) : (locY - y);	//@直角三角形的纵向边
//				int maxL = w > h ? w : h;
//				//		PointF p[demBMP_HEIGHT / 2 + 5];
//				//		double angle[demBMP_HEIGHT / 2 + 5];
//				PointF *p = new PointF[demBMP_HEIGHT / 2 + 5];
//				double *angle = new double[demBMP_HEIGHT / 2 + 5];
//				double maxAng = 0;
//				int i;
//				for (i = maxL; i > 0; i--)
//				{
//					if (w > h)
//					{
//						int a = (int)((double)h*i / w + 0.5);
//						if (x > locX)
//						{
//							p[i].X = locX + i;
//						}
//						else
//						{
//							p[i].X = locX - i;
//						}
//						if (y > locY)
//						{
//							p[i].Y = locY + a;
//						}
//						else
//						{
//							p[i].Y = locY - a;
//						}
//					}
//					else
//					{
//						int a = (int)((double)w*i / h + 0.5);
//						if (x > locX)
//						{
//							p[i].X = locX + a;
//						}
//						else
//						{
//							p[i].X = locX - a;
//						}
//						if (y > locY)
//						{
//							p[i].Y = locY + i;
//						}
//						else
//						{
//							p[i].Y = locY - i;
//						}
//					}
//				}
//
//				//开始在一条扫描线上扫描
//				tarH = demData_mem.pdemUseData[(int)p[1].Y*demBMP_WIDTH + (int)p[1].X];
//				double R = zoom*sqrt((p[1].X - locX)*(p[1].X - locX)*DX*DX + (p[1].Y - locY)*(p[1].Y - locY)*DY*DY);	//@离雷达物理距离（米）
//
//				//----------------测试-----------------//
//				/*if (bcheck[(int)p[1].Y*demBMP_WIDTH + (int)p[1].X] == false)
//				{
//				sumPoint++;
//				bcheck[(int)p[1].Y*demBMP_WIDTH + (int)p[1].X] = true;
//				if (tarH > 100)
//				sumHeight[102]++;
//				else if (tarH < 0)
//				sumHeight[101]++;
//				else
//				sumHeight[(int)tarH]++;
//				}*/
//				//------------------------------------//	
//
//				if (tarH <= locSumH)
//				{
//					angle[1] = atan(R / (locSumH - tarH));
//				}
//				else
//				{
//					angle[1] = PI / 2 + atan((tarH - locSumH) / R);
//				}
//				maxAng = angle[1];
//				double Rmax = zoom*sqrt((p[maxL].X - locX)*(p[maxL].X - locX)*DX*DX + (p[maxL].Y - locY)*(p[maxL].Y - locY)*DY*DY);	//@扫描线像素长度
//
//				for (i = 2; i <= maxL; i++)
//				{
//					tarH = demData_mem.pdemUseData[(int)p[i].Y*demBMP_WIDTH + (int)p[i].X];
//					R = zoom*sqrt((p[i].X - locX)*(p[i].X - locX)*DX*DX + (p[i].Y - locY)*(p[i].Y - locY)*DY*DY);	//@离雷达物理距离（米）
//
//					//----------------测试-----------------//
//					/*if (bcheck[(int)p[i].Y*demBMP_WIDTH + (int)p[i].X] == false)
//					{
//					sumPoint++;
//					bcheck[(int)p[i].Y*demBMP_WIDTH + (int)p[i].X] = true;
//					if (tarH > 100)
//					sumHeight[102]++;
//					else if (tarH < 0)
//					sumHeight[101]++;
//					else
//					sumHeight[(int)tarH]++;
//					}*/
//					//------------------------------------//
//
//					if (tarH <= locH)	//当目标地势低于本船地势时，默认目标处于水域或无障碍物，不可反射电磁波
//					{
//						continue;
//					}
//					if (tarH <= locSumH)
//					{
//						angle[i] = atan(R / (locSumH - tarH));
//					}
//					else
//					{
//						angle[i] = PI / 2 + atan((tarH - locSumH) / R);
//					}
//					if (angle[i] >= maxAng)
//					{
//						//计算亮度
//						double k1 = (t1*atan(tarH / 10.0f)) / (PI / 2);
//						double slope = atan((tarH - demData_mem.pdemUseData[(int)p[i - 1].Y*demBMP_WIDTH + (int)p[i - 1].X]) / (zoom*sqrt((p[i].X - p[i - 1].X)*(p[i].X - p[i - 1].X)*DX*DX + (p[i].Y - p[i - 1].Y)*(p[i].Y - p[i - 1].Y)*DY*DY)));	//@斜角
//						double angleTrans = AngleTrans(angle[i] - slope);
//						double k2 = t2*cos(angleTrans);
//
//						//----------------测试-----------------//
//						/*if (bcheck2[(int)p[i].Y*demBMP_WIDTH + (int)p[i].X] == false)
//						{
//						sumAngle++;
//						bcheck2[(int)p[i].Y*demBMP_WIDTH + (int)p[i].X] = true;
//						double _ang = _ToAng(angle[i] - slope);
//						if (_ang > 90)
//						sumAng[92]++;
//						else if (_ang < 0)
//						sumAng[91]++;
//						else
//						sumAng[(int)_ang]++;
//						}*/
//						//------------------------------------//
//
//						//	double k2 = 10*t2*cos(angle[i] - slope);
//						double t = k1 + k2;
//						double Pb = t*(1 - (R / Rmax)*(R / Rmax)*(R / Rmax)*(R / Rmax));
//
//						if (Pb < 0)Pb = 0;
//						if (Pb > 1)Pb = 1;
//
//						//	Pb = 1;
//
//						int Gray_R = 255 * Pb*gain;
//						if (Gray_R > 255) Gray_R = 255;
//						DWORD Gray = _RGB(Gray_R, Gray_R, 0);
//
//						maxAng = angle[i];
//						bitmapEx.SetPixel((int)p[i].X, (int)p[i].Y, Gray);
//					}
//				}
//				delete[] p;
//				delete[] angle;
//			}
//			x = 0;
//			for (y = 0; y < demBMP_HEIGHT; y++)
//			{
//				int w = x>locX ? (x - locX) : (locX - x);	//@直角三角形的横向边
//				int h = y > locY ? (y - locY) : (locY - y);	//@直角三角形的纵向边
//				int maxL = w > h ? w : h;
//				/*PointF p[demBMP_WIDTH / 2 + 5];
//				double angle[demBMP_HEIGHT / 2 + 5];*/
//				PointF *p = new PointF[demBMP_WIDTH / 2 + 5];
//				double *angle = new double[demBMP_HEIGHT / 2 + 5];
//				double maxAng = 0;
//				int i;
//				for (i = maxL; i > 0; i--)
//				{
//					if (w > h)
//					{
//						int a = (int)((double)h*i / w + 0.5);
//						if (x > locX)
//						{
//							p[i].X = locX + i;
//						}
//						else
//						{
//							p[i].X = locX - i;
//						}
//						if (y > locY)
//						{
//							p[i].Y = locY + a;
//						}
//						else
//						{
//							p[i].Y = locY - a;
//						}
//					}
//					else
//					{
//						int a = (int)((double)w*i / h + 0.5);
//						if (x > locX)
//						{
//							p[i].X = locX + a;
//						}
//						else
//						{
//							p[i].X = locX - a;
//						}
//						if (y > locY)
//						{
//							p[i].Y = locY + i;
//						}
//						else
//						{
//							p[i].Y = locY - i;
//						}
//					}
//				}
//				//开始在一条扫描线上扫描
//				tarH = demData_mem.pdemUseData[(int)p[1].Y*demBMP_WIDTH + (int)p[1].X];
//				double R = zoom*sqrt((p[1].X - locX)*(p[1].X - locX)*DX*DX + (p[1].Y - locY)*(p[1].Y - locY)*DY*DY);	//@离雷达物理距离（米）
//
//				//----------------测试-----------------//
//				/*if (bcheck[(int)p[1].Y*demBMP_WIDTH + (int)p[1].X] == false)
//				{
//				sumPoint++;
//				bcheck[(int)p[1].Y*demBMP_WIDTH + (int)p[1].X] = true;
//				if (tarH > 100)
//				sumHeight[102]++;
//				else if (tarH < 0)
//				sumHeight[101]++;
//				else
//				sumHeight[(int)tarH]++;
//				}*/
//				//------------------------------------//	
//
//				if (tarH <= locSumH)
//				{
//					angle[1] = atan(R / (locSumH - tarH));
//				}
//				else
//				{
//					angle[1] = PI / 2 + atan((tarH - locSumH) / R);
//				}
//				maxAng = angle[1];
//				double Rmax = zoom*sqrt((p[maxL].X - locX)*(p[maxL].X - locX)*DX*DX + (p[maxL].Y - locY)*(p[maxL].Y - locY)*DY*DY);	//@扫描线像素长度
//
//				for (i = 2; i <= maxL; i++)
//				{
//					tarH = demData_mem.pdemUseData[(int)p[i].Y*demBMP_WIDTH + (int)p[i].X];
//					R = zoom*sqrt((p[i].X - locX)*(p[i].X - locX)*DX*DX + (p[i].Y - locY)*(p[i].Y - locY)*DY*DY);	//@离雷达物理距离（米）
//
//
//					//----------------测试-----------------//
//					/*if (bcheck[(int)p[i].Y*demBMP_WIDTH + (int)p[i].X] == false)
//					{
//					sumPoint++;
//					bcheck[(int)p[i].Y*demBMP_WIDTH + (int)p[i].X] = true;
//					if (tarH > 100)
//					sumHeight[102]++;
//					else if (tarH < 0)
//					sumHeight[101]++;
//					else
//					sumHeight[(int)tarH]++;
//					}*/
//					//------------------------------------//
//
//					if (tarH <= locH)	//当目标地势低于本船地势时，默认目标处于水域或无障碍物，不可反射电磁波
//					{
//						continue;
//					}
//					if (tarH <= locSumH)
//					{
//						angle[i] = atan(R / (locSumH - tarH));
//					}
//					else
//					{
//						angle[i] = PI / 2 + atan((tarH - locSumH) / R);
//					}
//					if (angle[i] >= maxAng)
//					{
//						//计算亮度
//						double k1 = (t1*atan(tarH / 10.0f)) / (PI / 2);
//						double slope = atan((tarH - demData_mem.pdemUseData[(int)p[i - 1].Y*demBMP_WIDTH + (int)p[i - 1].X]) / (zoom*sqrt((p[i].X - p[i - 1].X)*(p[i].X - p[i - 1].X)*DX*DX + (p[i].Y - p[i - 1].Y)*(p[i].Y - p[i - 1].Y)*DY*DY)));	//@斜角
//						double angleTrans = AngleTrans(angle[i] - slope);
//						double k2 = t2*cos(angleTrans);
//
//						//----------------测试-----------------//
//						/*if (bcheck2[(int)p[i].Y*demBMP_WIDTH + (int)p[i].X] == false)
//						{
//						sumAngle++;
//						bcheck2[(int)p[i].Y*demBMP_WIDTH + (int)p[i].X] = true;
//						double _ang = _ToAng(angle[i] - slope);
//						if (_ang > 90)
//						sumAng[92]++;
//						else if (_ang < 0)
//						sumAng[91]++;
//						else
//						sumAng[(int)_ang]++;
//						}*/
//						//------------------------------------//
//
//						//	double k2 = 10*t2*cos(angle[i] - slope);
//						double t = k1 + k2;
//						double Pb = t*(1 - (R / Rmax)*(R / Rmax)*(R / Rmax)*(R / Rmax));
//
//						if (Pb < 0)Pb = 0;
//						if (Pb > 1)Pb = 1;
//
//						//		Pb = 1;
//
//						int Gray_R = 255 * Pb*gain;
//						if (Gray_R > 255) Gray_R = 255;
//						DWORD Gray = _RGB(Gray_R, Gray_R, 0);
//
//						maxAng = angle[i];
//						bitmapEx.SetPixel((int)p[i].X, (int)p[i].Y, Gray);
//					}
//				}
//				delete[] p;
//				delete[] angle;
//			}
//			x = demBMP_WIDTH - 1;
//			for (y = 0; y < demBMP_HEIGHT; y++)
//			{
//				int w = x>locX ? (x - locX) : (locX - x);	//@直角三角形的横向边
//				int h = y > locY ? (y - locY) : (locY - y);	//@直角三角形的纵向边
//				int maxL = w > h ? w : h;
//				/*PointF p[demBMP_WIDTH / 2 + 5];
//				double angle[demBMP_HEIGHT / 2 + 5];*/
//				PointF *p = new PointF[demBMP_WIDTH / 2 + 5];
//				double *angle = new double[demBMP_HEIGHT / 2 + 5];
//				double maxAng = 0;
//				int i;
//				for (i = maxL; i > 0; i--)
//				{
//					if (w > h)
//					{
//						int a = (int)((double)h*i / w + 0.5);
//						if (x > locX)
//						{
//							p[i].X = locX + i;
//						}
//						else
//						{
//							p[i].X = locX - i;
//						}
//						if (y > locY)
//						{
//							p[i].Y = locY + a;
//						}
//						else
//						{
//							p[i].Y = locY - a;
//						}
//					}
//					else
//					{
//						int a = (int)((double)w*i / h + 0.5);
//						if (x > locX)
//						{
//							p[i].X = locX + a;
//						}
//						else
//						{
//							p[i].X = locX - a;
//						}
//						if (y > locY)
//						{
//							p[i].Y = locY + i;
//						}
//						else
//						{
//							p[i].Y = locY - i;
//						}
//					}
//				}
//				//开始在一条扫描线上扫描
//				tarH = demData_mem.pdemUseData[(int)p[1].Y*demBMP_WIDTH + (int)p[1].X];
//				double R = zoom*sqrt((p[1].X - locX)*(p[1].X - locX)*DX*DX + (p[1].Y - locY)*(p[1].Y - locY)*DY*DY);	//@离雷达物理距离（米）
//
//				//----------------测试-----------------//
//				/*if (bcheck[(int)p[1].Y*demBMP_WIDTH + (int)p[1].X] == false)
//				{
//				sumPoint++;
//				bcheck[(int)p[1].Y*demBMP_WIDTH + (int)p[1].X] = true;
//				if (tarH > 100)
//				sumHeight[102]++;
//				else if (tarH < 0)
//				sumHeight[101]++;
//				else
//				sumHeight[(int)tarH]++;
//				}*/
//				//------------------------------------//	
//
//				if (tarH <= locSumH)
//				{
//					angle[1] = atan(R / (locSumH - tarH));
//				}
//				else
//				{
//					angle[1] = PI / 2 + atan((tarH - locSumH) / R);
//				}
//				maxAng = angle[1];
//				double Rmax = zoom*sqrt((p[maxL].X - locX)*(p[maxL].X - locX)*DX*DX + (p[maxL].Y - locY)*(p[maxL].Y - locY)*DY*DY);	//@扫描线像素长度
//
//				for (i = 2; i <= maxL; i++)
//				{
//					tarH = demData_mem.pdemUseData[(int)p[i].Y*demBMP_WIDTH + (int)p[i].X];
//					R = zoom*sqrt((p[i].X - locX)*(p[i].X - locX)*DX*DX + (p[i].Y - locY)*(p[i].Y - locY)*DY*DY);	//@离雷达物理距离（米）
//
//					//----------------测试-----------------//
//					/*if (bcheck[(int)p[i].Y*demBMP_WIDTH + (int)p[i].X] == false)
//					{
//					sumPoint++;
//					bcheck[(int)p[i].Y*demBMP_WIDTH + (int)p[i].X] = true;
//					if (tarH > 100)
//					sumHeight[102]++;
//					else if (tarH < 0)
//					sumHeight[101]++;
//					else
//					sumHeight[(int)tarH]++;
//					}*/
//					//------------------------------------//
//
//					if (tarH <= locH)	//当目标地势低于本船地势时，默认目标处于水域或无障碍物，不可反射电磁波
//					{
//						continue;
//					}
//					if (tarH <= locSumH)
//					{
//						angle[i] = atan(R / (locSumH - tarH));
//					}
//					else
//					{
//						angle[i] = PI / 2 + atan((tarH - locSumH) / R);
//					}
//					if (angle[i] >= maxAng)
//					{
//						//计算亮度
//						double k1 = (t1*atan(tarH / 10.0f)) / (PI / 2);
//						double slope = atan((tarH - demData_mem.pdemUseData[(int)p[i - 1].Y*demBMP_WIDTH + (int)p[i - 1].X]) / (zoom*sqrt((p[i].X - p[i - 1].X)*(p[i].X - p[i - 1].X)*DX*DX + (p[i].Y - p[i - 1].Y)*(p[i].Y - p[i - 1].Y)*DY*DY)));	//@斜角
//						double angleTrans = AngleTrans(angle[i] - slope);
//						double k2 = t2*cos(angleTrans);
//
//						//----------------测试-----------------//
//						/*if (bcheck2[(int)p[i].Y*demBMP_WIDTH + (int)p[i].X] == false)
//						{
//						sumAngle++;
//						bcheck2[(int)p[i].Y*demBMP_WIDTH + (int)p[i].X] = true;
//						double _ang = _ToAng(angle[i] - slope);
//						if (_ang > 90)
//						sumAng[92]++;
//						else if (_ang < 0)
//						sumAng[91]++;
//						else
//						sumAng[(int)_ang]++;
//						}*/
//						//------------------------------------//
//
//						//	double k2 = 10*t2*cos(angle[i] - slope);
//						double t = k1 + k2;
//						double Pb = t*(1 - (R / Rmax)*(R / Rmax)*(R / Rmax)*(R / Rmax));
//
//
//
//						if (Pb < 0)Pb = 0;
//						if (Pb > 1)Pb = 1;
//
//						//		Pb = 1;
//
//						int Gray_R = 255 * Pb*gain;
//						if (Gray_R > 255) Gray_R = 255;
//						DWORD Gray = _RGB(Gray_R, Gray_R, 0);
//
//						maxAng = angle[i];
//						bitmapEx.SetPixel((int)p[i].X, (int)p[i].Y, Gray);
//					}
//				}
//				delete[] p;
//				delete[] angle;
//			}
//
//
//			//---------------测试----------------//
//			/*std::ofstream ofile1(_T("C:/Users/Administrator/Desktop/1.txt"));
//			std::ofstream ofile2(_T("C:/Users/Administrator/Desktop/2.txt"));
//			ofile1.precision(5);
//			ofile2.precision(5);
//			int sum_1 = 0, sum_2 = 0;
//			for (int cont = 0; cont < 103; cont++)
//			{
//			sum_1 += sumHeight[cont];
//			ofile1 << (double)sumHeight[cont] / (double)sumPoint << std::endl;
//			}
//			ofile1.close();
//			for (int cont = 0; cont < 93; cont++)
//			{
//			sum_2 += sumAng[cont];
//			ofile2 << (double)sumAng[cont] / (double)sumAngle << std::endl;
//			}
//			ofile2.close();
//			if (bcheck != NULL)
//			{
//			free(bcheck);
//			bcheck = NULL;
//			}
//			if (bcheck2 != NULL)
//			{
//			free(bcheck2);
//			bcheck2 = NULL;
//			}*/
//			//---------------------------------//
//
//			mtx_demUseDataMem.unlock();
//			//	bitmapEx.FillWhiteKNN();
//			//	bitmapEx.rainClutter(locX, locY, 400);
//			//	bitmapEx.rainClutter(450, 450, 400);
//			//	bitmapEx.BilateralBlur(2);	//平均每加1多耗时1S
//			bitmapEx.Save(_T("../Radar/pic/DEM.bmp"));
//			//	Sleep(100);
//			DWORD d2 = GetTickCount();
//			DWORD diff1 = d2 - d1;
//
//
//			CBitmapEx bmp;
//			bmp.Create(pThis->m_radiusP * 2, pThis->m_radiusP * 2);
//			bmp.Draw(0, 0, bmp.GetWidth(), bmp.GetHeight(), bitmapEx, 0, 0, bitmapEx.GetWidth(), bitmapEx.GetHeight());
//			bmp.BilateralBlur(2);	//默认为2
//			mtx_BMPmem.lock();
//			bmp.Save(_T("../Radar/pic/DEM_mem.bmp"));
//			mtx_BMPmem.unlock();
//
//			DWORD d3 = GetTickCount();
//			DWORD diff2 = d3 - d2;
//		}
//		else
//		{
//			break;
//		}
//		Sleep(100);
//	}
//	SetEvent(closeThread_drawDemBMP);
//	return 0;
//}