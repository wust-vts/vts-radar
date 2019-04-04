#include"stdafx.h"
#include"vtsRadar.h"
#include"MyString.h"
#include"vtsDEM.h"
#include"BitmapEx.h"
#include"PublicMethod.h"
#include<mutex>
#include<list>

using namespace std;

extern VtsDem vtsDem; //vtsDem
extern int m_vtsRadar_selected;
extern mutex mtx_vtsRadar_selected;
extern mutex mtx_vtsRadars;
extern BlackHost blackHost; //无效的ip端口列表


VtsRadar_s::VtsRadar_s() :host(){
	this->id = "";
	this->op = "";
	this->lat = 0;
	this->lng = 0;
	this->height = 0;
	this->range = 0;
}

VtsRadar_s::~VtsRadar_s(){

}

VtsRadar_s& VtsRadar_s::operator = (const VtsRadar_s& other){
	this->id = other.id;
	this->host = other.host;
	this->lat = other.lat;
	this->lng = other.lng;
	this->op = other.op;
	this->height = other.height;
	this->range = other.range;
	return *this;
}


VtsRadar::VtsRadar()
{
}

VtsRadar::VtsRadar(const VtsRadar& vtsRadar){
	id = vtsRadar.id;
	lng = vtsRadar.lng;
	lat = vtsRadar.lat;
	range = vtsRadar.range;
	height = vtsRadar.height;
	centerX = vtsRadar.centerX;
	centerY = vtsRadar.centerY;
	minX = vtsRadar.minX;
	minY = vtsRadar.minY;
	maxX = vtsRadar.maxX;
	maxY = vtsRadar.maxY;
	scale = vtsRadar.scale;
	horl = vtsRadar.horl;
	hosts = vtsRadar.hosts;
	dem = vtsRadar.dem;
	radarPicOrigin = vtsRadar.radarPicOrigin;
	radarPicWithShip = vtsRadar.radarPicWithShip;
}

VtsRadar::~VtsRadar()
{
}

std::string VtsRadar::getId() const{
	return this->id;
}

void VtsRadar::setId(const std::string& id){
	this->id = id;
}

double VtsRadar::getLat() const{
	return this->lat;
}

void VtsRadar::setLat(double lat){
	this->lat = lat;
}

double VtsRadar::getLng() const{
	return this->lng;
}

void VtsRadar::setLng(double lng){
	this->lng = lng;
}

double VtsRadar::getHeight() const{
	return this->height;
}

void VtsRadar::setHeight(double height){
	this->height = height;
}

double VtsRadar::getRange() const{
	return this->range;
}

void VtsRadar::setRange(double range){
	this->range = range;
}

double VtsRadar::getHorl() const{
	return this->horl;
}
void VtsRadar::setHorl(double horl){
	this->horl = horl;
}

std::list<Host> VtsRadar::getHosts() const{
	return this->hosts;
}

double VtsRadar::getCenterX() const{
	return this->centerX;
}

void VtsRadar::setCenterX(double centerX){
	this->centerX = centerX;
}

double VtsRadar::getCenterY() const{
	return this->centerY;
}

void VtsRadar::setCenterY(double centerY){
	this->centerY = centerY;
}

double VtsRadar::getMinX() const{
	return this->minX;
}
double VtsRadar::getMinY() const{
	return this->minY;
}

double VtsRadar::getMaxX() const{
	return this->maxX;
}
double VtsRadar::getMaxY() const{
	return this->maxY;
}
void VtsRadar::setMinX(double minX){
	this->minX = minX;
}
void VtsRadar::setMinY(double minY){
	this->minY = minY;
}
void VtsRadar::setMaxX(double maxX){
	this->maxX = maxX;
}
void VtsRadar::setMaxY(double maxY){
	this->maxY = maxY;
}

double VtsRadar::getScale() const{
	return this->scale;
}

void VtsRadar::setScale(double scale){
	this->scale = scale;
}

std::vector<std::vector<float>> VtsRadar::getDem() const{
	return this->dem;
}

void VtsRadar::setDem(const std::vector<std::vector<float>>& dem){
	//this->dem.clear();
	this->dem = dem;
}

//std::vector<std::vector<char>> VtsRadar::getRadarMatrix() const{
//	return this->radarMatrix;
//}
//
//void VtsRadar::setRadarMatrix(const std::vector<std::vector<char>>& radarMatrix){
//	this->radarMatrix = radarMatrix;
//}

void VtsRadar::removeHost(Host host){
	this->hosts.remove(host);
}

void VtsRadar::clearHosts(){
	this->hosts.clear();
}

bool VtsRadar::containsHost(const Host& host) const{
	std::list<Host>::const_iterator iter = this->hosts.begin();
	while (iter != this->hosts.end())
	{
		if (!iter->getIp().compare(host.getIp()) && iter->getPort() == host.getPort()){
			return true;
		}
		iter++;
	}
	return false;
}

