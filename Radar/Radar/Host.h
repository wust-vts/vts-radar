#pragma once

#include<string>
#include<vector>

class Host{
private:
	std::string ip;
	u_short port;

public:
	Host();
	Host(const std::string& ip, u_short port);
	~Host();
	Host& operator =(const Host& other);
	bool operator ==(const Host& other)const;

	std::string getIp() const { return this->ip; }
	void setIp(const std::string& ip){ this->ip = ip; }
	u_short getPort()const{ return this->port; }
	void setPort(const u_short& port){ this->port = port; }
};

class BlackHost
{
public:
	BlackHost();
	~BlackHost();

	bool insert(const Host& host);
	bool erase(const Host& host);
	bool contains(const Host& host);

private:
	std::vector<Host> blackList;
};