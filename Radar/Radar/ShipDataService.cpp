#include "stdafx.h"
#include"ShipDataService.h"
#include<stdio.h>
#include"login.h"

std::mutex sta_mutex;

ShipDataService::ShipDataService()
{
	mmsi = _T("999999999");
	rudderOrder_symbol = _T("0");
	rudderOrder_num = _T("0");
	rudderAngle_symbol = _T("0");
	rudderAngle_num = _T("0");
	telegraph_symbol_L = _T("0");
	telegraph_num_L = _T("0");
	telegraph_symbol_R = _T("0");
	telegraph_num_R = _T("0");
	RPM_L = _T("0");
	RPM_R = _T("0");
	PITCH_L = _T("0");
	PITCH_R = _T("0");
	StartAir_L = _T("0");
	StartAir_R = _T("0");
	rotate_symbol = _T("0");
	rotate_num = _T("00.00");
	distance_total = _T("0.00");
	distance_reset = _T("0.00");
	heading = _T("44.44");
	speed = _T("0.00");
	course = _T("0.00");
	latitude_symbol = _T("S");
	latitude_num = _T("0.0000");
	longitude_symbol = _T("E");
	longitude_num = _T("0.0000");
	steeringAlarm_all = _T("0000");
	hostAlarm_all = _T("000000");
	singleSteeringAlarmNum = _T("99");
	singleSteeringAlarmflag = _T("99");
	singleHostAlarmNum = _T("99");
	singleHostAlarmflag = _T("99");
	status_CONSVR = 0;	//ConningServices
	status_SG = 0;		//SG_Panel
	status_PS = 0;		//PS_Panel
	status_LOG = 0;		//LOG_Panel
	status_ALARM = 0;	//Alarm_Panel
	status_RADAR = 0;	//Radar
	status_ECDIS = 0;	//电子海图
	status_VI = 0;		//虚拟仪表
	ip_CONSVR = _T("000.000.000.000");
	ip_SG = _T("000.000.000.000");
	ip_PS = _T("000.000.000.000");
	ip_LOG = _T("000.000.000.000");
	ip_ALARM = _T("000.000.000.000");
	ip_RADAR = _T("000.000.000.000");
	ip_ECDIS = _T("000.000.000.000");
	ip_VI = _T("000.000.000.000");
}

ShipDataService::~ShipDataService()
{

}

//注意！此函数内部给char* new了内存，调用此函数需手动delete char*
//char* _CStringToCharArray(const CString str)
//{
//	char *ptr;
//
//#ifdef _UNICODE
//	LONG len;
//	len = WideCharToMultiByte(CP_ACP, 0, str, -1, NULL, 0, NULL, NULL);
//	ptr = new char[len + 1];
//	memset(ptr, 0, len + 1);
//	WideCharToMultiByte(CP_ACP, 0, str, -1, ptr, len + 1, NULL, NULL);
//#else
//	ptr = new char[str.GetAllocLength() + 1];
//	sprintf(ptr, _T("%s"), str);
//#endif
//	return ptr;
//}

bool _CStringToCharArray(const CString cstr, char *pstr, int maxLen)
{
#ifdef _UNICODE
	LONG len;
	len = WideCharToMultiByte(CP_ACP, 0, cstr, -1, NULL, 0, NULL, NULL);
	if (maxLen >= (len + 1))
	{
		memset(pstr, 0, maxLen);
		WideCharToMultiByte(CP_ACP, 0, cstr, -1, pstr, len + 1, NULL, NULL);
	}
	else
		return false;
#else
	int len = cstr.GetAllocLength();
	if (maxLen >= (len + 1))
	{
		sprintf(pstr, _T("%s"), cstr);
	}
	else
		return false;
#endif
	return true;
}

CString _GetCheckStr(const CString strSource)
{
	int len = strSource.GetLength();
	if (len <= 0)
		return _T("");
	char c_char = strSource[0];
	for (int i = 1; i < len; i++)
	{
		char a = strSource[i];
		c_char = c_char^a;
	}
	int i_char = (int)c_char;
	CString str;
	str.Format(_T("%x"), i_char);
	if (str.GetLength() == 1)
		str = '0' + str;
	return str;
}

////获取心跳包协议
//CString ShipDataService::GetProtocol_HeartBeat(const char* IP, const char* name, int state)
//{
//	CString str1, str;
//	str1.Format(_T("GPHBT,%s,%s,%s,%d"), mmsi, IP, name, state);
//	str.Format(_T("$%s*%s\r\n"), str1, _GetCheckStr(str1));
//	return str;
//}
//
////获取心跳包协议
//CString ShipDataService::GetProtocol_HeartBeat(CString IP, CString name, int state)
//{
//	CString str1, str;
//	str1.Format(_T("GPHBT,%s,%s,%s,%d"), mmsi, IP, name, state);
//	str.Format(_T("$%s*%s\r\n"), str1, _GetCheckStr(str1));
//	return str;
//}

CString ShipDataService::GetProtocol_GPNRO()
{
	if (!sta_mutex.try_lock())
		return NULL;
	CString str = _getProtocol_GPNRO();
	sta_mutex.unlock();
	return str;
}

//获取舵令协议
CString ShipDataService::_getProtocol_GPNRO()
{
	CString str1, str;
	str1.Format(_T("GPNRO,%s,%s,%s"), mmsi, rudderOrder_symbol, rudderOrder_num);
	str.Format(_T("$%s*%s\r\n"), str1, _GetCheckStr(str1));
	return str;
}

CString ShipDataService::GetProtocol_GPNRO_R()
{
	if (!sta_mutex.try_lock())
		return NULL;
	CString str = _getProtocol_GPNRO_R();
	sta_mutex.unlock();
	return str;
}

//获取舵令协议（0左1右）
CString ShipDataService::_getProtocol_GPNRO_R()
{
	CString str1, str;
	CString symbol = _T("1");
	if (rudderOrder_symbol == "1")
		symbol = _T("0");
	str1.Format(_T("GPNRO,%s,%s,%s"), mmsi, symbol, rudderOrder_num);
	str.Format(_T("$%s*%s\r\n"), str1, _GetCheckStr(str1));
	return str;
}

CString ShipDataService::GetProtocol_GPNRA()
{
	if (!sta_mutex.try_lock())
		return NULL;
	CString str = _getProtocol_GPNRA();
	sta_mutex.unlock();
	return str;
}

//获取舵角协议
CString ShipDataService::_getProtocol_GPNRA()
{
	CString str1, str;
	str1.Format(_T("GPNRA,%s,%s,%s"), mmsi, rudderAngle_symbol, rudderAngle_num);
	str.Format(_T("$%s*%s\r\n"), str1, _GetCheckStr(str1));
	return str;
}

CString ShipDataService::GetProtocol_GPNRA_R()
{
	if (!sta_mutex.try_lock())
		return NULL;
	CString str = _getProtocol_GPNRA_R();
	sta_mutex.unlock();
	return str;
}

//获取舵角协议（相反）
CString ShipDataService::_getProtocol_GPNRA_R()
{
	CString str, str1;
	CString symbol = _T("1");
	if (rudderAngle_symbol == "1")
		symbol = _T("0");
	str1.Format(_T("GPNRA,%s,%s,%s"), mmsi, symbol, rudderAngle_num);
	str.Format(_T("$%s*%s\r\n"), str1, _GetCheckStr(str1));
	return str;
}

CString ShipDataService::GetProtocol_GPNCZ_L()
{
	if (!sta_mutex.try_lock())
		return NULL;
	CString str = _getProtocol_GPNCZ_L();
	sta_mutex.unlock();
	return str;
}

//获取左车钟协议
CString ShipDataService::_getProtocol_GPNCZ_L()
{
	CString str1, str;
	str1.Format(_T("GPNCZ,%s,%s,%s,L"), mmsi, telegraph_symbol_L, telegraph_num_L);
	str.Format(_T("$%s*%s\r\n"), str1, _GetCheckStr(str1));
	return str;
}

CString ShipDataService::GetProtocol_GPNCZ_R()
{
	if (!sta_mutex.try_lock())
		return NULL;
	CString str = _getProtocol_GPNCZ_R();
	sta_mutex.unlock();
	return str;
}

//获取右车钟协议
CString ShipDataService::_getProtocol_GPNCZ_R()
{
	CString str1, str;
	str1.Format(_T("GPNCZ,%s,%s,%s,R"), mmsi, telegraph_symbol_R, telegraph_num_R);
	str.Format(_T("$%s*%s\r\n"), str1, _GetCheckStr(str1));
	return str;
}

CString ShipDataService::GetProtocol_GPEST_L()
{
	if (!sta_mutex.try_lock())
		return NULL;
	CString str = _getProtocol_GPEST_L();
	sta_mutex.unlock();
	return str;
}

//获取左发动机状态协议
CString ShipDataService::_getProtocol_GPEST_L()
{
	CString str1, str;
	str1.Format(_T("GPEST,%s,L,%s,%s,%s"), mmsi, RPM_L, PITCH_L, StartAir_L);
	str.Format(_T("$%s*%s\r\n"), str1, _GetCheckStr(str1));
	return str;
}

CString ShipDataService::GetProtocol_GPEST_R()
{
	if (!sta_mutex.try_lock())
		return NULL;
	CString str = _getProtocol_GPEST_R();
	sta_mutex.unlock();
	return str;
}

//获取右发动机状态协议
CString ShipDataService::_getProtocol_GPEST_R()
{
	CString str1, str;
	str1.Format(_T("GPEST,%s,R,%s,%s,%s"), mmsi, RPM_R, PITCH_R, StartAir_R);
	str.Format(_T("$%s*%s\r\n"), str1, _GetCheckStr(str1));
	return str;
}

