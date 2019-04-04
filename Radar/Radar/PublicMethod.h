#pragma once
#include"stdafx.h"
#include<fstream>

#define PI 3.1415926

extern HANDLE closeWindowEvent;
extern HANDLE closeThread_loadDemFile_event;
extern HANDLE closeThread_threadCalculateARPARecord;
extern HANDLE closeThread_drawDemBMP;
extern HANDLE rangeUpdate;

enum WallpaperStyle
{
	Tile,//平铺
	Center,//居中
	Stretch,//拉伸
	Fit, //适应
	Fill//填充
};
HRESULT SetDesktopWallpaper(PWSTR pszFile, WallpaperStyle style);
void changePicture(std::string PictureFullpath);

//高斯投影由经纬度(Unit:DD)反算大地坐标(含带号，Unit:Metres)
void GaussProjCal(double longitude, double latitude, double *X, double *Y);
//高斯投影由大地坐标(Unit:Metres)反算经纬度(Unit:DD) 
void GaussProjInvCal(double X, double Y, double *longitude, double *latitude);
bool get_integer(char ** buffer, int &result);
void get_string(char** buffer, char * str, int maxSize);
bool get_float(char** buffer, double &result);
int m_itoa(char * str, long num, int maxSize_str);
void m_itoa_rec(char * str, long num, int &strlen, int maxSize_str);
size_t getFileAllSize(std::ifstream * file);
size_t getFileLastSize(std::ifstream * file);
double modulus_f(double a, double b);
double getLinearinterpolationOf4Point(double leftUpX, double leftUpY, double leftUpH, double rightUpX, double rightUpY, double rightUpH,
	double leftDownX, double leftDownY, double leftDownH, double rightDownX, double rightDownY, double rightDownH, double destX, double destY);
bool isPointInEllipse(Gdiplus::Rect rect, double x, double y);
void TranslatePoint(int &x, int &y, int translateX, int translateY);
void RotatePoint(int *x, int *y, CPoint center, double angle);			//将一个点顺时针旋转angle角
CPoint RotatePoint(int x, int y, CPoint center, double angle);
double AngleTrans(double angle);
void WebMercatorProjCal(double lng, double lat, double &dadiX, double &dadiY);//Web墨卡托正投影（经纬度转平面坐标）
void WebMercatorProjInvcal(double dadiX, double dadiY, double &lng, double &lat);//Web墨卡托逆投影（平面坐标转经纬度）
double _ToRad(double a);
double _ToAng(double a);

void GetPixelPointFromGeo(int &destX, int &destY, double destLng, double destLat, int srcX, int srcY, double srcLng, double srcLat, double scale);
void GetGeograFromScrPoint(int destX, int destY, double &destLng, double &destLat, int srcX, int srcY, double srcLng, double srcLat, double scale);
//void GetGeodeFromScrPoint(int destX, int destY, double &destDadiX, double &destDadiY, int srcX, int srcY, double srcDadiX, double srcDadiY);
void GetPixelPointFromDadi(int &destX, int &destY, double destDadiX, double destDadiY, int srcX, int srcY, double srcDadiX, double srcDadiY, double scale);