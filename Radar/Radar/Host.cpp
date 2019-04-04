#include"stdafx.h"
#include"Host.h"

Host::Host(){
	this->ip = "";
	this->port = 0;
}

Host::Host(const std::string& ip, u_short port){
	this->ip = ip;
	this->port = port;
}

Host::~Host()
{
}

Host& Host::operator = (const Host& other){
	this->ip = other.ip;
	this->port = other.port;
	return *this;
}

bool Host::operator ==(const Host& other)const{
	return (this->ip.compare(other.ip) == 0) && (this->port == other.port);
}

BlackHost::BlackHost()
{
}

BlackHost::~BlackHost()
{
}

bool BlackHost::insert(const Host& host){
	for (std::vector<Host>::const_iterator it = blackList.begin(); it != blackList.end(); it++){
		if (*it == host){
			return false;
		}
	}
	blackList.push_back(host);
}

bool BlackHost::erase(const Host& host){
	bool isFound = false;
	for (std::vector<Host>::const_iterator it = blackList.begin(); it != blackList.end(); it++){
		if (*it == host){
			it = blackList.erase(it);
			isFound = true;
		}
	}
	return isFound;
}

bool BlackHost::contains(const Host& host){
	for (std::vector<Host>::const_iterator it = blackList.begin(); it != blackList.end(); it++){
		if (*it == host){
			return true;
		}
	}
	return false;
}