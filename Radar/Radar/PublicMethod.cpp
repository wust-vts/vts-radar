#include"stdafx.h"
#include"PublicMethod.h"
#include<math.h>
#include<iostream>

using namespace std;

#define PI 3.1415927

//////6�ȴ���   54�걱������ϵ  
//��˹ͶӰ�ɾ�γ��(Unit:DD)����������(�����ţ�Unit:Metres)
void GaussProjCal(double longitude, double latitude, double *X, double *Y)
{
	int ProjNo = 0;
	int ZoneWide;   ////����  
	double  longitude1, latitude1, longitude0, latitude0, X0, Y0, xval, yval;
	double  a, f, e2, ee, NN, T, C, A, M, iPI;
	iPI = 0.0174532925199433;   ////3.1415926535898/180.0;   
	ZoneWide = 6;     ////6�ȴ���  
	a = 6378245.0;
	f = 1.0 / 298.3;      //54�걱������ϵ���� 
	//a=6378140.0;   f=1/298.257;      //80����������ϵ����  
	ProjNo = (int)(longitude / ZoneWide);

	longitude0 = ProjNo * ZoneWide + ZoneWide / 2;
	longitude0 = longitude0 * iPI;
	latitude0 = 0;
	longitude1 = longitude * iPI;   //����ת��Ϊ����  
	latitude1 = latitude * iPI;     //γ��ת��Ϊ���� 
	e2 = 2 * f - f*f;
	ee = e2*(1.0 - e2);
	NN = a / sqrt(1.0 - e2*sin(latitude1)*sin(latitude1));
	T = tan(latitude1)*tan(latitude1);
	C = ee*cos(latitude1)*cos(latitude1);
	A = (longitude1 - longitude0)*cos(latitude1);
	M = a*((1 - e2 / 4 - 3 * e2*e2 / 64 - 5 * e2*e2*e2 / 256)*latitude1 - (3 * e2 / 8 + 3 * e2*e2 / 32 + 45 * e2*e2*e2 / 1024)*sin(2 * latitude1)
		+ (15 * e2*e2 / 256 + 45 * e2*e2*e2 / 1024)*sin(4 * latitude1) - (35 * e2*e2*e2 / 3072)*sin(6 * latitude1));
	xval = NN*(A + (1 - T + C)*A*A*A / 6 + (5 - 18 * T + T*T + 72 * C - 58 * ee)*A*A*A*A*A / 120);
	yval = M + NN*tan(latitude1)*(A*A / 2 + (5 - T + 9 * C + 4 * C*C)*A*A*A*A / 24 + (61 - 58 * T + T*T + 600 * C - 330 * ee)*A*A*A*A*A*A / 720);
	X0 = 1000000L * (ProjNo + 1) + 500000L;
	Y0 = 0;
	xval = xval + X0;
	yval = yval + Y0;
	*X = xval;
	*Y = yval;
}