void VtsRadar::insertHostIfNotContains(const Host& host){
	if (!containsHost(host)){
		this->hosts.push_back(host);
	}
}

string VtsRadar::toString() const{
	//char str[1024];
	//sprintf(str, "%s;%lf;%lf;%lf;%lf;",id,lng,lat,heignt,range);
	//string s(str);
	string s = id + ";" + d2s(lng) + ";" + d2s(lat) + ";" + d2s(height) + ";" + d2s(range) + ";" + d2s(horl);
	if (!hosts.empty()){
		list<Host>::const_iterator iter = hosts.begin();
		//sprintf(str, "%s:%d", iter->ip, iter->port);
		s += iter->getIp() + ":" + i2s(iter->getPort());
		while ((++iter) != hosts.end())
		{
			//sprintf(str, ",%s:%d", iter->ip, iter->port); 
			s += "," + iter->getIp() + ":" + i2s(iter->getPort());
		}
	}
	return s;
}

VtsRadar& VtsRadar::operator =(const VtsRadar& vtsRadar){
	id = vtsRadar.id;
	lng = vtsRadar.lng;
	lat = vtsRadar.lat;
	range = vtsRadar.range;
	height = vtsRadar.height;
	centerX = vtsRadar.centerX;
	centerY = vtsRadar.centerY;
	minX = vtsRadar.minX;
	minY = vtsRadar.minY;
	maxX = vtsRadar.maxX;
	maxY = vtsRadar.maxY;
	scale = vtsRadar.scale;
	horl = vtsRadar.horl;
	hosts = vtsRadar.hosts;
	dem = vtsRadar.dem;
	radarPicOrigin = vtsRadar.radarPicOrigin;
	radarPicWithShip = vtsRadar.radarPicWithShip;

	return *this;
}

std::string VtsRadar::radarMatrix2String(const std::vector<std::vector<char>>& matrix) const{
	if (matrix.empty()){
		return "";
	}
	/*size_t row = radarMatrix.size();
	size_t col = radarMatrix[0].size();
	size_t size = row*col + 10;
	char* str = new char[size];
	char* p = str;
	for (int i = 0; i < row; i++){
	for (int j = 0; j < col; j++){
	int t = radarMatrix[i][j];
	*p = t>0 ? '1' : '0';
	p++;
	}
	}
	*p = '\0';
	return string(str);*/
	size_t maxStrSize = 10000;
	size_t count = 0;
	char* str = new char[maxStrSize];
	char* temp = new char[matrix[0].size()];

	for (int i = 0; i < matrix.size(); i++){
		char * p = temp;
		size_t size = freCompression(matrix[i], p);

		if (count + size >= maxStrSize - 10){
			//若空间不够用，扩大存储
			maxStrSize *= 2;
			char* str_temp = new char[maxStrSize];
			//拷贝内容
			memcpy(str_temp, str, count);
			delete str;
			str = str_temp;
		}

		char* q = str + count;
		memcpy(q, temp, size);
		count += size;
	}
	*(str + count) = '\0';
	std::string cString(str);
	delete str;
	delete temp;
	return cString;
}

std::string byteCompression(const std::vector<char>& data){
	std::string result;
	for (int i = 0; i < data.size(); i += 8){
		BYTE a = 0x01;
		BYTE b = 0x00;
		for (int j = i; j < data.size(); j++){
			BYTE c = data[j] == 0 ? 0x00 : a;
			b = b | c;
			a = a << 1;
		}
		result += (char)b;
	}
	return result;
}

//按出现的频率压缩，0为-，1为+
size_t freCompression(const std::vector<char>& data, char* p){
	
	size_t count = 0;
	for (int i = 0; i < data.size();){
		int n = 0;
		if (data[i]>0){
			while (i < data.size() && data[i]>0 && n<127){
				i++;
				n++;
			}
		}
		else{
			while (i < data.size() && data[i] == 0 && n>-127){
				i++;
				n--;
			}
		}
		*(p++) = n;
		count++;
	}
	return count;
}

string vtsRadarMapToString(const vector<VtsRadar>& vtsRadarMap){
	string s;
	if (!vtsRadarMap.empty()){
		vector<VtsRadar>::const_iterator iter = vtsRadarMap.begin();
		//s.append(iter->second.toString());
		while (iter != vtsRadarMap.end())
		{
			s.append("\n");
			s.append(iter->toString());
			++iter;
		}
	}
	s += "#";
	return s;
}

std::string getVtsRadarSAMsg(const std::vector<VtsRadar>& vtsRadars){
	string s("VTSRADAR-SA");
	s.append(vtsRadarMapToString(vtsRadars));
	return s;
}

