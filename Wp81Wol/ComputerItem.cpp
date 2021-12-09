#include "pch.h"
#include "ComputerItem.h"

using namespace Wp81Wol;

ComputerItem::ComputerItem(Platform::String ^ Name, Platform::String ^ MacAddress) : name{ Name }, macAddress{ MacAddress }
{
}

Platform::String^ ComputerItem::getName() {

	return this->name;
}

void ComputerItem::setName(Platform::String^ Name) {
	this->name = Name; 
}

Platform::String^ ComputerItem::getMacAddress() {

	return this->macAddress;
}

void ComputerItem::setMacAddress(Platform::String^ MacAddress) {
	this->macAddress = MacAddress; 
}