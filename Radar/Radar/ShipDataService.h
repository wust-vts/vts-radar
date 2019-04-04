#pragma once
#include<mutex>
class ShipDataService
{
public:
	ShipDataService();
	~ShipDataService();

	inline static WORD CalculateCRC(const BYTE array[], int size)
	{
		int parity = 0;
		for (int i = 0; i < size; i++)
			parity = parity^array[i];
		WORD crc = (WORD)parity;
		return crc;
	}

	//Session通讯协议中的CRC校验，低字节在前，高字节在后    flag==FALSE时默认关闭CRC检查
	inline static bool CRCcheck(const BYTE array[], int size, BOOL flag = FALSE)
	{
		if (!flag)
			return true;
		if (size < 3)
			return false;
		int parity = 0;
		for (int i = 0; i < size - 2; i++)
			parity = parity^array[i];
		WORD crc = (WORD)parity;
		BYTE heigh = crc / 256;
		BYTE low = crc % 256;
		return((array[size - 2] == low) && (array[size - 1] == heigh));
	}

	void AddCheckSum(CString& str);//用于发送给真实罗经的数据协议，添加校验后缀

	
	enum SessionState
	{
		SS_NONE = 0,	//未登陆
		SS_DROPPED,		//非正常掉线，重新登陆会进行断线重连功能	
		SS_LOBBY,		//大厅状态，已登陆,但还未进入案例仿真
		SS_CASEPREPARE,//案例前的准备工作已就绪,例如完成了初始化
		SS_CASERUNNING,//正在进行案例仿真
		SS_MANAGING,	//正在管理  (保留类型，暂时未用),
		SS_ALL         //代表所有状态
	};

	enum SessionIdentity
	{
		SI_UNDEFINED = 0,			//未定义的身份
		SI_MANAGER,					//教练站-管理端(6)
		SI_MONITOR,					//瞭望监控端(3)
		SI_LOGIC_SERVER,				//逻辑服务器(3)
		SI_MAINSHIP_RENDERER,     //主本船渲染端(1-7)
		SI_SECONDARYSHIP_CLIENT,  //副本船渲染端(1-2)
		SI_BRIDGE_CHART,		      //驾驶台-电子海图(1-3)
		SI_BRIDGE_RADAR,		      //驾驶台-雷达模拟器(1-3)
		SI_BRIDGE_CONNING,		   //驾驶台-Conning(1-3)
		SI_DISPATCHER,				//应急调度与处置端(5)
		SI_PLAN_MONITOR,				//预案执行监控端(4)
		SI_ALL,                          //代表所有身份
	};

	static void GetNewHBT(BYTE msg[], int maxlength, BYTE identify, BYTE inum);//将身份、编号封装到协议，命令字0x00
	static void GetNewHBT(BYTE msg[], int maxlength, BYTE identify, BYTE inum, BYTE istate);//将身份、编号、状态封装到协议，命令字0x01
	static int GetNewHBT(BYTE msg[], int maxlength, const char* Protocol);//将字符串封装到协议，命令字0x06

	//-------------线程安全的--------------------------//
	CString GetProtocol_GPNRO();	//获取舵令协议
	CString GetProtocol_GPNRO_R();	//获取舵令协议（0左1右）
	CString GetProtocol_GPNRA();	//获取舵角协议
	CString GetProtocol_GPNRA_R();	//获取舵角协议(相反)
	CString GetProtocol_GPNCZ_L();	//获取左车钟协议
	CString GetProtocol_GPNCZ_R();	//获取右车钟协议
	CString GetProtocol_GPEST_L();	//获取左发动机状态协议
	CString GetProtocol_GPEST_R();	//获取右发动机状态协议
	CString GetProtocol_GPROT();	//获取转速率协议
	CString GetProtocol_GPDIS();	//获取航程协议
	CString GetProtocol_GPHED();	//获取船首向协议
	CString GetProtocol_GPSME();	//获取船舶主要航行信息协议
	CString GetProtocol_GPSTE();	//获取舵机报警协议
	CString GetProtocol_GPMAE();	//获取主机报警协议
	CString GetProtocol_GPSSE();	//获取舵机单灯报警协议
	CString GetProtocol_GPSMA();	//获取主机单灯报警协议
	CString GetProtocol_HEHDT();	//获取发送给真实罗经的协议
	//----------------------------------------------------------//

	//CString GetProtocol_HeartBeat(const char* IP, const char* name, int state);	//获取心跳包协议
	//CString GetProtocol_HeartBeat(CString IP, CString name, int state);	//获取心跳包协议