//��˹ͶӰ�ɴ������(Unit:Metres)���㾭γ��(Unit:DD) 
void GaussProjInvCal(double X, double Y, double *longitude, double *latitude)
{
	int ProjNo;
	int ZoneWide;   ////����  
	double longitude1, latitude1, longitude0, latitude0, X0, Y0, xval, yval;
	double e1, e2, f, a, ee, NN, T, C, M, D, R, u, fai, iPI;
	iPI = 0.0174532925199433;   ////3.1415926535898/180.0;   
	a = 6378245.0;
	f = 1.0 / 298.3;    //54�걱������ϵ����
	//a=6378140.0;   f=1/298.257;      //80����������ϵ����   
	ZoneWide = 6;     ////6�ȴ���  
	ProjNo = (int)(X / 1000000L);    //���Ҵ��� 
	longitude0 = (ProjNo - 1) * ZoneWide + ZoneWide / 2;
	longitude0 = longitude0 * iPI;     //���뾭��  
	X0 = ProjNo * 1000000L + 500000L;
	Y0 = 0;
	xval = X - X0;
	yval = Y - Y0;      //���ڴ������ 

	e2 = 2 * f - f*f;
	e1 = (1.0 - sqrt(1 - e2)) / (1.0 + sqrt(1 - e2));
	ee = e2 / (1 - e2);
	M = yval;
	u = M / (a*(1 - e2 / 4 - 3 * e2*e2 / 64 - 5 * e2*e2*e2 / 256));
	fai = u + (3 * e1 / 2 - 27 * e1*e1*e1 / 32)*sin(2 * u) + (21 * e1*e1 / 16 - 55 * e1*e1*e1*e1 / 32)*sin(4 * u)
		+ (151 * e1*e1*e1 / 96)*sin(6 * u) + (1097 * e1*e1*e1*e1 / 512)*sin(8 * u);
	C = ee*cos(fai)*cos(fai);
	T = tan(fai)*tan(fai);
	NN = a / sqrt(1.0 - e2*sin(fai)*sin(fai));
	R = a*(1 - e2) / sqrt((1 - e2*sin(fai)*sin(fai))*(1 - e2*sin(fai)*sin(fai))*(1 - e2*sin(fai)*sin(fai)));
	D = xval / NN;
	//���㾭��(Longitude) γ��(Latitude) 
	longitude1 = longitude0 + (D - (1 + 2 * T + C)*D*D*D / 6 + (5 - 2 * C + 28 * T - 3 * C*C + 8 * ee + 24 * T*T)*D*D*D*D*D / 120) / cos(fai);
	latitude1 = fai - (NN*tan(fai) / R)*(D*D / 2 - (5 + 3 * T + 10 * C - 4 * C*C - 9 * ee)*D*D*D*D / 24
		+ (61 + 90 * T + 298 * C + 45 * T*T - 256 * ee - 3 * C*C)*D*D*D*D*D*D / 720);   //ת��Ϊ�� DD 
	*longitude = longitude1 / iPI;
	*latitude = latitude1 / iPI;
}


/**
 * Webī������ͶӰ����γ��תƽ�����꣩
 * lng:����
 * lat:γ��
 * dadiX:ƽ������(�������)X
 * dadiY:ƽ������(�������)Y
**/
void WebMercatorProjCal(double lng, double lat, double &dadiX, double &dadiY)
{
	double x = lng *20037508.34 / 180;
	double y = log(tan((90 + lat)*PI / 360)) / (PI / 180);
	y = y *20037508.34 / 180;
	dadiX = x;
	dadiY = y;
}

/**
 * Webī������ͶӰ��ƽ������ת��γ�ȣ�
 * dadiX:ƽ������(�������)X
 * dadiY:ƽ������(�������)Y
 * lng:����
 * lat:γ��
**/
void WebMercatorProjInvcal(double dadiX, double dadiY, double &lng, double &lat)
{
	double x = dadiX / 20037508.34 * 180;
	double y = dadiY / 20037508.34 * 180;
	y = 180 / PI*(2 * atan(exp(y*PI / 180)) - PI / 2);
	lng = x;
	lat = y;
}

bool get_integer(char ** buffer, int &result)
{
	result = 0;
	while ((**buffer) < 48 || (**buffer) > 57)
	{
		if (**buffer == '\0')
			return false;
		(*buffer)++;
	}
	while ((**buffer) >= 48 && (**buffer) <= 57)
	{
		result *= 10;
		result += **buffer - 48;
		(*buffer)++;
	}
	return true;
}

//��buffer��ȡһ���ַ��� ����������� ��󳤶�20
void get_string(char** buffer,char * str,int maxSize)
{
	int i = 0;
	while ((**buffer) == ' ' || (**buffer) == '\t' || (**buffer) == '\n')
	{
		(*buffer)++;
	}
	while ((**buffer) != ' ' && (**buffer) != '\t' && (**buffer) != '\n' && (**buffer) != EOF && (**buffer) != '\0')
	{
		str[i++] = **buffer;
		(*buffer)++;
		if (i >=maxSize-1)
			break;
	}
	str[i] = '\0';
}

bool get_float(char** buffer, double &result)
{
	result = 0;
	double dfs = 0.1;
	while ((**buffer) < 48 || (**buffer) > 57)
	{
		if (**buffer == '\0')
			return false;
		(*buffer)++;
	}
	while ((**buffer) >= 48 && (**buffer) <= 57)
	{
		result *= 10;
		result += **buffer - 48;
		(*buffer)++;
	}
	if ((**buffer) == '.')
	{
		(*buffer)++;
		//����С������
		while ((**buffer) >= 48 && (**buffer) <= 57)
		{
			result += (double)(**buffer - 48)*dfs;
			dfs /= 10;
			(*buffer)++;
		}
	}
	return true;
}