void decodeVtsRadarSMsg(const std::string& msg, std::vector<VtsRadar>& vtsRadars){
	size_t len = msg.find("#");
	string msg1 = msg.substr(0, len);
	vector<string> cells = split(msg1, ";");

	if (cells.size() < 9 || cells[0].compare("VTSRADAR-S")){
		return;
	}

	try{
		const std::string op = cells[1];

		if (!op.compare("AR")){
			opVtsRadarS_AR(cells, vtsRadars);
		}
		else if (!op.compare("AT")){
			opVtsRadarS_AT(cells, vtsRadars);
		}
		else if (!op.compare("AR&AT")){
			opVtsRadarS_ARAT(cells, vtsRadars);
		}
		else if (!op.compare("ER")){
			opVtsRadarS_ER(cells, vtsRadars);
		}
		else if (!op.compare("DT")){
			opVtsRadarS_DT(cells, vtsRadars);
		}
		else if (!op.compare("DR")){
			opVtsRadarS_DR(cells, vtsRadars);
		}
	}
	catch (std::exception& e){
		//msg无法解析
	}
}

void opVtsRadarS_AR(const std::vector<std::string>& cells, std::vector<VtsRadar>& vtsRadars){
	string id = cells[2];
	std::vector<VtsRadar>::iterator iter = vtsRadars.begin();

	while (iter != vtsRadars.end()){
		if (iter->getId().compare(id) == 0){
			break;
		}
		iter++;
	}
	if (iter == vtsRadars.end()){
		double lng = atof(cells[5].data());
		double lat = atof(cells[6].data());
		double height = atof(cells[7].data());
		double range = atof(cells[8].data());
		double horl = atof(cells[9].data());
		double centerX = 0;
		double centerY = 0;
		WebMercatorProjCal(lng, lat, centerX, centerY);
		VtsRadar vr;
		vr.setId(id);
		vr.setLng(lng);
		vr.setLat(lat);
		vr.setHeight(height);
		vr.setRange(range);
		vr.setCenterX(centerX);
		vr.setCenterY(centerY);
		vr.setMinX(centerX - range);
		vr.setMinY(centerY - range);
		vr.setMaxX(centerX + range);
		vr.setMaxY(centerY + range);
		vr.setHorl(horl);
		//VtsDemData radarUseDem = vtsDem.getVtsRadarUseDem(20930000, 3470000, range);
		VtsDemData radarUseDem = vtsDem.getVtsRadarUseDem(centerX, centerY, range);
		vr.setScale(radarUseDem.scale);
		vr.setDem(radarUseDem.data);
		//vr.generateRadarMatrix(horl);
		//drawRadarImage(vr.getRadarMatrix());
		vr.generateRadarPic();

		//push_back操作可能会使m_vtsRadar_selected的内存指向失效
		mtx_vtsRadars.lock();
		vtsRadars.push_back(vr);
		mtx_vtsRadars.unlock();
	}
}

void drawRadarImage(const std::vector<std::vector<char>>& data){
	int picWidth = data[0].size() * 2 - 1;
	DWORD white = _RGB(255, 255, 255);
	DWORD yellow = _RGB(255, 255, 0);
	DWORD black = _RGB(0, 0, 0);
	CBitmapEx bitmapEx;
	bitmapEx.Create(picWidth, picWidth);
	bitmapEx.Clear(white);
	const int centerX = (picWidth - 1) / 2;
	const int centerY = centerX;
	double angle = 0.0;
	for (int i = 0; i < data.size(); i++, angle += 0.5){
		for (int j = 0; j < data[i].size(); j++){
			int x = centerX + (int)(j*sin(_ToRad(angle)) + 0.5);
			int y = centerY - (int)(j*cos(_ToRad(angle)) + 0.5);
			if (data[i][j] != 0){
				bitmapEx.SetPixel(x, y, yellow);
			}
			else{
				bitmapEx.SetPixel(x, y, black);
			}
		}
	}
	bitmapEx.Save(_T(".\\pic\\data.bmp"));
}

void opVtsRadarS_AT(const std::vector<std::string>& cells, std::vector<VtsRadar>& vtsRadars){
	string id = cells[2];
	string ip = cells[3];
	u_short port = atoi(cells[4].data());
	Host host(ip, port);

	//删除ip黑名单
	blackHost.erase(host);

	vector<VtsRadar>::iterator iter = vtsRadars.begin();
	while (iter != vtsRadars.end()){
		if (iter->getId().compare(id) == 0){
			break;
		}
		iter++;
	}
	if (vtsRadars.end() != iter){
		iter->insertHostIfNotContains(host);
	}
}