	//Decode为线程安全方法，内部调用_decode();
	bool Decode(const CString& message);		//解析协议
	bool Decode(const CString& message, CString& head);		//解析协议,并获取协议头
	bool Decode(const CString& message, CString& head, CString& GPHBT_name);	//GPHBT_name用于获取发送心跳包的名称
	bool DecodeRealTelegraph(const CString& message);	//解析真实车钟数据
	bool DecodeRealRudder(const BYTE msg[], int length);	//解析真实舵数据

	double GetHeading() const;
	double GetSpeed() const;
	double GetCourse() const;
	double GetDistanceTotal() const;
	double GetDistanceWP() const;
	double GetLongitude();
	double GetLatitude();
	double GetRotate() const;
	int GetRudderOrder() const;
	int GetRudderAngle() const;
	int GetTelegraph_L() const;
	int GetTelegraph_R() const;
	int GetRPM_L() const;
	int GetRPM_R() const;
	int GetPITCH_L() const;
	int GetPITCH_R() const;
	int GetStartAir_L() const;
	int GetStartAir_R() const;
	CString GetMMSI();
	CString GetsteeringAlarm();			//返回舵机报警数据，在View中转化有用二进制
	CString GethostAlarm();				//返回主机报警数据
	int GetSingleSteeringAlarmNum();	//返回单灯舵机报警的位置
	int GetSingleSteeringAlarmFlag();	//返回单灯舵机报警的亮灭情况
	int GetSingleHostAlarmNum();		//单灯主机报警的位置
	int GetSingleHostAlarmFlag();		//单灯主机报警的亮灭情况
	int GetStatus_CONSVR();
	int GetStatus_PS();
	int GetStatus_SG();
	int GetStatus_LOG();
	int GetStatus_ALARM();
	int GetStatus_RADAR();
	int GetStatus_ECDIS();
	int GetStatus_VI();
	CString GetIP_CONSVR();
	CString GetIP_PS();
	CString GetIP_SG();
	CString GetIP_LOG();
	CString GetIP_ALARM();
	CString GetIP_RADAR();
	CString GetIP_ECDIS();
	CString GetIP_VI();

	void SetMMSI(CString mmsi);
	void SetStatus_CONSVR(int status);
	void SetStatus_PS(int status);
	void SetStatus_SG(int status);
	void SetStatus_LOG(int status);
	void SetStatus_ALARM(int status);
	void SetStatus_RADAR(int status);
	void SetStatus_ECDIS(int status);
	void SetStatus_VI(int status);
	void SetIP_CONSVR(CString ip);
	void SetIP_PS(CString ip);
	void SetIP_SG(CString ip);
	void SetIP_LOG(CString ip);
	void SetIP_ALARM(CString ip);
	void SetIP_RADAR(CString ip);
	void SetIP_ECDIS(CString ip);
	void SetIP_VI(CString ip);

