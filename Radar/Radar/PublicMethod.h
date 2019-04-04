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
	Tile,//ƽ��
	Center,//����
	Stretch,//����
	Fit, //��Ӧ
	Fill//���
};
HRESULT SetDesktopWallpaper(PWSTR pszFile, WallpaperStyle style);
void changePicture(std::string PictureFullpath);

//��˹ͶӰ�ɾ�γ��(Unit:DD)����������(�����ţ�Unit:Metres)
void GaussProjCal(double longitude, double latitude, double *X, double *Y);
//��˹ͶӰ�ɴ������(Unit:Metres)���㾭γ��(Unit:DD) 
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
void RotatePoint(int *x, int *y, CPoint center, double angle);			//��һ����˳ʱ����תangle��
CPoint RotatePoint(int x, int y, CPoint center, double angle);
double AngleTrans(double angle);
void WebMercatorProjCal(double lng, double lat, double &dadiX, double &dadiY);//Webī������ͶӰ����γ��תƽ�����꣩
void WebMercatorProjInvcal(double dadiX, double dadiY, double &lng, double &lat);//Webī������ͶӰ��ƽ������ת��γ�ȣ�
double _ToRad(double a);
double _ToAng(double a);

void GetPixelPointFromGeo(int &destX, int &destY, double destLng, double destLat, int srcX, int srcY, double srcLng, double srcLat, double scale);
void GetGeograFromScrPoint(int destX, int destY, double &destLng, double &destLat, int srcX, int srcY, double srcLng, double srcLat, double scale);
//void GetGeodeFromScrPoint(int destX, int destY, double &destDadiX, double &destDadiY, int srcX, int srcY, double srcDadiX, double srcDadiY);
void GetPixelPointFromDadi(int &destX, int &destY, double destDadiX, double destDadiY, int srcX, int srcY, double srcDadiX, double srcDadiY, double scale);