//����ת�ַ����������ַ�����ʵ�ʳ���
int m_itoa(char * str,long num,int maxSize_str)
{
	int i = 0;
	m_itoa_rec(str, num, i, maxSize_str-1);
	str[i] = '\0';
	return i;
}

void m_itoa_rec(char * str,long num,int &strlen,int maxSize_str)
{
	if (num == 0 || maxSize_str ==0)
		return;
	m_itoa_rec(str,num / 10, strlen,maxSize_str-1);
	str[strlen] = num % 10+'0';
	strlen++;
}

//��ȡ�ļ���ȫ������
size_t getFileAllSize(ifstream * file)
{
	size_t temp = file->tellg(); // ��¼�µ�ǰλ��
	file->seekg(0, ios_base::end); // �ƶ����ļ�β
	size_t size = file->tellg(); // ȡ�õ�ǰλ�õ�ָ�볤�� ���ļ�����
	file->seekg(temp); // �ƶ���ԭ����λ��
	return size;
}

//��ȡ�ļ���ʣ�³���
size_t getFileLastSize(ifstream * file)
{
	size_t begin = file->tellg(); // ��¼�µ�ǰλ��
	file->seekg(0, ios_base::end); // �ƶ����ļ�β
	size_t end = file->tellg(); // ȡ�õ�ǰλ�õ�ָ�볤�� ���ļ�����
	file->seekg(begin); // �ƶ���ԭ����λ��
	return (end - begin);
}


//��a%b
double modulus_f(double a, double b)
{
	int i = a / b;
	return (double)(a - (i*b));
}

//��ȡ�ĸ�������Բ�ֵ���(4����Ϊ����)
double getLinearinterpolationOf4Point(double leftUpX, double leftUpY, double leftUpH, double rightUpX, double rightUpY, double rightUpH,
	double leftDownX, double leftDownY, double leftDownH, double rightDownX, double rightDownY, double rightDownH,double destX,double destY)
{
	double h1 = (rightUpH - leftUpH)*((destX - leftUpX) / (rightUpX - leftUpX))+leftUpH;
	double h2 = (rightDownH - leftDownH)*((destX - leftDownX) / (rightDownX - leftDownX))+leftDownH;
	double x1 = destX;
	double y1 = leftUpY;
	double x2 = destX;
	double y2 = leftDownY;
	double h = (h2 - h1)*((destY-y1)/(y2-y1)) + h1;
	return h;
}

//�ж�һ�����Ƿ�λ��һ������Ӿ���ȷ������Բ��
bool isPointInEllipse(Gdiplus::Rect rect, double x, double y)
{
	double centerX = rect.X + (double)rect.Width / 2;
	double centerY = rect.Y + (double)rect.Height / 2;
	double a = (double)rect.Width / 2;
	double b = (double)rect.Height / 2;
	double num1 = (x - centerX) / a;
	double num2 = (y - centerY) / b;
	if ((num1*num1 + num2*num2) > 1)
		return false;
	else
		return true;
}

//ƽ��һ����
void TranslatePoint(int &x, int &y, int translateX, int translateY)
{
	x += translateX;
	y += translateY;
}

CPoint RotatePoint(int x, int y, CPoint center, double angle)
{
	CPoint p(x, y);
	if (angle == 0)
		return p;
	CPoint p1(0, -1), p2(x, y);
	p2 = p2 - center;
	double l = sqrt(p2.x*p2.x + p2.y*p2.y);
	double a;
	a = acos(((double)-p2.y) / l);
	if (p2.x < 0)
		a = 2 * PI - a;
	a += angle;
	if (a>2 * PI)
		a -= 2 * PI;
	p.x = center.x + l*sin(a);
	p.y = center.y - l*cos(a);
	return p;
}

void RotatePoint(int *x, int *y, CPoint center, double angle)
{
	if (angle == 0)
		return;
	CPoint p1(0, -1), p2(*x, *y);
	p2 = p2 - center;
	double l = sqrt(p2.x*p2.x + p2.y*p2.y);
	double a;
	a = acos(((double)-p2.y) / l);
	if (p2.x < 0)
		a = 2 * PI - a;
	a += angle;
	if (a>2 * PI)
		a -= 2 * PI;
	*x = center.x + l*sin(a);
	*y = center.y - l*cos(a);
}

double AngleTrans(double angle)
{
	return pow(angle, 30) / 486941.1;
	//	return pow(angle, 20) / 5324.5;
	//return (pow(1000, angle) - 1) / 32829.2;
}