void opVtsRadarS_ARAT(const std::vector<std::string>& cells, std::vector<VtsRadar>& vtsRadars){
	string id = cells[2];
	string ip = cells[3];
	u_short port = atoi(cells[4].data());
	Host host(ip, port);

	//删除ip黑名单
	blackHost.erase(host);

	std::vector<VtsRadar>::iterator iter = vtsRadars.begin();
	while (iter != vtsRadars.end()){
		if (iter->getId().compare(id) == 0){
			break;
		}
		iter++;
	}
	if (vtsRadars.end() != iter){
		iter->insertHostIfNotContains(host);
	}
	else
	{
		double lng = atof(cells[5].data());
		double lat = atof(cells[6].data());
		double height = atof(cells[7].data());
		double range = atof(cells[8].data());
		double horl = atof(cells[9].data());
		double centerX = 0;
		double centerY = 0;
		WebMercatorProjCal(lng, lat, centerX, centerY);
		VtsRadar vr;
		vr.setId(id);
		vr.setLng(lng);
		vr.setLat(lat);
		vr.setHeight(height);
		vr.setRange(range);
		vr.setCenterX(centerX);
		vr.setCenterY(centerY);
		vr.setMinX(centerX - range);
		vr.setMinY(centerY - range);
		vr.setMaxX(centerX + range);
		vr.setMaxY(centerY + range);
		vr.insertHostIfNotContains(host);
		vr.setHorl(horl);
		//VtsDemData radarUseDem = vtsDem.getVtsRadarUseDem(20930000, 3470000, range);
		VtsDemData radarUseDem = vtsDem.getVtsRadarUseDem(centerX, centerY, range);
		vr.setScale(radarUseDem.scale);
		vr.setDem(radarUseDem.data);
		//vr.generateRadarMatrix(horl);
		//drawRadarImage(vr.getRadarMatrix());
		vr.generateRadarPic();

		mtx_vtsRadars.lock();
		vtsRadars.push_back(vr);
		mtx_vtsRadars.unlock();
	}
}

void opVtsRadarS_ER(const std::vector<std::string>& cells, std::vector<VtsRadar>& vtsRadars)
{
	string id = cells[2];
	vector<VtsRadar>::iterator iter = vtsRadars.begin();
	while (iter != vtsRadars.end()){
		if (iter->getId().compare(id) == 0){
			break;
		}
		iter++;
	}
	if (vtsRadars.end() != iter){
		double lng = atof(cells[5].data());
		double lat = atof(cells[6].data());
		double height = atof(cells[7].data());
		double range = atof(cells[8].data());
		double horl = atof(cells[9].data());
		double centerX = 0;
		double centerY = 0;
		WebMercatorProjCal(lng, lat, centerX, centerY);
		iter->setLng(lng);
		iter->setLat(lat);
		iter->setHeight(height);
		iter->setRange(range);
		iter->setCenterX(centerX);
		iter->setCenterY(centerY);
		iter->setMinX(centerX - range);
		iter->setMinY(centerY - range);
		iter->setMaxX(centerX + range);
		iter->setMaxY(centerY + range);
		iter->setHorl(horl);
		//VtsDemData radarUseDem = vtsDem.getVtsRadarUseDem(20930000, 3470000, range);
		VtsDemData radarUseDem = vtsDem.getVtsRadarUseDem(centerX, centerY, range);
		iter->setScale(radarUseDem.scale);
		iter->setDem(radarUseDem.data);
		//iter->second.generateRadarMatrix(horl);
		iter->generateRadarPic();
	}
}

void opVtsRadarS_DT(const std::vector<std::string>& cells, std::vector<VtsRadar>& vtsRadars){
	string id = cells[2];
	vector<VtsRadar>::iterator iter = vtsRadars.begin();
	while (iter != vtsRadars.end()){
		if (iter->getId().compare(id) == 0){
			break;
		}
		iter++;
	}
	if (vtsRadars.end() != iter){
		string ip = cells[3];
		u_short port = atoi(cells[4].data());
		Host host(ip, port);
		iter->removeHost(host);
	}
}

void opVtsRadarS_DR(const std::vector<std::string>& cells, std::vector<VtsRadar>& vtsRadars){
	string id = cells[2];
	int i = 0;
	for (; i < vtsRadars.size(); i++){
		if (vtsRadars[i].getId().compare(id) == 0){
			break;
		}
	}
	if (i < vtsRadars.size()){
		mtx_vtsRadar_selected.lock();
		if (m_vtsRadar_selected == i){		
			m_vtsRadar_selected = -1;
		}
		vtsRadars[i].clearHosts();
		mtx_vtsRadars.lock();
		vtsRadars.erase(vtsRadars.begin() + i);
		mtx_vtsRadars.unlock();
		mtx_vtsRadar_selected.unlock();
	}
}