	void SetRudderOrder(const int rudder);
	void SetTelegraph_L(const int tele_L);
	void SetTelegraph_R(const int tele_R);

private:
	CString mmsi;				//船舶标识号mmsi，9位长度
	CString rudderOrder_symbol;	//舵令符号（0正/1负）
	CString rudderOrder_num;	//舵令数值
	CString rudderAngle_symbol;	//舵角符号（0正/1负）
	CString rudderAngle_num;	//舵角数值
	CString telegraph_symbol_L;	//左车钟符号（0正/1负）
	CString telegraph_num_L;	//左车钟数值（0,1,2,3,4,5）
	CString telegraph_symbol_R;	//右车钟符号（0正/1负）
	CString telegraph_num_R;	//右车钟数值（0,1,2,3,4,5）
	CString RPM_L;				//左RPM数值
	CString RPM_R;				//右RPM数值
	CString PITCH_L;			//左PITCH数值
	CString PITCH_R;			//右PITCH数值
	CString StartAir_L;			//左StartAir数值
	CString StartAir_R;			//右StartAir数值
	CString rotate_symbol;		//转速率符号（0正/1负）
	CString rotate_num;			//转速率，精确到小数点后两位
	CString distance_total;		//航行总路程，精确到小数点后两位
	CString distance_reset;		//重置后的航行总路程，精确到小数点后两位
	CString heading;			//船首向，精确到小数点后两位
	CString speed;				//船舶航行速度，单位节，XX.XX,精确到小数点后两位，Knots
	CString course;				//船舶航向，单位度，XX.XX,精确到小数点后两位，以正北为起点
	CString latitude_symbol;	//纬度符号（N/S）
	CString latitude_num;		//纬度ddmm.mmmm，度分格式（前导位数不足则补0）
	CString longitude_symbol;	//经度符号（W/E）
	CString longitude_num;		//经度dddmm.mmmm，度分格式（前导位数不足则补0）
	CString steeringAlarm_all;		//舵机报警，16进制格式的ASCII字符（XXYY，表示2字节16bit位，高位在前，低位在后，前置位不足补0），为二进制信号量组合而成，需解析
	//Bit位：15 14 13••••••••••••••••••••••••••••2 1 0
	//从低位0到高位15表，以0、1表示报警灯的亮灭，0灭1亮，多余的位数保留。
	//0：power failure
	//1：control failure
	//2：pressure reduction
	//3：pump1 overLoad
	//4：pump2 overLoad
	//5：spare overLoad
	//6：pump1 failure
	//7：pump2 failure
	//8：spare pump faiure
	//9~15位暂时保留
	CString hostAlarm_all;			//主机报警，16进制格式的ASCII字符（XXYYZZ，表示3字节24bit位，高位在前，低位在后，前置位不足补0），为二进制信号量组合而成，需解析
	//Bit位：23•••••••••••••15 14 13••••••••••••••••••••••••••••2 1 0
	//从低位0到高位23，以0、1表示报警灯的亮灭，0灭1亮，多余的位数保留。
	//0：over Load   过载
	//1：cool T.High
	//2：cool P.Low
	//3：Exh.Gas T.High
	//4：Scav Fire
	//5：Qil Mist
	//6：slow down activared
	//7：slut down activared
	//8：over speed
	//9：Lub oil P.Low
	//10：engine Failure
	//11：turn gear
	//12：reverse fait
	//13：start air P.Low
	//14：system error
	//15：Too long Start
	//16~23位暂时保留
	CString singleSteeringAlarmNum;	//单个舵机报警信号灯，第几个灯，即下标  0-8
	CString singleSteeringAlarmflag;//单个舵机报警信号灯亮灭，0-灭，1-亮
	CString singleHostAlarmNum;	//单个主机报警信号灯，第几个灯，即下标  0-15
	CString singleHostAlarmflag;//单个主机报警信号灯亮灭，0-灭，1-亮
private:
	//各设备的开关状态，0为在线，1为下线
	int status_CONSVR;	//ConningServices
	int status_SG;		//SG_Panel
	int status_PS;		//PS_Panel
	int status_LOG;		//LOG_Panel
	int status_ALARM;	//Alarm_Panel
	int status_RADAR;	//Radar
	int status_ECDIS;	//电子海图
	int status_VI;		//虚拟仪表
	//各设备的IP
	CString ip_CONSVR;
	CString ip_SG;
	CString ip_PS;
	CString ip_LOG;
	CString ip_ALARM;
	CString ip_RADAR;
	CString ip_ECDIS;
	CString ip_VI;

private:
	//-----------------线程不安全的----------------//
	bool _decode(const CString& message);
	bool _decode(const CString& message, CString& head);		//解析协议,并获取协议头
	bool _decode(const CString& message, CString& head, CString& GPHBT_name);	//GPHBT_name用于获取发送心跳包的名称
	bool _decodeRealTelegraph(const CString& message);	//解析真实车钟数据
	bool _decodeRealRudder(const BYTE msg[], int length);	//解析真实舵数据
	
	CString _getProtocol_GPNRO();	//获取舵令协议
	CString _getProtocol_GPNRO_R();	//获取舵令协议（0左1右）
	CString _getProtocol_GPNRA();	//获取舵角协议
	CString _getProtocol_GPNRA_R();	//获取舵角协议(相反)
	CString _getProtocol_GPNCZ_L();	//获取左车钟协议
	CString _getProtocol_GPNCZ_R();	//获取右车钟协议
	CString _getProtocol_GPEST_L();	//获取左发动机状态协议
	CString _getProtocol_GPEST_R();	//获取右发动机状态协议
	CString _getProtocol_GPROT();	//获取转速率协议
	CString _getProtocol_GPDIS();	//获取航程协议
	CString _getProtocol_GPHED();	//获取船首向协议
	CString _getProtocol_GPSME();	//获取船舶主要航行信息协议
	CString _getProtocol_GPSTE();	//获取舵机报警协议
	CString _getProtocol_GPMAE();	//获取主机报警协议
	CString _getProtocol_GPSSE();	//获取舵机单灯报警协议
	CString _getProtocol_GPSMA();	//获取主机单灯报警协议
	CString _getProtocol_HEHDT();	//获取发送给真实罗经的协议
	//---------------------------------------------//
};

// char* _CStringToCharArray(const CString str);
bool _CStringToCharArray(const CString cstr, char *pstr, int maxLen);
CString _GetCheckStr(const CString strSource);