HRESULT SetDesktopWallpaper(PWSTR pszFile, WallpaperStyle style)
{
	HRESULT hr = S_OK;

	//���ñ�ֽ����չ����ʽ
	//��Control Panel\Desktop�е�������ֵ��������
	// TileWallpaper
	//  0: ͼƬ����ƽ�� 
	//  1: ��ƽ�� 
	// WallpaperStyle
	//  0:  0��ʾͼƬ���У�1��ʾƽ��
	//  2:  �������������Ļ
	//  6:  ������Ӧ��Ļ�����ָ߶ȱ�
	//  10: ͼƬ��������С�ü���Ӧ��Ļ�����ݺ��

	//�Կɶ���д�ķ�ʽ��HKCU\Control Panel\Desktopע�����
	HKEY hKey = NULL;
	hr = HRESULT_FROM_WIN32(RegOpenKeyEx(HKEY_CURRENT_USER,
		L"Control Panel\\Desktop", 0, KEY_READ | KEY_WRITE, &hKey));
	if (SUCCEEDED(hr))
	{
		PWSTR pszWallpaperStyle = L"2";
		PWSTR pszTileWallpaper = L"0";

		switch (style)
		{
		case Tile:
			pszWallpaperStyle = L"0";
			pszTileWallpaper = L"1";
			break;

		case Center:
			pszWallpaperStyle = L"0";
			pszTileWallpaper = L"0";
			break;

		case Stretch:
			pszWallpaperStyle = L"2";
			pszTileWallpaper = L"0";
			break;

		case Fit: // (Windows 7 and later)
			pszWallpaperStyle = L"6";
			pszTileWallpaper = L"0";
			break;

		case Fill: // (Windows 7 and later)
			pszWallpaperStyle = L"10";
			pszTileWallpaper = L"0";
			break;
		}
		// ���� WallpaperStyle �� TileWallpaper ��ע�����.
		DWORD cbData = lstrlen(pszWallpaperStyle) * sizeof(*pszWallpaperStyle);
		hr = HRESULT_FROM_WIN32(RegSetValueEx(hKey, L"WallpaperStyle", 0, REG_SZ,
			reinterpret_cast<const BYTE *>(pszWallpaperStyle), cbData));
		if (SUCCEEDED(hr))
		{
			cbData = lstrlen(pszTileWallpaper) * sizeof(*pszTileWallpaper);
			hr = HRESULT_FROM_WIN32(RegSetValueEx(hKey, L"TileWallpaper", 0, REG_SZ,
				reinterpret_cast<const BYTE *>(pszTileWallpaper), cbData));
		}

		RegCloseKey(hKey);
	}

	//ͨ������Win32 API����SystemParametersInfo ���������ֽ
	/************************************************
	֮ǰ�����Ѿ������˱�ֽ�����ͣ����Ǳ�ֽͼƬ��ʵ���ļ�·����û
	���á�SystemParametersInfo �������λ��user32.dll�У���֧��
	���Ǵ�ϵͳ�л��Ӳ����������Ϣ�������ĸ���������һ��ָ��������
	��������Ҫִ�еĲ���������������������ָ����Ҫ���õ����ݣ�����
	��һ���������趨�����һ������ָ���ı��Ƿ񱻱��档�����һ������
	����Ӧָ��SPI_SETDESKWALLPAPER��ָ��������Ҫ���ñ�ֽ����������
	�ļ�·������Vista֮ǰ������һ��.bmp���ļ���Vista�͸��߼���ϵͳ
	֧��.jpg��ʽ
	*************************************************/
	//SPI_SETDESKWALLPAPER����ʹ�ñ�ֽ�ı䱣�沢�����ɼ���
	if (SUCCEEDED(hr))
	{
		if (!SystemParametersInfo(SPI_SETDESKWALLPAPER, 0,
			static_cast<PVOID>(pszFile),
			SPIF_UPDATEINIFILE | SPIF_SENDWININICHANGE))
		{
			hr = HRESULT_FROM_WIN32(GetLastError());
		}
	}

	return hr;
}