void VtsRadar::sendVtsRadarData() const{
	string title = "VTSRADAR-D";
	string id = this->id;
	string lng = d2s(this->lng);
	string lat = d2s(this->lat);
	string height = d2s(this->height);
	string range = d2s(this->range);
	string horl = d2s(this->horl);
	string scale = d2s(this->scale);
	string pointCount = "0";
	string lineCount = "0";
	std::vector<std::vector<char>> matrix;
	DWORD t1 = GetTickCount();
	getRadarMatrixFromBitmap(matrix, radarPicWithShip, _RGB(0, 0, 64));
	DWORD t2 = GetTickCount();
	DWORD t = t2 - t1;
	if (!matrix.empty()){
		pointCount = i2s(matrix[0].size());
		lineCount = i2s(matrix.size());
	}

	string msg = title + ";" + id + ";" + lng + ";" + lat + ";" + height + ";" + range + ";" + scale + ";" + horl + ";" + pointCount + ";" + lineCount + ";" + radarMatrix2String(matrix) + "#";

	const list<Host>& hosts = getHosts();

	for (list<Host>::const_iterator iter = hosts.begin(); iter != hosts.end(); iter++){
		if (blackHost.contains(*iter)){
			continue;
		}
		DWORD beginTime = GetTickCount();
		sendData(msg, *iter);
		DWORD endTime = GetTickCount();
		DWORD useTime = endTime - beginTime;
		//若目标无效会导致耗时较久,加入黑名单
		if (useTime > 10000){
			blackHost.insert(*iter);
		}
	}
}

void VtsRadar::sendData(const std::string& msg, const Host& host) const{
	//WSADATA wsaData;//初始化
	//SOCKET SendSocket;
	//sockaddr_in RecvAddr;//服务器地址
	//int Port = host.port;//服务器监听地址
	//
	////初始化Socket
	//WSAStartup(MAKEWORD(2, 2), &wsaData);
	////创建Socket对象
	//SendSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	////设置服务器地址
	//RecvAddr.sin_family = AF_INET;
	//RecvAddr.sin_port = htons(Port);
	//RecvAddr.sin_addr.s_addr = inet_addr(host.ip.data());
	////发送数据报
	//sendto(SendSocket, msg.data(), strlen(msg.data())+1, 0, (SOCKADDR *)&RecvAddr, sizeof(RecvAddr));
	////发送完成，关闭Socket
	////printf("finished sending,close socket.\n");
	//closesocket(SendSocket);
	////printf("Exting.\n");
	//WSACleanup();
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

	wVersionRequested = MAKEWORD(1, 1);//第一个参数为低位字节；第二个参数为高位字节  

	err = WSAStartup(wVersionRequested, &wsaData);//对winsock DLL（动态链接库文件）进行初始化，协商Winsock的版本支持，并分配必要的资源。  
	if (err != 0)
	{
		return;
	}

	if (LOBYTE(wsaData.wVersion) != 1 || HIBYTE(wsaData.wVersion) != 1)//LOBYTE（）取得16进制数最低位；HIBYTE（）取得16进制数最高（最左边）那个字节的内容        
	{
		WSACleanup();
		return;
	}
	SOCKET sockClient = socket(AF_INET, SOCK_STREAM, 0);

	SOCKADDR_IN addrClt;//需要包含服务端IP信息  
	addrClt.sin_addr.S_un.S_addr = inet_addr(host.getIp().data());// inet_addr将IP地址从点数格式转换成网络字节格式整型。  
	addrClt.sin_family = AF_INET;
	addrClt.sin_port = htons(host.getPort());

	connect(sockClient, (SOCKADDR*)&addrClt, sizeof(SOCKADDR));//客户机向服务器发出连接请求  
	//char recvBuf[50];
	//recv(sockClient, recvBuf, 50, 0);
	//printf("my reply is : %s\n", recvBuf);

	//char sendBuf[50];
	//strcat(sendBuf, "server node of: yaopeng");
	//char * buffer = new char[msg.size() + 10];
	//memcpy(buffer, msg.data(), sizeof(char)*msg.size());
	//buffer[msg.size()] = 0;
	send(sockClient, msg.data(), msg.size(), 0);

	closesocket(sockClient);
	WSACleanup();

	
	//delete buffer;
}

//void VtsRadar::generateRadarMatrix(double horl){
//	using namespace std;
//
//	const vector<vector<float>>& dem = this->dem;
//	double scale = this->scale;
//	int radius = dem.size() / 2 + 1;
//	const int centerX = radius;  //dem中心点X坐标
//	const int centerY = radius;  //dem中心点Y坐标
//	const double angleStep = 0.5; //每隔0.5度取扫描线
//
//	//开始计算
//	radarMatrix.clear();
//	for (double angle = 0; angle < 360; angle += angleStep){
//		try{
//			vector<float> scanLineDem = getScanLineDem(dem, angle, radius, centerX, centerY);
//			vector<char> scanLineMatrix = getScanLineMatrix(scanLineDem, scale, horl, this->heignt);
//			radarMatrix.push_back(scanLineMatrix);
//		}
//		catch (exception e){
//			printf(e.what());
//		}
//	}
//}