CString ShipDataService::GetProtocol_GPROT()
{
	if (!sta_mutex.try_lock())
		return NULL;
	CString str = _getProtocol_GPROT();
	sta_mutex.unlock();
	return str;
}

//获取转速率协议
CString ShipDataService::_getProtocol_GPROT()
{
	CString str1, str;
	str1.Format(_T("GPROT,%s,%s,%s"), mmsi, rotate_symbol, rotate_num);
	str.Format(_T("$%s*%s\r\n"), str1, _GetCheckStr(str1));
	return str;
}

CString ShipDataService::GetProtocol_GPDIS()
{
	if (!sta_mutex.try_lock())
		return NULL;
	CString str = _getProtocol_GPDIS();
	sta_mutex.unlock();
	return str;
}

//获取航程协议
CString ShipDataService::_getProtocol_GPDIS()
{
	CString str1, str;
	str1.Format(_T("GPDIS,%s,%s,%s"), mmsi, distance_total, distance_reset);
	str.Format(_T("$%s*%s\r\n"), str1, _GetCheckStr(str1));
	return str;
}

CString ShipDataService::GetProtocol_GPHED()
{
	if (!sta_mutex.try_lock())
		return NULL;
	CString str = _getProtocol_GPHED();
	sta_mutex.unlock();
	return str;
}

//获取船首向协议
CString ShipDataService::_getProtocol_GPHED()
{
	CString str1, str;
	str1.Format(_T("GPHED,%s,%s"), mmsi, heading);
	str.Format(_T("$%s*%s\r\n"), str1, _GetCheckStr(str1));
	return str;
}

CString ShipDataService::GetProtocol_GPSME()
{
	if (!sta_mutex.try_lock())
		return NULL;
	CString str = _getProtocol_GPSME();
	sta_mutex.unlock();
	return str;
}

//获取船舶主要航行信息协议
CString ShipDataService::_getProtocol_GPSME()
{
	CString str1, str;
	str1.Format(_T("GPSME,%s,%s,%s,%s,%s,%s,%s"), mmsi, speed, course, latitude_num, latitude_symbol, longitude_num, longitude_symbol);
	str.Format(_T("$%s*%s\r\n"), str1, _GetCheckStr(str1));
	return str;
}

CString ShipDataService::GetProtocol_GPSTE()
{
	if (!sta_mutex.try_lock())
		return NULL;
	CString str = _getProtocol_GPSTE();
	sta_mutex.unlock();
	return str;
}

//获取舵机报警协议
CString ShipDataService::_getProtocol_GPSTE()
{
	CString str1, str;
	str1.Format(_T("GPSTE,%s,%s"), mmsi, steeringAlarm_all);
	str.Format(_T("$%s*%s\r\n"), str1, _GetCheckStr(str1));
	return str;
}

CString ShipDataService::GetProtocol_GPMAE()
{
	if (!sta_mutex.try_lock())
		return NULL;
	CString str = _getProtocol_GPMAE();
	sta_mutex.unlock();
	return str;
}

//获取主机报警协议
CString ShipDataService::_getProtocol_GPMAE()
{
	CString str1, str;
	str1.Format(_T("GPMAE,%s,%s"), mmsi, hostAlarm_all);
	str.Format(_T("$%s*%s\r\n"), str1, _GetCheckStr(str1));
	return str;
}

CString ShipDataService::GetProtocol_GPSSE()
{
	if (!sta_mutex.try_lock())
		return NULL;
	CString str = _getProtocol_GPSSE();
	sta_mutex.unlock();
	return str;
}

//获取舵机单灯报警协议
CString ShipDataService::_getProtocol_GPSSE()
{
	CString str1, str;
	str1.Format(_T("GPSSE,%s,%s,%s"), mmsi, singleSteeringAlarmNum, singleSteeringAlarmflag);
	str.Format(_T("$%s*%s\r\n"), str1, _GetCheckStr(str1));
	return str;
}

CString ShipDataService::GetProtocol_GPSMA()
{
	if (!sta_mutex.try_lock())
		return NULL;
	CString str = _getProtocol_GPSMA();
	sta_mutex.unlock();
	return str;
}

//获取主机单灯报警协议
CString ShipDataService::_getProtocol_GPSMA()
{
	CString str1, str;
	str1.Format(_T("GPSMA,%s,%s,%s"), mmsi, singleHostAlarmNum, singleHostAlarmflag);
	str.Format(_T("$%s*%s\r\n"), str1, _GetCheckStr(str1));
	return str;
}

bool ShipDataService::Decode(const CString& str)
{
	if (!sta_mutex.try_lock())
		return false;
	bool isDecodeSuccess = false;
	try{
		isDecodeSuccess = _decode(str);
		sta_mutex.unlock();
	}
	catch (std::exception& e)
	{
		sta_mutex.unlock();
		TRACE(e.what());
	}
	return isDecodeSuccess;
}