//�ı����汳��(PictureFullpath:ͼƬ����·��)
void changePicture(std::string PictureFullpath)
{
	bool result = false;
	result = SystemParametersInfoA(SPI_SETDESKWALLPAPER,
		0,
		(PVOID)PictureFullpath.c_str(),
		0);
	if (result == false)
	{
		cout << "���ձ�ֽ����ʧ�ܣ�����ϵ������Ա��" << endl;
	}

	else
	{
		SystemParametersInfoA(SPI_SETDESKWALLPAPER,
			0,
			(PVOID)PictureFullpath.c_str(),
			SPIF_SENDCHANGE);
		//deleteBmpAndXml();    //windows8�������ϻ��ɺ�ɫ�����������ŵ�main������û���⣬����ԭ��δ֪
		system("cls");
		std::cout << "version:1.0.0 (Author:xiaoxi666)" << std::endl << std::endl;
		std::cout << "���ձ�ֽ���³ɹ���" << std::endl << std::endl;
		std::cout << "���õ�һ�쿪ʼ�����������ܰɣ�" << std::endl << std::endl;
	}
}

//�Ƕ�ת����
double _ToRad(double a){ return a*PI / 180; }
//����ת�Ƕ�
double _ToAng(double a){ return a * 180 / PI; }

/**
* ����Ŀ�������ڲ��յ����������
* destX:Ŀ�����������X
* destY:Ŀ�����������Y
* destLng:Ŀ��㾭��
* destLat:Ŀ���γ��
* srcX:���յ���������X
* srcY:���յ���������Y
* srcLng:���յ㾭��
* srcLat:���յ�γ��
* scale:��λ���ص����(��)
**/
void GetPixelPointFromGeo(int &destX, int &destY, double destLng, double destLat, int srcX, int srcY, double srcLng, double srcLat, double scale)
{
	double destDadiX, destDadiY;	//Ŀ���������;
	double srcDadiX, srcDadiY;	//���յ�������;
	WebMercatorProjCal(destLng, destLat, destDadiX, destDadiY);
	WebMercatorProjCal(srcLng, srcLat, srcDadiX, srcDadiY);
	GetPixelPointFromDadi(destX, destY, destDadiX, destDadiY, srcX, srcY, srcDadiX, srcDadiY, scale);
}

//void GetGeodeFromScrPoint(int x, int y, double *DadiX, double *DadiY)
//{
//	CPoint p = RotatePoint(x, y, CPoint(m_MainShip.posX, m_MainShip.posY), 2 * PI - m_northAngle);
//	double mainShipDadiX, mainShipDadiY;	//�����Ĵ������;
//	WebMercatorProjCal(m_MainShip.longitude, m_MainShip.latitude, &mainShipDadiX, &mainShipDadiY);
//	*DadiX = mainShipDadiX + (p.x - m_MainShip.posX)*m_perPixelM;
//	*DadiY = mainShipDadiY - (p.y - m_MainShip.posY)*m_perPixelM;
//}

/**
* ����Ŀ�������ڲ��յ����������
* destX:Ŀ�����������X
* destY:Ŀ�����������Y
* destDadiX:Ŀ���������X
* destDadiY:Ŀ���������Y
* srcX:���յ���������X
* srcY:���յ���������Y
* srcDadiX:���յ�������X
* srcDadiY:���յ�������Y
* scale:��λ���ص����(��)
**/
void GetPixelPointFromDadi(int &destX, int &destY, double destDadiX, double destDadiY, int srcX, int srcY, double srcDadiX, double srcDadiY, double scale)
{
	destX = srcX + (int)((destDadiX - srcDadiX) / scale + 0.5);
	destY = srcY - (int)((destDadiY - srcDadiY) / scale + 0.5);
}

/**
* �ɲ��յ����Ŀ���ľ�γ��
* destX:Ŀ�����������X
* destY:Ŀ�����������Y
* destLng:Ŀ��㾭��
* destLat:Ŀ���γ��
* srcX:���յ���������X
* srcY:���յ���������Y
* srcLng:���յ㾭��
* srcLat:���յ�γ��
* scale:��λ���ص����(��)
**/
void GetGeograFromScrPoint(int destX, int destY, double &destLng, double &destLat, int srcX, int srcY, double srcLng, double srcLat, double scale)
{
	double destDadiX, destDadiY;	//Ŀ���������;
	double srcDadiX, srcDadiY;	//���յ�������;
	WebMercatorProjCal(srcLng, srcLat, srcDadiX, srcDadiY);
	destDadiX = srcDadiX + (destX - srcX)*scale;
	destDadiY = srcDadiY - (destY - srcY)*scale;
	WebMercatorProjInvcal(destDadiX, destDadiY, destLng, destLat);
}