std::vector<float> VtsRadar::getScanLineDem(const std::vector<std::vector<float>>& dem, double angle, int radius, int centerX, int centerY){
	using namespace std;
	vector<float> scanLineDem;
	if (dem.size() == 0){
		return scanLineDem;
	}
	for (int i = 0; i < radius; i++){
		int x = centerX + (int)(i*sin(_ToRad(angle)) + 0.5);
		x = x >= dem[0].size() ? dem[0].size() - 1 : x;
		x = x < 0 ? 0 : x;

		int y = centerY - (int)(i*cos(_ToRad(angle)) + 0.5);
		y = y >= dem.size() ? dem.size() - 1 : y;
		y = y < 0 ? 0 : y;

		scanLineDem.push_back(dem[y][x]);
	}
	return scanLineDem;
}

std::vector<char> VtsRadar::getScanLineMatrix(const std::vector<float>& scanLineDem, double scale, double horl, double radarHeight){
	using namespace std;
	vector<char> scanLineMatrix;
	if (scanLineDem.size() == 0){
		return scanLineMatrix;
	}

	const double maxHorl = horl + 5;//默认最大海平面高度为horl+5
	const double realRadarHeight = scanLineDem[0] + radarHeight;
	double maxPitch = 0;//最大俯仰角
	scanLineMatrix.push_back(FALSE);
	for (int i = 1; i < scanLineDem.size(); i++){
		//低于最大水平面不显示
		const double targetHeight = scanLineDem[i];
		if (targetHeight < maxHorl){
			scanLineMatrix.push_back(FALSE);
			continue;
		}
		if (targetHeight < realRadarHeight){
			if (maxPitch >= PI / 2){
				scanLineMatrix.push_back(FALSE);
				continue;
			}
			else
			{
				double pitch = atan(i*scale / (realRadarHeight - targetHeight));
				scanLineMatrix.push_back(pitch >= maxPitch ? TRUE : FALSE);
				maxPitch = pitch > maxPitch ? pitch : maxPitch;
				continue;
			}
		}
		else
		{
			double pitch = PI / 2 + atan((targetHeight - realRadarHeight) / (i*scale));
			scanLineMatrix.push_back(pitch >= maxPitch ? TRUE : FALSE);
			maxPitch = pitch > maxPitch ? pitch : maxPitch;
			continue;
		}

	}
	return scanLineMatrix;
}