//解析协议
bool ShipDataService::_decode(const CString& str)
{
	CString source = str;
	int sourceLen = source.GetLength();
	if (sourceLen <= 3)
		return false;
	if (source[0] != '$' || source[sourceLen - 1] != '\n' || source[sourceLen - 2] != '\r')
		return false;
	//获取数据位和校验位
	int pos_checkpre = source.Find('*');
	if (pos_checkpre == -1) return false;	//找不到校验和前缀
	CString data = source.Mid(1, pos_checkpre - 1);	//数据位
	CString checknum = source.Mid(pos_checkpre + 1, 2);	//校验位
	if (checknum.MakeUpper() != _GetCheckStr(data).MakeUpper()) return false;	//校验出错

	//获取地址域5字节
	char comma = ',';
	int pos_comma = data.Find(comma);
	if (pos_comma == -1) return false;
	CString addr = data.Left(5);
	CString dataWithouAddr = data.Mid(pos_comma + 1);	//除去地址域和第一个分隔符后剩下的部分
	if (addr == _T("GPNRO"))
	{
		pos_comma = dataWithouAddr.Find(comma);
		if (pos_comma != 9) return false;
		CString MMSI = dataWithouAddr.Left(9);
		dataWithouAddr = dataWithouAddr.Mid(pos_comma + 1);

		pos_comma = dataWithouAddr.Find(comma);
		if (pos_comma != 1) return false;
		CString RUDDERORDER_SYMBOL = dataWithouAddr.Left(1);
		dataWithouAddr = dataWithouAddr.Mid(pos_comma + 1);

		CString RUDDERORDER_NUM = dataWithouAddr;

		mmsi = MMSI;
		rudderOrder_symbol = RUDDERORDER_SYMBOL;
		rudderOrder_num = RUDDERORDER_NUM;
	}
	else if (addr == _T("GPNRA"))
	{
		pos_comma = dataWithouAddr.Find(comma);
		if (pos_comma != 9) return false;
		CString MMSI = dataWithouAddr.Left(9);
		dataWithouAddr = dataWithouAddr.Mid(pos_comma + 1);

		pos_comma = dataWithouAddr.Find(comma);
		if (pos_comma != 1) return false;
		CString RUDDERANGLE_SYMBOL = dataWithouAddr.Left(1);
		dataWithouAddr = dataWithouAddr.Mid(pos_comma + 1);

		CString RUDDERANGLE_NUM = dataWithouAddr;

		mmsi = MMSI;
		rudderAngle_symbol = RUDDERANGLE_SYMBOL;
		rudderAngle_num = RUDDERANGLE_NUM;
	}
	else if (addr == _T("GPNCZ"))
	{
		pos_comma = dataWithouAddr.Find(comma);
		if (pos_comma != 9) return false;
		CString MMSI = dataWithouAddr.Left(9);
		dataWithouAddr = dataWithouAddr.Mid(pos_comma + 1);

		pos_comma = dataWithouAddr.Find(comma);
		if (pos_comma != 1) return false;
		CString TELEGRAPH_SYMBOL = dataWithouAddr.Left(1);
		dataWithouAddr = dataWithouAddr.Mid(pos_comma + 1);

		pos_comma = dataWithouAddr.Find(comma);
		if (pos_comma != 1) return false;
		CString TELEGRAPH_NUM = dataWithouAddr.Left(1);
		dataWithouAddr = dataWithouAddr.Mid(pos_comma + 1);

		CString TELEGRAPH_SIDE = dataWithouAddr;

		if (TELEGRAPH_SIDE.MakeUpper().Compare(_T("L")) == 0)
		{
			mmsi = MMSI;
			telegraph_symbol_L = TELEGRAPH_SYMBOL;
			telegraph_num_L = TELEGRAPH_NUM;
		}
		else if (TELEGRAPH_SIDE.MakeUpper().Compare(_T("R")) == 0)
		{
			mmsi = MMSI;
			telegraph_symbol_R = TELEGRAPH_SYMBOL;
			telegraph_num_R = TELEGRAPH_NUM;
		}
		else
			return false;

	}
	else if (addr == _T("GPROT"))
	{
		pos_comma = dataWithouAddr.Find(comma);
		if (pos_comma != 9) return false;
		CString MMSI = dataWithouAddr.Left(9);
		dataWithouAddr = dataWithouAddr.Mid(pos_comma + 1);

		pos_comma = dataWithouAddr.Find(comma);
		if (pos_comma != 1) return false;
		CString ROTATE_SYMBOL = dataWithouAddr.Left(1);
		dataWithouAddr = dataWithouAddr.Mid(pos_comma + 1);

		CString ROTATE_NUM = dataWithouAddr;

		mmsi = MMSI;
		rotate_symbol = ROTATE_SYMBOL;
		rotate_num = ROTATE_NUM;
	}
	else if (addr == _T("GPDIS"))
	{
		pos_comma = dataWithouAddr.Find(comma);
		if (pos_comma != 9) return false;
		CString MMSI = dataWithouAddr.Left(9);
		dataWithouAddr = dataWithouAddr.Mid(pos_comma + 1);

		pos_comma = dataWithouAddr.Find(comma);
		CString DISTANCETOTAL = dataWithouAddr.Left(pos_comma);
		CString DISTANCERESET = dataWithouAddr.Mid(pos_comma + 1);

		mmsi = MMSI;
		distance_total = DISTANCETOTAL;
		distance_reset = DISTANCERESET;
	}
	else if (addr == _T("GPHED"))
	{
		pos_comma = dataWithouAddr.Find(comma);
		if (pos_comma != 9) return false;
		CString MMSI = dataWithouAddr.Left(9);
		dataWithouAddr = dataWithouAddr.Mid(pos_comma + 1);

		CString HEADING = dataWithouAddr;

		mmsi = MMSI;
		heading = HEADING;
	}
	else if (addr == _T("GPSME"))
	{
		pos_comma = dataWithouAddr.Find(comma);
		if (pos_comma != 9) return false;
		CString MMSI = dataWithouAddr.Left(9);
		dataWithouAddr = dataWithouAddr.Mid(pos_comma + 1);

		pos_comma = dataWithouAddr.Find(comma);
		CString SPEED = dataWithouAddr.Left(pos_comma);
		dataWithouAddr = dataWithouAddr.Mid(pos_comma + 1);

		pos_comma = dataWithouAddr.Find(comma);
		CString COURSE = dataWithouAddr.Left(pos_comma);
		dataWithouAddr = dataWithouAddr.Mid(pos_comma + 1);

		pos_comma = dataWithouAddr.Find(comma);
		CString LATITUDENUM = dataWithouAddr.Left(pos_comma);
		dataWithouAddr = dataWithouAddr.Mid(pos_comma + 1);

		pos_comma = dataWithouAddr.Find(comma);
		CString LATITUDESYMBOL = dataWithouAddr.Left(pos_comma);
		if (LATITUDESYMBOL.GetLength() != 1) return false;
		dataWithouAddr = dataWithouAddr.Mid(pos_comma + 1);

		pos_comma = dataWithouAddr.Find(comma);
		CString LONGITUDENUM = dataWithouAddr.Left(pos_comma);
		dataWithouAddr = dataWithouAddr.Mid(pos_comma + 1);

		CString LONGITUDESYMBOL = dataWithouAddr;
		if (LONGITUDESYMBOL.GetLength() != 1) return false;

		mmsi = MMSI;
		speed = SPEED;
		course = COURSE;
		latitude_num = LATITUDENUM;
		latitude_symbol = LATITUDESYMBOL;
		longitude_num = LONGITUDENUM;
		longitude_symbol = LONGITUDESYMBOL;
	}
	else if (addr == _T("GPSTE"))
	{
		pos_comma = dataWithouAddr.Find(comma);
		if (pos_comma != 9) return false;
		CString MMSI = dataWithouAddr.Left(9);
		dataWithouAddr = dataWithouAddr.Mid(pos_comma + 1);

		CString STEERINGALARM = dataWithouAddr;
		if (STEERINGALARM.GetLength() != 4) return false;

		mmsi = MMSI;
		steeringAlarm_all = STEERINGALARM;
	}
	else if (addr == _T("GPMAE"))
	{
		pos_comma = dataWithouAddr.Find(comma);
		if (pos_comma != 9) return false;
		CString MMSI = dataWithouAddr.Left(9);
		dataWithouAddr = dataWithouAddr.Mid(pos_comma + 1);

		CString HOSTALARM = dataWithouAddr;
		if (HOSTALARM.GetLength() != 6) return false;

		mmsi = MMSI;
		hostAlarm_all = HOSTALARM;
	}
	else if (addr == _T("GPSSE"))
	{
		pos_comma = dataWithouAddr.Find(comma);
		if (pos_comma != 9) return false;
		CString MMSI = dataWithouAddr.Left(9);
		dataWithouAddr = dataWithouAddr.Mid(pos_comma + 1);

		pos_comma = dataWithouAddr.Find(comma);
		if (pos_comma != 1)
			return false;
		CString steeringNum = dataWithouAddr.Left(1);
		CString steerflag = dataWithouAddr.Mid(pos_comma + 1);

		singleSteeringAlarmNum = steeringNum;
		singleSteeringAlarmflag = steerflag;
	}
	else if (addr == _T("GPSMA"))
	{
		CString hostNum;
		pos_comma = dataWithouAddr.Find(comma);
		if (pos_comma != 9) return false;
		CString MMSI = dataWithouAddr.Left(9);
		dataWithouAddr = dataWithouAddr.Mid(pos_comma + 1);

		pos_comma = dataWithouAddr.Find(comma);
		if (pos_comma != 2 && pos_comma != 1)
			return false;
		if (pos_comma == 2)
			hostNum = dataWithouAddr.Left(2);
		else if (pos_comma == 1)
			hostNum = dataWithouAddr.Left(1);
		CString hostflag = dataWithouAddr.Mid(pos_comma + 1);

		singleHostAlarmNum = hostNum;
		singleHostAlarmflag = hostflag;
	}
	else if (addr == _T("GPEST"))
	{
		pos_comma = dataWithouAddr.Find(comma);
		if (pos_comma != 9) return false;
		CString MMSI = dataWithouAddr.Left(9);
		dataWithouAddr = dataWithouAddr.Mid(pos_comma + 1);

		pos_comma = dataWithouAddr.Find(comma);
		CString SIDE = dataWithouAddr.Left(pos_comma);
		dataWithouAddr = dataWithouAddr.Mid(pos_comma + 1);

		pos_comma = dataWithouAddr.Find(comma);
		CString RPM = dataWithouAddr.Left(pos_comma);
		dataWithouAddr = dataWithouAddr.Mid(pos_comma + 1);

		pos_comma = dataWithouAddr.Find(comma);
		CString PITCH = dataWithouAddr.Left(pos_comma);
		dataWithouAddr = dataWithouAddr.Mid(pos_comma + 1);

		CString STARTAIR = dataWithouAddr;


		if (SIDE.MakeUpper().Compare(_T("L")) == 0)
		{
			mmsi = MMSI;
			RPM_L = RPM;
			PITCH_L = PITCH;
			StartAir_L = STARTAIR;
		}
		else if (SIDE.MakeUpper().Compare(_T("R")) == 0)
		{
			mmsi = MMSI;
			RPM_R = RPM;
			PITCH_R = PITCH;
			StartAir_R = STARTAIR;
		}
		else
			return false;
	}
	else if (addr == _T("GPHBT"))
	{
		pos_comma = dataWithouAddr.Find(comma);
		if (pos_comma != 9) return false;
		CString MMSI = dataWithouAddr.Left(9);
		dataWithouAddr = dataWithouAddr.Mid(pos_comma + 1);

		pos_comma = dataWithouAddr.Find(comma);
		CString sz_IP = dataWithouAddr.Left(pos_comma);
		dataWithouAddr = dataWithouAddr.Mid(pos_comma + 1);

		pos_comma = dataWithouAddr.Find(comma);
		CString sz_Name = dataWithouAddr.Left(pos_comma);
		dataWithouAddr = dataWithouAddr.Mid(pos_comma + 1);

		CString sz_Status = dataWithouAddr;

		if (sz_Name.MakeUpper() == _T("CONSVR"))
		{
			ip_CONSVR = sz_IP;
			char c_status[50];
			_CStringToCharArray(sz_Status, c_status,50);
			status_CONSVR = atoi(c_status);
		}
		else if (sz_Name.MakeUpper() == _T("SG"))
		{
			ip_SG = sz_IP;
			char c_status[50];
			_CStringToCharArray(sz_Status, c_status, 50);
			status_SG = atoi(c_status);
		}
		else if (sz_Name.MakeUpper() == _T("PS"))
		{
			ip_PS = sz_IP;
			char c_status[50];
			_CStringToCharArray(sz_Status, c_status, 50);
			status_PS = atoi(c_status);
		}
		else if (sz_Name.MakeUpper() == _T("LOG"))
		{
			ip_LOG = sz_IP;
			char c_status[50];
			_CStringToCharArray(sz_Status, c_status, 50);
			status_LOG = atoi(c_status);
		}
		else if (sz_Name.MakeUpper() == _T("ALARM"))
		{
			ip_ALARM = sz_IP;
			char c_status[50];
			_CStringToCharArray(sz_Status, c_status, 50);
			status_ALARM = atoi(c_status);
		}
		else if (sz_Name.MakeUpper() == _T("RADAR"))
		{
			ip_RADAR = sz_IP;
			char c_status[50];
			_CStringToCharArray(sz_Status, c_status, 50);
			status_RADAR = atoi(c_status);
		}
		else if (sz_Name.MakeUpper() == _T("ECDIS"))
		{
			ip_ECDIS = sz_IP;
			char c_status[50];
			_CStringToCharArray(sz_Status, c_status, 50);
			status_ECDIS = atoi(c_status);
		}
		else if (sz_Name.MakeUpper() == _T("VI"))
		{
			ip_VI = sz_IP;
			char c_status[50];
			_CStringToCharArray(sz_Status, c_status, 50);
			status_VI = atoi(c_status);
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;	//地址域不符合协议
	}

	return true;
}

bool ShipDataService::Decode(const CString& str, CString &info)
{
	if (!sta_mutex.try_lock())
		return false;
	bool isDecodeSuccess = false;
	try{
		isDecodeSuccess = _decode(str, info);
		sta_mutex.unlock();
	}
	catch (std::exception& e)
	{
		sta_mutex.unlock();
		TRACE(e.what());
	}
	return isDecodeSuccess;
}

//解析协议
bool ShipDataService::_decode(const CString& str, CString &info)
{
	CString source = str;
	info = _T("other error");
	int sourceLen = source.GetLength();
	if (sourceLen <= 3)
		return false;
	if (source[0] != '$' || source[sourceLen - 1] != '\n' || source[sourceLen - 2] != '\r')
		return false;
	//获取数据位和校验位
	int pos_checkpre = source.Find('*');
	if (pos_checkpre == -1) return false;	//找不到校验和前缀
	CString data = source.Mid(1, pos_checkpre - 1);	//数据位
	CString checknum = source.Mid(pos_checkpre + 1, 2);	//校验位
	if (checknum.MakeUpper() != _GetCheckStr(data).MakeUpper())
	{
		info = _T("check error");
		return false;	//校验出错
	}

	//获取地址域5字节
	char comma = ',';
	int pos_comma = data.Find(comma);
	if (pos_comma == -1) return false;
	CString addr = data.Left(5);
	CString dataWithouAddr = data.Mid(pos_comma + 1);	//除去地址域和第一个分隔符后剩下的部分
	if (addr == _T("GPNRO"))
	{
		pos_comma = dataWithouAddr.Find(comma);
		if (pos_comma != 9) return false;
		CString MMSI = dataWithouAddr.Left(9);
		dataWithouAddr = dataWithouAddr.Mid(pos_comma + 1);

		pos_comma = dataWithouAddr.Find(comma);
		if (pos_comma != 1) return false;
		CString RUDDERORDER_SYMBOL = dataWithouAddr.Left(1);
		dataWithouAddr = dataWithouAddr.Mid(pos_comma + 1);

		CString RUDDERORDER_NUM = dataWithouAddr;

		mmsi = MMSI;
		rudderOrder_symbol = RUDDERORDER_SYMBOL;
		rudderOrder_num = RUDDERORDER_NUM;

	}
	else if (addr == _T("GPNRA"))
	{
		pos_comma = dataWithouAddr.Find(comma);
		if (pos_comma != 9) return false;
		CString MMSI = dataWithouAddr.Left(9);
		dataWithouAddr = dataWithouAddr.Mid(pos_comma + 1);

		pos_comma = dataWithouAddr.Find(comma);
		if (pos_comma != 1) return false;
		CString RUDDERANGLE_SYMBOL = dataWithouAddr.Left(1);
		dataWithouAddr = dataWithouAddr.Mid(pos_comma + 1);

		CString RUDDERANGLE_NUM = dataWithouAddr;

		mmsi = MMSI;
		rudderAngle_symbol = RUDDERANGLE_SYMBOL;
		rudderAngle_num = RUDDERANGLE_NUM;
	}
	else if (addr == _T("GPNCZ"))
	{
		pos_comma = dataWithouAddr.Find(comma);
		if (pos_comma != 9) return false;
		CString MMSI = dataWithouAddr.Left(9);
		dataWithouAddr = dataWithouAddr.Mid(pos_comma + 1);

		pos_comma = dataWithouAddr.Find(comma);
		if (pos_comma != 1) return false;
		CString TELEGRAPH_SYMBOL = dataWithouAddr.Left(1);
		dataWithouAddr = dataWithouAddr.Mid(pos_comma + 1);

		pos_comma = dataWithouAddr.Find(comma);
		if (pos_comma != 1) return false;
		CString TELEGRAPH_NUM = dataWithouAddr.Left(1);
		dataWithouAddr = dataWithouAddr.Mid(pos_comma + 1);

		CString TELEGRAPH_SIDE = dataWithouAddr;

		if (TELEGRAPH_SIDE.MakeUpper().Compare(_T("L")) == 0)
		{
			mmsi = MMSI;
			telegraph_symbol_L = TELEGRAPH_SYMBOL;
			telegraph_num_L = TELEGRAPH_NUM;
		}
		else if (TELEGRAPH_SIDE.MakeUpper().Compare(_T("R")) == 0)
		{
			mmsi = MMSI;
			telegraph_symbol_R = TELEGRAPH_SYMBOL;
			telegraph_num_R = TELEGRAPH_NUM;
		}
		else
			return false;
	}
	else if (addr == _T("GPROT"))
	{
		pos_comma = dataWithouAddr.Find(comma);
		if (pos_comma != 9) return false;
		CString MMSI = dataWithouAddr.Left(9);
		dataWithouAddr = dataWithouAddr.Mid(pos_comma + 1);

		pos_comma = dataWithouAddr.Find(comma);
		if (pos_comma != 1) return false;
		CString ROTATE_SYMBOL = dataWithouAddr.Left(1);
		dataWithouAddr = dataWithouAddr.Mid(pos_comma + 1);

		CString ROTATE_NUM = dataWithouAddr;

		mmsi = MMSI;
		rotate_symbol = ROTATE_SYMBOL;
		rotate_num = ROTATE_NUM;
	}
	else if (addr == _T("GPDIS"))
	{
		pos_comma = dataWithouAddr.Find(comma);
		if (pos_comma != 9) return false;
		CString MMSI = dataWithouAddr.Left(9);
		dataWithouAddr = dataWithouAddr.Mid(pos_comma + 1);

		pos_comma = dataWithouAddr.Find(comma);
		CString DISTANCETOTAL = dataWithouAddr.Left(pos_comma);
		CString DISTANCERESET = dataWithouAddr.Mid(pos_comma + 1);

		mmsi = MMSI;
		distance_total = DISTANCETOTAL;
		distance_reset = DISTANCERESET;
	}
	else if (addr == _T("GPHED"))
	{
		pos_comma = dataWithouAddr.Find(comma);
		if (pos_comma != 9) return false;
		CString MMSI = dataWithouAddr.Left(9);
		dataWithouAddr = dataWithouAddr.Mid(pos_comma + 1);

		CString HEADING = dataWithouAddr;

		mmsi = MMSI;
		heading = HEADING;
	}
	else if (addr == _T("GPSME"))
	{
		pos_comma = dataWithouAddr.Find(comma);
		if (pos_comma != 9) return false;
		CString MMSI = dataWithouAddr.Left(9);
		dataWithouAddr = dataWithouAddr.Mid(pos_comma + 1);

		pos_comma = dataWithouAddr.Find(comma);
		CString SPEED = dataWithouAddr.Left(pos_comma);
		dataWithouAddr = dataWithouAddr.Mid(pos_comma + 1);

		pos_comma = dataWithouAddr.Find(comma);
		CString COURSE = dataWithouAddr.Left(pos_comma);
		dataWithouAddr = dataWithouAddr.Mid(pos_comma + 1);

		pos_comma = dataWithouAddr.Find(comma);
		CString LATITUDENUM = dataWithouAddr.Left(pos_comma);
		dataWithouAddr = dataWithouAddr.Mid(pos_comma + 1);

		pos_comma = dataWithouAddr.Find(comma);
		CString LATITUDESYMBOL = dataWithouAddr.Left(pos_comma);
		if (LATITUDESYMBOL.GetLength() != 1) return false;
		dataWithouAddr = dataWithouAddr.Mid(pos_comma + 1);

		pos_comma = dataWithouAddr.Find(comma);
		CString LONGITUDENUM = dataWithouAddr.Left(pos_comma);
		dataWithouAddr = dataWithouAddr.Mid(pos_comma + 1);

		CString LONGITUDESYMBOL = dataWithouAddr;
		if (LONGITUDESYMBOL.GetLength() != 1) return false;

		mmsi = MMSI;
		speed = SPEED;
		course = COURSE;
		latitude_num = LATITUDENUM;
		latitude_symbol = LATITUDESYMBOL;
		longitude_num = LONGITUDENUM;
		longitude_symbol = LONGITUDESYMBOL;
	}
	else if (addr == _T("GPSTE"))
	{
		pos_comma = dataWithouAddr.Find(comma);
		if (pos_comma != 9) return false;
		CString MMSI = dataWithouAddr.Left(9);
		dataWithouAddr = dataWithouAddr.Mid(pos_comma + 1);

		CString STEERINGALARM = dataWithouAddr;
		if (STEERINGALARM.GetLength() != 4) return false;

		mmsi = MMSI;
		steeringAlarm_all = STEERINGALARM;
	}
	else if (addr == _T("GPMAE"))
	{
		pos_comma = dataWithouAddr.Find(comma);
		if (pos_comma != 9) return false;
		CString MMSI = dataWithouAddr.Left(9);
		dataWithouAddr = dataWithouAddr.Mid(pos_comma + 1);

		CString HOSTALARM = dataWithouAddr;
		if (HOSTALARM.GetLength() != 6) return false;

		mmsi = MMSI;
		hostAlarm_all = HOSTALARM;
	}
	else if (addr == _T("GPSSE"))
	{
		pos_comma = dataWithouAddr.Find(comma);
		if (pos_comma != 9) return false;
		CString MMSI = dataWithouAddr.Left(9);
		dataWithouAddr = dataWithouAddr.Mid(pos_comma + 1);

		pos_comma = dataWithouAddr.Find(comma);
		if (pos_comma != 1)
			return false;
		CString steeringNum = dataWithouAddr.Left(1);
		CString steerflag = dataWithouAddr.Mid(pos_comma + 1);

		singleSteeringAlarmNum = steeringNum;
		singleSteeringAlarmflag = steerflag;
	}
	else if (addr == _T("GPSMA"))
	{
		CString hostNum;
		pos_comma = dataWithouAddr.Find(comma);
		if (pos_comma != 9) return false;
		CString MMSI = dataWithouAddr.Left(9);
		dataWithouAddr = dataWithouAddr.Mid(pos_comma + 1);

		pos_comma = dataWithouAddr.Find(comma);
		if (pos_comma != 2 && pos_comma != 1)
			return false;
		if (pos_comma == 2)
			hostNum = dataWithouAddr.Left(2);
		else if (pos_comma == 1)
			hostNum = dataWithouAddr.Left(1);
		CString hostflag = dataWithouAddr.Mid(pos_comma + 1);

		singleHostAlarmNum = hostNum;
		singleHostAlarmflag = hostflag;
	}
	else if (addr == _T("GPEST"))
	{
		pos_comma = dataWithouAddr.Find(comma);
		if (pos_comma != 9) return false;
		CString MMSI = dataWithouAddr.Left(9);
		dataWithouAddr = dataWithouAddr.Mid(pos_comma + 1);

		pos_comma = dataWithouAddr.Find(comma);
		CString SIDE = dataWithouAddr.Left(pos_comma);
		dataWithouAddr = dataWithouAddr.Mid(pos_comma + 1);

		pos_comma = dataWithouAddr.Find(comma);
		CString RPM = dataWithouAddr.Left(pos_comma);
		dataWithouAddr = dataWithouAddr.Mid(pos_comma + 1);

		pos_comma = dataWithouAddr.Find(comma);
		CString PITCH = dataWithouAddr.Left(pos_comma);
		dataWithouAddr = dataWithouAddr.Mid(pos_comma + 1);

		CString STARTAIR = dataWithouAddr;


		if (SIDE.MakeUpper().Compare(_T("L")) == 0)
		{
			mmsi = MMSI;
			RPM_L = RPM;
			PITCH_L = PITCH;
			StartAir_L = STARTAIR;
		}
		else if (SIDE.MakeUpper().Compare(_T("R")) == 0)
		{
			mmsi = MMSI;
			RPM_R = RPM;
			PITCH_R = PITCH;
			StartAir_R = STARTAIR;
		}
		else
			return false;
	}
	else if (addr == _T("GPHBT"))
	{
		pos_comma = dataWithouAddr.Find(comma);
		if (pos_comma != 9) return false;
		CString MMSI = dataWithouAddr.Left(9);
		dataWithouAddr = dataWithouAddr.Mid(pos_comma + 1);

		pos_comma = dataWithouAddr.Find(comma);
		CString sz_IP = dataWithouAddr.Left(pos_comma);
		dataWithouAddr = dataWithouAddr.Mid(pos_comma + 1);

		pos_comma = dataWithouAddr.Find(comma);
		CString sz_Name = dataWithouAddr.Left(pos_comma);
		dataWithouAddr = dataWithouAddr.Mid(pos_comma + 1);

		CString sz_Status = dataWithouAddr;

		if (sz_Name.MakeUpper() == _T("CONSVR"))
		{
			ip_CONSVR = sz_IP;
			char c_status[50];
			_CStringToCharArray(sz_Status, c_status, 50);
			status_CONSVR = atoi(c_status);
		}
		else if (sz_Name.MakeUpper() == _T("SG"))
		{
			ip_SG = sz_IP;
			char c_status[50];
			_CStringToCharArray(sz_Status, c_status, 50);
			status_SG = atoi(c_status);
			
		}
		else if (sz_Name.MakeUpper() == _T("PS"))
		{
			ip_PS = sz_IP;
			char c_status[50];
			_CStringToCharArray(sz_Status, c_status, 50);
			status_PS = atoi(c_status);
			
		}
		else if (sz_Name.MakeUpper() == _T("LOG"))
		{
			ip_LOG = sz_IP;
			char c_status[50];
			_CStringToCharArray(sz_Status, c_status, 50);
			status_LOG = atoi(c_status);
			
		}
		else if (sz_Name.MakeUpper() == _T("ALARM"))
		{
			ip_ALARM = sz_IP;
			char c_status[50];
			_CStringToCharArray(sz_Status, c_status, 50);
			status_ALARM = atoi(c_status);
			
		}
		else if (sz_Name.MakeUpper() == _T("RADAR"))
		{
			ip_RADAR = sz_IP;
			char c_status[50];
			_CStringToCharArray(sz_Status, c_status, 50);
			status_RADAR = atoi(c_status);
			
		}
		else if (sz_Name.MakeUpper() == _T("ECDIS"))
		{
			ip_ECDIS = sz_IP;
			char c_status[50];
			_CStringToCharArray(sz_Status, c_status, 50);
			status_ECDIS = atoi(c_status);
			
		}
		else if (sz_Name.MakeUpper() == _T("VI"))
		{
			ip_VI = sz_IP;
			char c_status[50];
			_CStringToCharArray(sz_Status, c_status, 50);
			status_VI = atoi(c_status);
			
		}
		else
		{
			info = _T("GPHBT name error");
			return false;
		}
	}
	else
	{
		info = _T("Head error");
		return false;	//地址域不符合协议
	}
	info = addr;
	return true;
}

bool ShipDataService::Decode(const CString& str, CString& info, CString& GPHBT_name)
{
	if (!sta_mutex.try_lock())
		return false;
	bool isDecodeSuccess = false;
	try{
		isDecodeSuccess = _decode(str, info, GPHBT_name);
		sta_mutex.unlock();
	}
	catch (std::exception& e)
	{
		sta_mutex.unlock();
		TRACE(e.what());
	}
	return isDecodeSuccess;
}

bool ShipDataService::_decode(const CString& str, CString& info, CString& GPHBT_name)
{
	CString source = str;
	info = _T("other error");
	int sourceLen = source.GetLength();
	if (sourceLen <= 3)
		return false;
	if (source[0] != '$' || source[sourceLen - 1] != '\n' || source[sourceLen - 2] != '\r')
		return false;
	//获取数据位和校验位
	int pos_checkpre = source.Find('*');
	if (pos_checkpre == -1) return false;	//找不到校验和前缀
	CString data = source.Mid(1, pos_checkpre - 1);	//数据位
	CString checknum = source.Mid(pos_checkpre + 1, 2);	//校验位
	if (checknum.MakeUpper() != _GetCheckStr(data).MakeUpper())
	{
		info = _T("check error");
		return false;	//校验出错
	}

	//获取地址域5字节
	char comma = ',';
	int pos_comma = data.Find(comma);
	if (pos_comma == -1) return false;
	CString addr = data.Left(5);
	CString dataWithouAddr = data.Mid(pos_comma + 1);	//除去地址域和第一个分隔符后剩下的部分
	if (addr == _T("GPNRO"))
	{
		pos_comma = dataWithouAddr.Find(comma);
		if (pos_comma != 9) return false;
		CString MMSI = dataWithouAddr.Left(9);
		dataWithouAddr = dataWithouAddr.Mid(pos_comma + 1);

		pos_comma = dataWithouAddr.Find(comma);
		if (pos_comma != 1) return false;
		CString RUDDERORDER_SYMBOL = dataWithouAddr.Left(1);
		dataWithouAddr = dataWithouAddr.Mid(pos_comma + 1);

		CString RUDDERORDER_NUM = dataWithouAddr;

		mmsi = MMSI;
		rudderOrder_symbol = RUDDERORDER_SYMBOL;
		rudderOrder_num = RUDDERORDER_NUM;

	}
	else if (addr == _T("GPNRA"))
	{
		pos_comma = dataWithouAddr.Find(comma);
		if (pos_comma != 9) return false;
		CString MMSI = dataWithouAddr.Left(9);
		dataWithouAddr = dataWithouAddr.Mid(pos_comma + 1);

		pos_comma = dataWithouAddr.Find(comma);
		if (pos_comma != 1) return false;
		CString RUDDERANGLE_SYMBOL = dataWithouAddr.Left(1);
		dataWithouAddr = dataWithouAddr.Mid(pos_comma + 1);

		CString RUDDERANGLE_NUM = dataWithouAddr;

		mmsi = MMSI;
		rudderAngle_symbol = RUDDERANGLE_SYMBOL;
		rudderAngle_num = RUDDERANGLE_NUM;
	}
	else if (addr == _T("GPNCZ"))
	{
		pos_comma = dataWithouAddr.Find(comma);
		if (pos_comma != 9) return false;
		CString MMSI = dataWithouAddr.Left(9);
		dataWithouAddr = dataWithouAddr.Mid(pos_comma + 1);

		pos_comma = dataWithouAddr.Find(comma);
		if (pos_comma != 1) return false;
		CString TELEGRAPH_SYMBOL = dataWithouAddr.Left(1);
		dataWithouAddr = dataWithouAddr.Mid(pos_comma + 1);

		pos_comma = dataWithouAddr.Find(comma);
		if (pos_comma != 1) return false;
		CString TELEGRAPH_NUM = dataWithouAddr.Left(1);
		dataWithouAddr = dataWithouAddr.Mid(pos_comma + 1);

		CString TELEGRAPH_SIDE = dataWithouAddr;

		if (TELEGRAPH_SIDE.MakeUpper().Compare(_T("L")) == 0)
		{
			mmsi = MMSI;
			telegraph_symbol_L = TELEGRAPH_SYMBOL;
			telegraph_num_L = TELEGRAPH_NUM;
		}
		else if (TELEGRAPH_SIDE.MakeUpper().Compare(_T("R")) == 0)
		{
			mmsi = MMSI;
			telegraph_symbol_R = TELEGRAPH_SYMBOL;
			telegraph_num_R = TELEGRAPH_NUM;
		}
		else
			return false;
	}
	else if (addr == _T("GPROT"))
	{
		pos_comma = dataWithouAddr.Find(comma);
		if (pos_comma != 9) return false;
		CString MMSI = dataWithouAddr.Left(9);
		dataWithouAddr = dataWithouAddr.Mid(pos_comma + 1);

		pos_comma = dataWithouAddr.Find(comma);
		if (pos_comma != 1) return false;
		CString ROTATE_SYMBOL = dataWithouAddr.Left(1);
		dataWithouAddr = dataWithouAddr.Mid(pos_comma + 1);

		CString ROTATE_NUM = dataWithouAddr;

		mmsi = MMSI;
		rotate_symbol = ROTATE_SYMBOL;
		rotate_num = ROTATE_NUM;
	}
	else if (addr == _T("GPDIS"))
	{
		pos_comma = dataWithouAddr.Find(comma);
		if (pos_comma != 9) return false;
		CString MMSI = dataWithouAddr.Left(9);
		dataWithouAddr = dataWithouAddr.Mid(pos_comma + 1);

		pos_comma = dataWithouAddr.Find(comma);
		CString DISTANCETOTAL = dataWithouAddr.Left(pos_comma);
		CString DISTANCERESET = dataWithouAddr.Mid(pos_comma + 1);

		mmsi = MMSI;
		distance_total = DISTANCETOTAL;
		distance_reset = DISTANCERESET;
	}
	else if (addr == _T("GPHED"))
	{
		pos_comma = dataWithouAddr.Find(comma);
		if (pos_comma != 9) return false;
		CString MMSI = dataWithouAddr.Left(9);
		dataWithouAddr = dataWithouAddr.Mid(pos_comma + 1);

		CString HEADING = dataWithouAddr;

		mmsi = MMSI;
		heading = HEADING;
	}
	else if (addr == _T("GPSME"))
	{
		pos_comma = dataWithouAddr.Find(comma);
		if (pos_comma != 9) return false;
		CString MMSI = dataWithouAddr.Left(9);
		dataWithouAddr = dataWithouAddr.Mid(pos_comma + 1);

		pos_comma = dataWithouAddr.Find(comma);
		CString SPEED = dataWithouAddr.Left(pos_comma);
		dataWithouAddr = dataWithouAddr.Mid(pos_comma + 1);

		pos_comma = dataWithouAddr.Find(comma);
		CString COURSE = dataWithouAddr.Left(pos_comma);
		dataWithouAddr = dataWithouAddr.Mid(pos_comma + 1);

		pos_comma = dataWithouAddr.Find(comma);
		CString LATITUDENUM = dataWithouAddr.Left(pos_comma);
		dataWithouAddr = dataWithouAddr.Mid(pos_comma + 1);

		pos_comma = dataWithouAddr.Find(comma);
		CString LATITUDESYMBOL = dataWithouAddr.Left(pos_comma);
		if (LATITUDESYMBOL.GetLength() != 1) return false;
		dataWithouAddr = dataWithouAddr.Mid(pos_comma + 1);

		pos_comma = dataWithouAddr.Find(comma);
		CString LONGITUDENUM = dataWithouAddr.Left(pos_comma);
		dataWithouAddr = dataWithouAddr.Mid(pos_comma + 1);

		CString LONGITUDESYMBOL = dataWithouAddr;
		if (LONGITUDESYMBOL.GetLength() != 1) return false;

		mmsi = MMSI;
		speed = SPEED;
		course = COURSE;
		latitude_num = LATITUDENUM;
		latitude_symbol = LATITUDESYMBOL;
		longitude_num = LONGITUDENUM;
		longitude_symbol = LONGITUDESYMBOL;
	}
	else if (addr == _T("GPSTE"))
	{
		pos_comma = dataWithouAddr.Find(comma);
		if (pos_comma != 9) return false;
		CString MMSI = dataWithouAddr.Left(9);
		dataWithouAddr = dataWithouAddr.Mid(pos_comma + 1);

		CString STEERINGALARM = dataWithouAddr;
		if (STEERINGALARM.GetLength() != 4) return false;

		mmsi = MMSI;
		steeringAlarm_all = STEERINGALARM;
	}
	else if (addr == _T("GPMAE"))
	{
		pos_comma = dataWithouAddr.Find(comma);
		if (pos_comma != 9) return false;
		CString MMSI = dataWithouAddr.Left(9);
		dataWithouAddr = dataWithouAddr.Mid(pos_comma + 1);

		CString HOSTALARM = dataWithouAddr;
		if (HOSTALARM.GetLength() != 6) return false;

		mmsi = MMSI;
		hostAlarm_all = HOSTALARM;
	}
	else if (addr == _T("GPSSE"))
	{
		pos_comma = dataWithouAddr.Find(comma);
		if (pos_comma != 9) return false;
		CString MMSI = dataWithouAddr.Left(9);
		dataWithouAddr = dataWithouAddr.Mid(pos_comma + 1);

		pos_comma = dataWithouAddr.Find(comma);
		if (pos_comma != 1)
			return false;
		CString steeringNum = dataWithouAddr.Left(1);
		CString steerflag = dataWithouAddr.Mid(pos_comma + 1);

		singleSteeringAlarmNum = steeringNum;
		singleSteeringAlarmflag = steerflag;
	}
	else if (addr == _T("GPSMA"))
	{
		CString hostNum;
		pos_comma = dataWithouAddr.Find(comma);
		if (pos_comma != 9) return false;
		CString MMSI = dataWithouAddr.Left(9);
		dataWithouAddr = dataWithouAddr.Mid(pos_comma + 1);

		pos_comma = dataWithouAddr.Find(comma);
		if (pos_comma != 2 && pos_comma != 1)
			return false;
		if (pos_comma == 2)
			hostNum = dataWithouAddr.Left(2);
		else if (pos_comma == 1)
			hostNum = dataWithouAddr.Left(1);
		CString hostflag = dataWithouAddr.Mid(pos_comma + 1);

		singleHostAlarmNum = hostNum;
		singleHostAlarmflag = hostflag;
	}
	else if (addr == _T("GPEST"))
	{
		pos_comma = dataWithouAddr.Find(comma);
		if (pos_comma != 9) return false;
		CString MMSI = dataWithouAddr.Left(9);
		dataWithouAddr = dataWithouAddr.Mid(pos_comma + 1);

		pos_comma = dataWithouAddr.Find(comma);
		CString SIDE = dataWithouAddr.Left(pos_comma);
		dataWithouAddr = dataWithouAddr.Mid(pos_comma + 1);

		pos_comma = dataWithouAddr.Find(comma);
		CString RPM = dataWithouAddr.Left(pos_comma);
		dataWithouAddr = dataWithouAddr.Mid(pos_comma + 1);

		pos_comma = dataWithouAddr.Find(comma);
		CString PITCH = dataWithouAddr.Left(pos_comma);
		dataWithouAddr = dataWithouAddr.Mid(pos_comma + 1);

		CString STARTAIR = dataWithouAddr;


		if (SIDE.MakeUpper().Compare(_T("L")) == 0)
		{
			mmsi = MMSI;
			RPM_L = RPM;
			PITCH_L = PITCH;
			StartAir_L = STARTAIR;
		}
		else if (SIDE.MakeUpper().Compare(_T("R")) == 0)
		{
			mmsi = MMSI;
			RPM_R = RPM;
			PITCH_R = PITCH;
			StartAir_R = STARTAIR;
		}
		else
			return false;
	}
	else if (addr == _T("GPHBT"))
	{
		pos_comma = dataWithouAddr.Find(comma);
		if (pos_comma != 9) return false;
		CString MMSI = dataWithouAddr.Left(9);
		dataWithouAddr = dataWithouAddr.Mid(pos_comma + 1);

		pos_comma = dataWithouAddr.Find(comma);
		CString sz_IP = dataWithouAddr.Left(pos_comma);
		dataWithouAddr = dataWithouAddr.Mid(pos_comma + 1);

		pos_comma = dataWithouAddr.Find(comma);
		CString sz_Name = dataWithouAddr.Left(pos_comma);
		dataWithouAddr = dataWithouAddr.Mid(pos_comma + 1);

		CString sz_Status = dataWithouAddr;

		if (sz_Name.MakeUpper() == _T("CONSVR"))
		{
			GPHBT_name = _T("CONSVR");
			ip_CONSVR = sz_IP;
			char c_status[50];
			_CStringToCharArray(sz_Status, c_status, 50);
			status_CONSVR = atoi(c_status);
			
		}
		else if (sz_Name.MakeUpper() == _T("SG"))
		{
			GPHBT_name = _T("SG");
			ip_SG = sz_IP;
			char c_status[50];
			_CStringToCharArray(sz_Status, c_status, 50);
			status_SG = atoi(c_status);
			
		}
		else if (sz_Name.MakeUpper() == _T("PS"))
		{
			GPHBT_name = _T("PS");
			ip_PS = sz_IP;
			char c_status[50];
			_CStringToCharArray(sz_Status, c_status, 50);
			status_PS = atoi(c_status);
			
		}
		else if (sz_Name.MakeUpper() == _T("LOG"))
		{
			GPHBT_name = _T("LOG");
			ip_LOG = sz_IP;
			char c_status[50];
			_CStringToCharArray(sz_Status, c_status, 50);
			status_LOG = atoi(c_status);
			
		}
		else if (sz_Name.MakeUpper() == _T("ALARM"))
		{
			GPHBT_name = _T("ALARM");
			ip_ALARM = sz_IP;
			char c_status[50];
			_CStringToCharArray(sz_Status, c_status, 50);
			status_ALARM = atoi(c_status);
			
		}
		else if (sz_Name.MakeUpper() == _T("RADAR"))
		{
			GPHBT_name = _T("RADAR");
			ip_RADAR = sz_IP;
			char c_status[50];
			_CStringToCharArray(sz_Status, c_status, 50);
			status_RADAR = atoi(c_status);
			
		}
		else if (sz_Name.MakeUpper() == _T("ECDIS"))
		{
			GPHBT_name = _T("ECDIS");
			ip_ECDIS = sz_IP;
			char c_status[50];
			_CStringToCharArray(sz_Status, c_status, 50);
			status_ECDIS = atoi(c_status);
			
		}
		else if (sz_Name.MakeUpper() == _T("VI"))
		{
			GPHBT_name = _T("VI");
			ip_VI = sz_IP;
			char c_status[50];
			_CStringToCharArray(sz_Status, c_status, 50);
			status_VI = atoi(c_status);
			
		}
		else
		{
			info = _T("GPHBT name error");
			return false;
		}
	}
	else
	{
		info = _T("Head error");
		return false;	//地址域不符合协议
	}
	info = addr;
	return true;
}

bool ShipDataService::DecodeRealTelegraph(const CString& message)
{
	if (!sta_mutex.try_lock())
		return false;
	bool isDecodeSuccess = false;
	try{
		isDecodeSuccess = _decodeRealTelegraph(message);
		sta_mutex.unlock();
	}
	catch (std::exception& e)
	{
		sta_mutex.unlock();
		TRACE(e.what());
	}
	return isDecodeSuccess;
}


//解析真实车钟数据
bool ShipDataService::_decodeRealTelegraph(const CString& str)
{
	CString message = str;
	if (message.Compare(_T("$LDXDR,A,09,,TRUE,A,09,,SET*12\r\n")) == 0)
	{
		telegraph_symbol_R = _T("0");
		telegraph_num_R = _T("5");
		return true;
	}
	else if (message.Compare(_T("$LDXDR,A,07,,TRUE,A,07,,SET*12\r\n")) == 0)
	{
		telegraph_symbol_R = _T("0");
		telegraph_num_R = _T("4");
		return true;
	}
	else if (message.Compare(_T("$LDXDR,A,05,,TRUE,A,05,,SET*12\r\n")) == 0)
	{
		telegraph_symbol_R = _T("0");
		telegraph_num_R = _T("3");
		return true;
	}
	else if (message.Compare(_T("$LDXDR,A,03,,TRUE,A,03,,SET*12\r\n")) == 0)
	{
		telegraph_symbol_R = _T("0");
		telegraph_num_R = _T("2");
		return true;
	}
	else if (message.Compare(_T("$LDXDR,A,01,,TRUE,A,01,,SET*12\r\n")) == 0)
	{
		telegraph_symbol_R = _T("0");
		telegraph_num_R = _T("1");
		return true;
	}
	else if (message.Compare(_T("$LDXDR,A,00,,TRUE,A,00,,SET*12\r\n")) == 0)
	{
		telegraph_symbol_R = _T("0");
		telegraph_num_R = _T("0");
		return true;
	}
	else if (message.Compare(_T("$LDXDR,A,02,,TRUE,A,02,,SET*12\r\n")) == 0)
	{
		telegraph_symbol_R = _T("1");
		telegraph_num_R = _T("1");
		return true;
	}
	else if (message.Compare(_T("$LDXDR,A,04,,TRUE,A,04,,SET*12\r\n")) == 0)
	{
		telegraph_symbol_R = _T("1");
		telegraph_num_R = _T("2");
		return true;
	}
	else if (message.Compare(_T("$LDXDR,A,06,,TRUE,A,06,,SET*12\r\n")) == 0)
	{
		telegraph_symbol_R = _T("1");
		telegraph_num_R = _T("3");
		return true;
	}
	else if (message.Compare(_T("$LDXDR,A,08,,TRUE,A,08,,SET*12\r\n")) == 0)
	{
		telegraph_symbol_R = _T("1");
		telegraph_num_R = _T("4");
		return true;
	}
	else if (message.Compare(_T("$LDXDR,A,10,,TRUE,A,10,,SET*12\r\n")) == 0)
	{
		telegraph_symbol_R = _T("1");
		telegraph_num_R = _T("5");
		return true;
	}
	return false;
}

bool ShipDataService::DecodeRealRudder(const BYTE msg[], int length)
{
	if (!sta_mutex.try_lock())
		return false;
	bool isDecodeSuccess = false;
	try{
		isDecodeSuccess = _decodeRealRudder(msg,length);
		sta_mutex.unlock();
	}
	catch (std::exception& e)
	{
		sta_mutex.unlock();
		TRACE(e.what());
	}
	return isDecodeSuccess;
}

//解析真实舵数据
bool ShipDataService::_decodeRealRudder(const BYTE msg[], int length)
{
	if (length<9)
		return false;
	int num = (int)msg[3] * 256 + (int)msg[4];
	//int mid = 520;     //506版
	int mid = 484;		//理工版119
	int result = 0;
	if (num >= mid)
		result = (num - mid) / 8.8 + 0.5;
	else
		result = (num - mid) / 8.8 - 0.5;
	//if (result > 25) result = 25;
	//if (result < -25) result = -25;
	SetRudderOrder(result);
	return true;
}

double ShipDataService::GetHeading() const
{
	char c_str[50];
	_CStringToCharArray(heading, c_str, 50);
	double f_str = atof(c_str);
	return f_str;
}

double ShipDataService::GetCourse() const
{
	char c_str[50];
	_CStringToCharArray(course, c_str, 50);
	double f_str = atof(c_str);
	return f_str;
}

double ShipDataService::GetRotate() const
{
	char c_str[50];
	_CStringToCharArray(rotate_num, c_str, 50);
	double rotate = atof(c_str);
	if (rotate_symbol == "1")
		rotate = -rotate;
	return rotate;
}

int ShipDataService::GetRudderOrder() const
{
	char c_str[50];
	_CStringToCharArray(rudderOrder_num, c_str, 50);
	int rudderOrder = atoi(c_str);
	if (rudderOrder_symbol == "1")
		rudderOrder = -rudderOrder;
	return rudderOrder;
}

int ShipDataService::GetRudderAngle() const
{
	char c_str[50];
	_CStringToCharArray(rudderAngle_num, c_str, 50);
	int rudderAngle = atoi(c_str);
	if (rudderAngle_symbol == "1")
		rudderAngle = -rudderAngle;
	return rudderAngle;
}

CString ShipDataService::GetMMSI()
{
	return mmsi;
}

void ShipDataService::SetMMSI(CString MMSI)
{
	mmsi = MMSI;
}

//获取航速,单位knots
double ShipDataService::GetSpeed() const
{
	char c_str[50];
	_CStringToCharArray(speed, c_str, 50);
	double  fspeed = atof(c_str);
	return fspeed;
}

//获取总航程，单位NM
double ShipDataService::GetDistanceTotal() const
{
	char c_str[50];
	_CStringToCharArray(distance_total, c_str, 50);
	double  disTotal = atof(c_str);
	return disTotal;
}

//获取重置航程，单位NM
double ShipDataService::GetDistanceWP() const
{
	char c_str[50];
	_CStringToCharArray(distance_reset, c_str, 50);
	double  disWP = atof(c_str);
	return disWP;
}

//获取经度，度分格式dddmm.mmmm，正为E，负为W
double ShipDataService::GetLongitude()
{
	char c_str[50];
	_CStringToCharArray(longitude_num, c_str,50);
	double lon = atof(c_str);
	if (longitude_symbol.MakeUpper().Compare(_T("W"))==0)
		lon = -lon;
	return lon;
}

//获取纬度，度分格式ddmm.mmmm，正为N，负为S
double ShipDataService::GetLatitude()
{
	char c_str[50];
	_CStringToCharArray(latitude_num, c_str,50);
	double lat = atof(c_str);
	if (latitude_symbol.MakeUpper().Compare(_T("S"))==0)
		lat = -lat;
	return lat;
}



//获取左车钟
int ShipDataService::GetTelegraph_L() const
{
	char c_num_L[50];
	_CStringToCharArray(telegraph_num_L, c_num_L, 50);
	int telegraph_L = atoi(c_num_L);
	char c_symbol_L[50];
	_CStringToCharArray(telegraph_symbol_L, c_symbol_L, 50);
	if (atoi(c_symbol_L) == 1)
		telegraph_L = -telegraph_L;
	return telegraph_L;
}

//获取右车钟
int ShipDataService::GetTelegraph_R() const
{
	char c_num_R[50];
	_CStringToCharArray(telegraph_num_R, c_num_R, 50);
	int telegraph_R = atoi(c_num_R);
	char c_symbol_R[50];
	_CStringToCharArray(telegraph_symbol_R, c_symbol_R, 50);
	if (atoi(c_symbol_R) == 1)
		telegraph_R = -telegraph_R;
	return telegraph_R;
}

//获取左RPM
int ShipDataService::GetRPM_L() const
{
	char c_str[50];
	_CStringToCharArray(RPM_L, c_str, 50);
	int rpm_L = atoi(c_str);
	return rpm_L;
}

//获取右RPM
int ShipDataService::GetRPM_R() const
{
	char c_str[50];
	_CStringToCharArray(RPM_R, c_str, 50);
	int rpm_R = atoi(c_str);
	return rpm_R;
}

//获取左PITCH
int ShipDataService::GetPITCH_L() const
{
	char c_str[50];
	_CStringToCharArray(PITCH_L, c_str, 50);
	int pitch_L = atoi(c_str);
	return pitch_L;
}

//获取右PITCH
int ShipDataService::GetPITCH_R() const
{
	char c_str[50];
	_CStringToCharArray(PITCH_R, c_str, 50);
	int pitch_R = atoi(c_str);
	return pitch_R;
}

//获取左StartAir
int ShipDataService::GetStartAir_L() const
{
	char c_str[50];
	_CStringToCharArray(StartAir_L, c_str, 50);
	int startAir_L = atoi(c_str);
	return startAir_L;
}

//获取右StartAir
int ShipDataService::GetStartAir_R() const
{
	char c_str[50];
	_CStringToCharArray(StartAir_R, c_str, 50);
	int startAir_R = atoi(c_str);
	return startAir_R;
}

//返回舵机报警数据，在View中转化有用二进制
CString ShipDataService::GetsteeringAlarm()
{
	return steeringAlarm_all;
}

//返回主机报警数据
CString ShipDataService::GethostAlarm()
{
	return hostAlarm_all;
}

int ShipDataService::GetSingleSteeringAlarmNum()
{
	char c_num[50];
	_CStringToCharArray(singleSteeringAlarmNum, c_num,50);
	int num = atoi(c_num);
	return num;
}

int ShipDataService::GetSingleSteeringAlarmFlag()
{
	char c_num[50];
	_CStringToCharArray(singleSteeringAlarmflag, c_num,50);
	int num = atoi(c_num);
	return num;
}

int ShipDataService::GetSingleHostAlarmNum()
{
	char c_num[50];
	_CStringToCharArray(singleHostAlarmNum, c_num,50);
	int num = atoi(c_num);
	return num;
}

int ShipDataService::GetSingleHostAlarmFlag()
{
	char c_num[50];
	_CStringToCharArray(singleHostAlarmflag, c_num,50);
	int num = atoi(c_num);
	return num;
}

//设置舵令
void ShipDataService::SetRudderOrder(const int rudder)
{
	if (rudder >= 0)
	{
		rudderOrder_symbol = _T("0");
		rudderOrder_num.Format(_T("%d"), rudder);
	}

	else
	{
		rudderOrder_symbol = _T("1");
		rudderOrder_num.Format(_T("%d"), -rudder);
	}
}

//设置左车钟
void ShipDataService::SetTelegraph_L(const int tele_L)
{
	if (tele_L >= 0)
	{
		telegraph_num_L.Format(_T("%d"), tele_L);
		telegraph_symbol_L = _T("0");
	}
	else
	{
		telegraph_num_L.Format(_T("%d"), -tele_L);
		telegraph_symbol_L = _T("1");
	}
}

//设置右车钟
void ShipDataService::SetTelegraph_R(const int tele_R)
{
	if (tele_R >= 0)
	{
		telegraph_num_R.Format(_T("%d"), tele_R);
		telegraph_symbol_R = _T("0");
	}
	else
	{
		telegraph_num_R.Format(_T("%d"), -tele_R);
		telegraph_symbol_R = _T("1");
	}
}

int ShipDataService::GetStatus_CONSVR()
{
	return status_CONSVR;
}

int ShipDataService::GetStatus_PS()
{
	return status_PS;
}

int ShipDataService::GetStatus_SG()
{
	return status_SG;
}

int ShipDataService::GetStatus_LOG()
{
	return status_LOG;
}

int ShipDataService::GetStatus_ALARM()
{
	return status_ALARM;
}

int ShipDataService::GetStatus_RADAR()
{
	return status_RADAR;
}

int ShipDataService::GetStatus_ECDIS()
{
	return status_ECDIS;
}

int ShipDataService::GetStatus_VI()
{
	return status_VI;
}

CString ShipDataService::GetIP_CONSVR()
{
	return ip_CONSVR;
}

CString ShipDataService::GetIP_PS()
{
	return ip_PS;
}

CString ShipDataService::GetIP_SG()
{
	return ip_SG;
}

CString ShipDataService::GetIP_LOG()
{
	return ip_LOG;
}

CString ShipDataService::GetIP_ALARM()
{
	return ip_ALARM;
}

CString ShipDataService::GetIP_RADAR()
{
	return ip_RADAR;
}

CString ShipDataService::GetIP_ECDIS()
{
	return ip_ECDIS;
}

CString ShipDataService::GetIP_VI()
{
	return ip_VI;
}

void ShipDataService::SetStatus_CONSVR(int status)
{
	status_CONSVR = status;
}
void ShipDataService::SetStatus_PS(int status)
{
	status_PS = status;
}

void ShipDataService::SetStatus_SG(int status)
{
	status_SG = status;
}

void ShipDataService::SetStatus_LOG(int status)
{
	status_LOG = status;
}

void ShipDataService::SetStatus_ALARM(int status)
{
	status_ALARM = status;
}

void ShipDataService::SetStatus_RADAR(int status)
{
	status_RADAR = status;
}

void ShipDataService::SetStatus_ECDIS(int status)
{
	status_ECDIS = status;
}

void ShipDataService::SetStatus_VI(int status)
{
	status_VI = status;
}

void ShipDataService::SetIP_CONSVR(CString ip)
{
	ip_CONSVR = ip;
}

void ShipDataService::SetIP_PS(CString ip)
{
	ip_PS = ip;
}

void ShipDataService::SetIP_SG(CString ip)
{
	ip_SG = ip;
}

void ShipDataService::SetIP_LOG(CString ip)
{
	ip_LOG = ip;
}

void ShipDataService::SetIP_ALARM(CString ip)
{
	ip_ALARM = ip;
}

void ShipDataService::SetIP_RADAR(CString ip)
{
	ip_RADAR = ip;
}

void ShipDataService::SetIP_ECDIS(CString ip)
{
	ip_ECDIS = ip;
}

void ShipDataService::SetIP_VI(CString ip)
{
	ip_VI = ip;
}

//登录
//将身份、编号封装到协议，命令字0x00
void ShipDataService::GetNewHBT(BYTE msg[], int maxlength, BYTE identify, BYTE inum)
{
	//BYTE msg=new BYTE;
	if (maxlength < 10) return;
	int size = 10;
	msg[0] = 0x0A;
	msg[1] = 0x00;	//长度

	msg[2] = MSG_SESSION_LOGIN;	//命令字

	msg[3] = 0x01;
	msg[4] = 0x00;	//序号

	msg[5] = 0xFF;  //版本号

	msg[6] = identify;
	msg[7] = inum;

	WORD cal = CalculateCRC(msg, 8);
	msg[9] = cal / 256;
	msg[8] = cal % 256;

}

//将身份、编号、状态封装到协议，命令字0x01
void ShipDataService::GetNewHBT(BYTE msg[], int maxlength, BYTE identify, BYTE inum, BYTE istate)
{
	//BYTE msg=new BYTE;
	if (maxlength < 11) return;
	int size = 10;
	msg[0] = 0x0B;
	msg[1] = 0x00;	//长度

	msg[2] = MSG_SESSION_STATE;	//命令字

	msg[3] = 0x01;
	msg[4] = 0x00;	//序号

	msg[5] = 0xFF;  //版本号

	msg[6] = identify;
	msg[7] = inum;
	msg[8] = istate;

	WORD cal = CalculateCRC(msg, 9);
	msg[10] = cal / 256;
	msg[9] = cal % 256;

}

//将字符串封装到协议，命令字0x06
int ShipDataService::GetNewHBT(BYTE msg[], int maxlength, const char* Protocol)
{
	int proLen = strlen(Protocol);
	int size = (proLen + 8);
	if (maxlength < size)
		return 0;
	msg[0] = size % 256;
	msg[1] = size / 256; //长度

	msg[2] = MSG_SESSION_RELAY;	//命令字

	msg[3] = 0x01;
	msg[4] = 0x00;	//序号

	msg[5] = 0xFF;  //版本号

	int i = 0, j = 6;
	for (; i < proLen; i++, j++)
	{
		msg[j] = (BYTE)Protocol[i];
	}
	WORD cal = CalculateCRC(msg, j);
	msg[j] = cal % 256;
	msg[++j] = cal / 256;
	return size;
}

//添加校验后缀
void ShipDataService::AddCheckSum(CString& str)
{
	CString str1 = str;
	int len = str1.GetLength();
	int parity = 0;
	for (int i = 0; i<len; i++)
		parity = parity^str1[i];

	str.Format(_T("$%s*%.2X\r\n"), str1, parity);
}

CString ShipDataService::GetProtocol_HEHDT()
{
	if (!sta_mutex.try_lock())
		return NULL;
	CString str = _getProtocol_HEHDT();
	sta_mutex.unlock();
	return str;
}

CString ShipDataService::_getProtocol_HEHDT()
{
	double dHeading = GetHeading();
	if (dHeading < 0 || dHeading > 360)
	{
		return _T("");
	}

	CString mdata;
	//为了满足现在买的罗经必须送4位数的要求
	if (dHeading >= 100)//整数位本来就有3位
	{
		mdata.Format(_T("HEHDT,%.1f,T"), dHeading);
	}
	else if (dHeading >= 10)//整数位有2位
	{
		mdata.Format(_T("HEHDT,0%.1f,T"), dHeading);
	}
	else//只有一位的
	{
		mdata.Format(_T("HEHDT,00%.1f,T"), dHeading);
	}
	//	mdata.Format("HEHDT,%.1f,T", angle);
	AddCheckSum(mdata);

	return mdata;
}