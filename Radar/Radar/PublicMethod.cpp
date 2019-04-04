#include"stdafx.h"
#include"PublicMethod.h"
#include<math.h>
#include<iostream>

using namespace std;

#define PI 3.1415927

//////6度带宽   54年北京坐标系  
//高斯投影由经纬度(Unit:DD)反算大地坐标(含带号，Unit:Metres)
void GaussProjCal(double longitude, double latitude, double *X, double *Y)
{
	int ProjNo = 0;
	int ZoneWide;   ////带宽  
	double  longitude1, latitude1, longitude0, latitude0, X0, Y0, xval, yval;
	double  a, f, e2, ee, NN, T, C, A, M, iPI;
	iPI = 0.0174532925199433;   ////3.1415926535898/180.0;   
	ZoneWide = 6;     ////6度带宽  
	a = 6378245.0;
	f = 1.0 / 298.3;      //54年北京坐标系参数 
	//a=6378140.0;   f=1/298.257;      //80年西安坐标系参数  
	ProjNo = (int)(longitude / ZoneWide);

	longitude0 = ProjNo * ZoneWide + ZoneWide / 2;
	longitude0 = longitude0 * iPI;
	latitude0 = 0;
	longitude1 = longitude * iPI;   //经度转换为弧度  
	latitude1 = latitude * iPI;     //纬度转换为弧度 
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

//高斯投影由大地坐标(Unit:Metres)反算经纬度(Unit:DD) 
void GaussProjInvCal(double X, double Y, double *longitude, double *latitude)
{
	int ProjNo;
	int ZoneWide;   ////带宽  
	double longitude1, latitude1, longitude0, latitude0, X0, Y0, xval, yval;
	double e1, e2, f, a, ee, NN, T, C, M, D, R, u, fai, iPI;
	iPI = 0.0174532925199433;   ////3.1415926535898/180.0;   
	a = 6378245.0;
	f = 1.0 / 298.3;    //54年北京坐标系参数
	//a=6378140.0;   f=1/298.257;      //80年西安坐标系参数   
	ZoneWide = 6;     ////6度带宽  
	ProjNo = (int)(X / 1000000L);    //查找带号 
	longitude0 = (ProjNo - 1) * ZoneWide + ZoneWide / 2;
	longitude0 = longitude0 * iPI;     //中央经线  
	X0 = ProjNo * 1000000L + 500000L;
	Y0 = 0;
	xval = X - X0;
	yval = Y - Y0;      //带内大地坐标 

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
	//计算经度(Longitude) 纬度(Latitude) 
	longitude1 = longitude0 + (D - (1 + 2 * T + C)*D*D*D / 6 + (5 - 2 * C + 28 * T - 3 * C*C + 8 * ee + 24 * T*T)*D*D*D*D*D / 120) / cos(fai);
	latitude1 = fai - (NN*tan(fai) / R)*(D*D / 2 - (5 + 3 * T + 10 * C - 4 * C*C - 9 * ee)*D*D*D*D / 24
		+ (61 + 90 * T + 298 * C + 45 * T*T - 256 * ee - 3 * C*C)*D*D*D*D*D*D / 720);   //转换为度 DD 
	*longitude = longitude1 / iPI;
	*latitude = latitude1 / iPI;
}


/**
 * Web墨卡托正投影（经纬度转平面坐标）
 * lng:经度
 * lat:纬度
 * dadiX:平面坐标(大地坐标)X
 * dadiY:平面坐标(大地坐标)Y
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
 * Web墨卡托逆投影（平面坐标转经纬度）
 * dadiX:平面坐标(大地坐标)X
 * dadiY:平面坐标(大地坐标)Y
 * lng:经度
 * lat:纬度
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

//从buffer读取一个字符串 遇间隔符返回 最大长度20
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
		//计算小数部分
		while ((**buffer) >= 48 && (**buffer) <= 57)
		{
			result += (double)(**buffer - 48)*dfs;
			dfs /= 10;
			(*buffer)++;
		}
	}
	return true;
}

//整型转字符串，返回字符串的实际长度
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

//获取文件的全部长度
size_t getFileAllSize(ifstream * file)
{
	size_t temp = file->tellg(); // 记录下当前位置
	file->seekg(0, ios_base::end); // 移动到文件尾
	size_t size = file->tellg(); // 取得当前位置的指针长度 即文件长度
	file->seekg(temp); // 移动到原来的位置
	return size;
}

//获取文件的剩下长度
size_t getFileLastSize(ifstream * file)
{
	size_t begin = file->tellg(); // 记录下当前位置
	file->seekg(0, ios_base::end); // 移动到文件尾
	size_t end = file->tellg(); // 取得当前位置的指针长度 即文件长度
	file->seekg(begin); // 移动到原来的位置
	return (end - begin);
}


//求a%b
double modulus_f(double a, double b)
{
	int i = a / b;
	return (double)(a - (i*b));
}

//获取四个点的线性插值结果(4个点为矩形)
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

//判断一个点是否位于一个由外接矩形确定的椭圆中
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

//平移一个点
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

	//设置壁纸风格和展开方式
	//在Control Panel\Desktop中的两个键值将被设置
	// TileWallpaper
	//  0: 图片不被平铺 
	//  1: 被平铺 
	// WallpaperStyle
	//  0:  0表示图片居中，1表示平铺
	//  2:  拉伸填充整个屏幕
	//  6:  拉伸适应屏幕并保持高度比
	//  10: 图片被调整大小裁剪适应屏幕保持纵横比

	//以可读可写的方式打开HKCU\Control Panel\Desktop注册表项
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
		// 设置 WallpaperStyle 和 TileWallpaper 到注册表项.
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

	//通过调用Win32 API函数SystemParametersInfo 设置桌面壁纸
	/************************************************
	之前我们已经设置了壁纸的类型，但是壁纸图片的实际文件路径还没
	设置。SystemParametersInfo 这个函数位于user32.dll中，它支持
	我们从系统中获得硬件和配置信息。它有四个参数，第一个指明调用这
	个函数所要执行的操作，接下来的两个参数指明将要设置的数据，依据
	第一个参数的设定。最后一个允许指定改变是否被保存。这里第一个参数
	我们应指定SPI_SETDESKWALLPAPER，指明我们是要设置壁纸。接下来是
	文件路径。在Vista之前必须是一个.bmp的文件。Vista和更高级的系统
	支持.jpg格式
	*************************************************/
	//SPI_SETDESKWALLPAPER参数使得壁纸改变保存并持续可见。
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

//改变桌面背景(PictureFullpath:图片完整路径)
void changePicture(std::string PictureFullpath)
{
	bool result = false;
	result = SystemParametersInfoA(SPI_SETDESKWALLPAPER,
		0,
		(PVOID)PictureFullpath.c_str(),
		0);
	if (result == false)
	{
		cout << "今日壁纸更新失败！请联系开发人员！" << endl;
	}

	else
	{
		SystemParametersInfoA(SPI_SETDESKWALLPAPER,
			0,
			(PVOID)PictureFullpath.c_str(),
			SPIF_SENDCHANGE);
		//deleteBmpAndXml();    //windows8及其以上会变成黑色，把这条语句放到main最后面就没问题，具体原因未知
		system("cls");
		std::cout << "version:1.0.0 (Author:xiaoxi666)" << std::endl << std::endl;
		std::cout << "今日壁纸更新成功！" << std::endl << std::endl;
		std::cout << "美好的一天开始啦！用心享受吧！" << std::endl << std::endl;
	}
}

//角度转弧度
double _ToRad(double a){ return a*PI / 180; }
//弧度转角度
double _ToAng(double a){ return a * 180 / PI; }

/**
* 计算目标点相对于参照点的像素坐标
* destX:目标点像素坐标X
* destY:目标点像素坐标Y
* destLng:目标点经度
* destLat:目标点纬度
* srcX:参照点像素坐标X
* srcY:参照点像素坐标Y
* srcLng:参照点经度
* srcLat:参照点纬度
* scale:单位像素点距离(米)
**/
void GetPixelPointFromGeo(int &destX, int &destY, double destLng, double destLat, int srcX, int srcY, double srcLng, double srcLat, double scale)
{
	double destDadiX, destDadiY;	//目标点大地坐标;
	double srcDadiX, srcDadiY;	//参照点大地坐标;
	WebMercatorProjCal(destLng, destLat, destDadiX, destDadiY);
	WebMercatorProjCal(srcLng, srcLat, srcDadiX, srcDadiY);
	GetPixelPointFromDadi(destX, destY, destDadiX, destDadiY, srcX, srcY, srcDadiX, srcDadiY, scale);
}

//void GetGeodeFromScrPoint(int x, int y, double *DadiX, double *DadiY)
//{
//	CPoint p = RotatePoint(x, y, CPoint(m_MainShip.posX, m_MainShip.posY), 2 * PI - m_northAngle);
//	double mainShipDadiX, mainShipDadiY;	//本船的大地坐标;
//	WebMercatorProjCal(m_MainShip.longitude, m_MainShip.latitude, &mainShipDadiX, &mainShipDadiY);
//	*DadiX = mainShipDadiX + (p.x - m_MainShip.posX)*m_perPixelM;
//	*DadiY = mainShipDadiY - (p.y - m_MainShip.posY)*m_perPixelM;
//}

/**
* 计算目标点相对于参照点的像素坐标
* destX:目标点像素坐标X
* destY:目标点像素坐标Y
* destDadiX:目标点大地坐标X
* destDadiY:目标点大地坐标Y
* srcX:参照点像素坐标X
* srcY:参照点像素坐标Y
* srcDadiX:参照点大地坐标X
* srcDadiY:参照点大地坐标Y
* scale:单位像素点距离(米)
**/
void GetPixelPointFromDadi(int &destX, int &destY, double destDadiX, double destDadiY, int srcX, int srcY, double srcDadiX, double srcDadiY, double scale)
{
	destX = srcX + (int)((destDadiX - srcDadiX) / scale + 0.5);
	destY = srcY - (int)((destDadiY - srcDadiY) / scale + 0.5);
}

/**
* 由参照点计算目标点的经纬度
* destX:目标点像素坐标X
* destY:目标点像素坐标Y
* destLng:目标点经度
* destLat:目标点纬度
* srcX:参照点像素坐标X
* srcY:参照点像素坐标Y
* srcLng:参照点经度
* srcLat:参照点纬度
* scale:单位像素点距离(米)
**/
void GetGeograFromScrPoint(int destX, int destY, double &destLng, double &destLat, int srcX, int srcY, double srcLng, double srcLat, double scale)
{
	double destDadiX, destDadiY;	//目标点大地坐标;
	double srcDadiX, srcDadiY;	//参照点大地坐标;
	WebMercatorProjCal(srcLng, srcLat, srcDadiX, srcDadiY);
	destDadiX = srcDadiX + (destX - srcX)*scale;
	destDadiY = srcDadiY - (destY - srcY)*scale;
	WebMercatorProjInvcal(destDadiX, destDadiY, destLng, destLat);
}