void VtsRadar::generateRadarPic(){
	using namespace std;
	double gain = 1;
	if (dem.size() == 0){
		return;
	}
	const int centerY = (dem.size() - 1) / 2;
	const int centerX = (dem[0].size() - 1) / 2;
	const double scale = getScale();
	const float horl = getHorl(); //水平线
	const float radarHeight = getHeight() + dem[centerY][centerX];

	bool** bcheck = new bool*[dem.size()];
	for (int i = 0; i < dem.size(); i++){
		bcheck[i] = new bool[dem[0].size()];
		memset(bcheck[i], false, dem[0].size()*sizeof(bool));
	}
	CBitmapEx bitmapEx;
	bitmapEx.Create(dem[0].size(), dem.size());
	bitmapEx.Clear(_RGB(0, 0, 64));

	int x = 0, y = 0;	//@当前扫面点坐标

	double obsH;  //@遮挡点高度
	//开始计算
	for (x = 0; x < dem[0].size(); x++){
		int w = abs(x - centerX); //@直角三角形的横向边
		int h = abs(y - centerY); //@直角三角形的纵向边
		int maxL = max(w, h);
		PointF* p = new PointF[dem.size() / 2 + 5];
		getScanLinePoints(p, centerX, centerY, x, y, w, h, maxL);
		double Rmax = scale*sqrt((p[maxL].X - centerX)*(p[maxL].X - centerX) + (p[maxL].Y - centerY)*(p[maxL].Y - centerY));	//@扫描线像素长度
		getScanLinePixel(bcheck, p, dem, centerX, centerY, maxL, horl, radarHeight, Rmax, scale, gain, bitmapEx);
		delete p;
	}

	y = dem.size() - 1;
	for (x = 0; x < dem[0].size(); x++)
	{
		int w = abs(x - centerX); //@直角三角形的横向边
		int h = abs(y - centerY); //@直角三角形的纵向边
		int maxL = max(w, h);
		PointF *p = new PointF[dem.size() / 2 + 5];
		getScanLinePoints(p, centerX, centerY, x, y, w, h, maxL);
		double Rmax = scale*sqrt((p[maxL].X - centerX)*(p[maxL].X - centerX) + (p[maxL].Y - centerY)*(p[maxL].Y - centerY));	//@扫描线像素长度
		getScanLinePixel(bcheck, p, dem, centerX, centerY, maxL, horl, radarHeight, Rmax, scale, gain, bitmapEx);
		delete p;
	}

	x = 0;
	for (y = 0; y < dem.size(); y++)
	{
		int w = abs(x - centerX); //@直角三角形的横向边
		int h = abs(y - centerY); //@直角三角形的纵向边
		int maxL = max(w, h);
		PointF *p = new PointF[dem.size() / 2 + 5];
		getScanLinePoints(p, centerX, centerY, x, y, w, h, maxL);
		double Rmax = scale*sqrt((p[maxL].X - centerX)*(p[maxL].X - centerX) + (p[maxL].Y - centerY)*(p[maxL].Y - centerY));	//@扫描线像素长度
		getScanLinePixel(bcheck, p, dem, centerX, centerY, maxL, horl, radarHeight, Rmax, scale, gain, bitmapEx);
		delete p;
	}

	x = dem[0].size() - 1;
	for (y = 0; y < dem.size(); y++)
	{
		int w = abs(x - centerX); //@直角三角形的横向边
		int h = abs(y - centerY); //@直角三角形的纵向边
		int maxL = max(w, h);
		PointF *p = new PointF[dem.size() / 2 + 5];
		getScanLinePoints(p, centerX, centerY, x, y, w, h, maxL);
		double Rmax = scale*sqrt((p[maxL].X - centerX)*(p[maxL].X - centerX) + (p[maxL].Y - centerY)*(p[maxL].Y - centerY));	//@扫描线像素长度
		getScanLinePixel(bcheck, p, dem, centerX, centerY, maxL, horl, radarHeight, Rmax, scale, gain, bitmapEx);
		delete p;
	}
	radarPicOrigin.Create(bitmapEx);
	//radarPicOrigin.Create(bitmapEx.GetWidth(), bitmapEx.GetHeight());
	//radarPicOrigin.Draw(0, 0, radarPicOrigin.GetWidth(), radarPicOrigin.GetHeight(), bitmapEx, 0, 0, bitmapEx.GetWidth(), bitmapEx.GetHeight());
	radarPicOrigin.BilateralBlur(2);	//默认为2
	radarPicOrigin.Save(_T("../Radar/pic/DEM_mem.bmp"));

	//销毁
	for (int i = 0; i < dem.size(); i++){
		delete bcheck[i];
	}
	delete[] bcheck;
}

void VtsRadar::getScanLinePoints(PointF *p, int centerX, int centerY, int x, int y, int w, int h, int maxL){
	for (int i = 0; i <= maxL; i++)
	{
		if (w > h)
		{
			int a = (int)((double)h*i / w + 0.5);
			p[i].X = (x > centerX) ? (centerX + i) : (centerX - i);
			p[i].Y = (y > centerY) ? (centerY + a) : (centerY - a);
		}
		else
		{
			int a = (int)((double)w*i / h + 0.5);
			p[i].X = (x > centerX) ? (centerX + a) : (centerX - a);
			p[i].Y = (y > centerY) ? (centerY + i) : (centerY - i);
		}
	}
}

void VtsRadar::getScanLinePixel(bool ** bcheck, PointF* p, const std::vector<std::vector<float>>& dem, int centerX, int centerY,
	int maxL, double horl, double radarHeight, double Rmax, double scale, double gain, CBitmapEx & bitmapEx){
	double maxAng = 0;
	const double t1 = 0.1;
	const double t2 = 1 - t1;
	for (int i = 1; i <= maxL; i++)
	{
		if (bcheck[(int)p[i].Y][(int)p[i].X]){
			continue;
		}
		bcheck[(int)p[i].Y][(int)p[i].X] = true;
		double tarH = dem[(int)p[i].Y][(int)p[i].X]; //@扫描目标点高度
		double R = scale*sqrt((p[i].X - centerX)*(p[i].X - centerX) + (p[i].Y - centerY)*(p[i].Y - centerY));	//@离雷达物理距离（米）

		if (tarH <= horl)	//当目标地势低于本船地势时，默认目标处于水域或无障碍物，不可反射电磁波
		{
			continue;
		}
		double angle = 0; //准线角
		if (tarH <= radarHeight)
		{
			angle = atan(R / (radarHeight - tarH));
		}
		else
		{
			angle = PI / 2 + atan((tarH - radarHeight) / R);
		}
		if (angle >= maxAng)
		{
			maxAng = angle;
			//计算亮度
			double k1 = (t1*atan(tarH / 10.0f)) / (PI / 2);
			double slope = atan((tarH - dem[(int)p[i - 1].Y][(int)p[i - 1].X]) / (scale*sqrt((p[i].X - p[i - 1].X)*(p[i].X - p[i - 1].X) + (p[i].Y - p[i - 1].Y)*(p[i].Y - p[i - 1].Y))));	//@斜角
			double angleTrans = AngleTrans(angle - slope);
			double k2 = t2*cos(angleTrans);
			//	double k2 = 10*t2*cos(angle[i] - slope);
			double t = k1 + k2;
			double Pb = t*(1 - (R / Rmax)*(R / Rmax)*(R / Rmax)*(R / Rmax));

			Pb = max(Pb, 0);
			Pb = min(Pb, 1);

			int Gray_R = 255 * Pb*gain;
			if (Gray_R > 255) Gray_R = 255;
			DWORD Gray = _RGB(Gray_R, Gray_R, 0);

			bitmapEx.SetPixel((int)p[i].X, (int)p[i].Y, Gray);
		}
	}
}

void VtsRadar::generateRadarWithShipPic(vector <SHIP> ships){
	if (!radarPicOrigin.IsValid()){
		return;
	}
	CBitmapEx bmpMem(radarPicOrigin);
	DWORD shipColor = _RGB(255, 255, 0);
	vector<SHIP>::const_iterator iter = ships.begin();
	while (iter != ships.end()){
		//绘制图像
		drawShipOnBmp(&bmpMem, *iter, shipColor);
		iter++;
	}
	radarPicWithShip = bmpMem;

	//测试写入文件
	/*string path = "./pic/" + id + "_with_ship.bmp";
	int length = path.size() + 1;
	LPTSTR tcBuffer = new TCHAR[length];
	memset(tcBuffer, 0, length);
	MultiByteToWideChar(CP_ACP, 0, path.data(), path.size(), tcBuffer, path.size());
	tcBuffer[path.size()] = 0;
	radarPicWithShip.Save(tcBuffer);
	delete tcBuffer;*/
}

bool VtsRadar::isShipInRadar(const SHIP& ship)const{
	return ship.dadiX > minX&&ship.dadiX<maxX&&ship.dadiY>minY&&ship.dadiY < maxY;
}

void VtsRadar::drawShipOnBmp(CBitmapEx* bmp, const SHIP& ship, DWORD color){
	//判断船是否在雷达范围内
	if (!isShipInRadar(ship)){
		return;
	}
	int widthPixel = ship.width / scale;
	widthPixel = max(widthPixel, 2);
	int lengthPixel = ship.length / scale;
	lengthPixel = max(lengthPixel, 8);

	int bmpCenterX = bmp->GetWidth() / 2;
	int bmpCenterY = bmp->GetHeight() / 2;
	int shipX, shipY;
	GetPixelPointFromDadi(shipX, shipY, ship.dadiX, ship.dadiY, bmpCenterX, bmpCenterY, centerX, centerY, scale);
	Rect rect(shipX - widthPixel / 2, shipY - lengthPixel / 2, widthPixel, lengthPixel);
	for (int y = rect.Y; y <= rect.Y + rect.Height; y++)
	{
		for (int x = rect.X; x <= rect.X + rect.Width; x++)
		{
			if (!isPointInEllipse(rect, x, y))
				continue;
			else
			{
				int a = x, b = y;
				RotatePoint(&a, &b, CPoint(shipX, shipY), _ToRad(ship.heading));
				bmp->SetPixel(a, b, color);
			}
		}
	}
}

/**
* matrix 计算结果
* bmp 数据来源图像
* empty 雷达视频阴影区的像素值
**/
void getRadarMatrixFromBitmap(std::vector<std::vector<char>>& matrix, const CBitmapEx& bmp, DWORD empty){
	using namespace std;
	const long width = bmp.GetWidth();
	const long height = bmp.GetHeight();
	matrix.clear();
	if (!bmp.IsValid() || !width || !height){
		return;
	}

	const double angleStep = 0.5; //间隔0.5度
	const double maxAngle = 360; //一圈360度
	const int centerX = (int)((width - 1) / 2);
	const int centerY = (int)((height - 1) / 2);
	const int radius = min(centerX, centerY);
	for (double angle = 0; angle < maxAngle; angle += angleStep){
		vector<char> line;
		for (int r = 0; r < radius; r++){
			int x = centerX + (int)(r*sin(_ToRad(angle)) + 0.5);
			x = x >= width ? width : x;
			x = x < 0 ? 0 : x;

			int y = centerY - (int)(r*cos(_ToRad(angle)) + 0.5);
			y = y >= height ? height - 1 : y;
			y = y < 0 ? 0 : y;

			char c = bmp.GetPixel(x, y) == empty ? 0 : 1;
			line.push_back(c);
		}
		matrix.push_back(line);
	}
}