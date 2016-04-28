/*
 * MimoFunTransmit.cpp
 *
 *  Created on: Apr 20, 2016
 *      Author: mcn
 */

#include "MimoFunTransmit.h"
#include <string>
#include <iostream>

#include "PracticalSocket.h"

MimoFunTransmit::MimoFunTransmit(std::string my_ip, unsigned short appPort,struct timeval tv, MimoFun *mimoFun):
	my_ip(my_ip),
	appPort(appPort),
	sendSock(appPort+1),
	recvSock(appPort),
	mimoFun(mimoFun)
{
	if(tv.tv_sec != 0 || tv.tv_usec != 0){
		recvSock.setTimeout(tv);
	}
}

void MimoFunTransmit::sendPkt(std::vector<uint8_t> pkt, std::string tar){
	try {
		sendSock.sendTo(pkt.data(), pkt.size(), tar, appPort);
	} catch (SocketException &e) {
		std::cerr << e.what() << std::endl;
		exit(1);
	}
}

int MimoFunTransmit::recvPkt(uint8_t* recvString, std::string &sourceAddress){
	int bytesRcvd;
	unsigned short sourcePort;         // Port of datagram source
	do{
		try {
			bytesRcvd = recvSock.recvFrom(recvString, MAXRCVSTRING, sourceAddress,sourcePort);
		} catch (SocketException &e) {
			std::cerr << e.what() << std::endl;
			exit(1);
		}
	}while(bytesRcvd>=0 && sourceAddress==my_ip); // self ip packet is skipped
	return bytesRcvd;
}

void MimoFunTransmit::genPktAndSendTo(MimoFun::pktType pType, std::string tar){
	vector<uint8_t> pkt = mimoFun->genPacket(pType);
	sendPkt(pkt,tar);
}

int MimoFunTransmit::recvPktAndWaitFor(MimoFun::pktType targetPType, std::string &tar){
	MimoFun::pktType pType;
	uint8_t recvString[MAXRCVSTRING + 1]; // Buffer for echo string + \0
	string sourceAddress;
	int bytesRcvd;
	do{
		bytesRcvd = recvPkt(recvString,sourceAddress);
		if(bytesRcvd<0) break; // time-out
		// react to QACK
		if(pType == MimoFun::Qack_p) genPktAndSendTo(MimoFun::Ack_p, sourceAddress);
		// Put packets into its decoder
		pType = mimoFun->recvPacket(recvString, bytesRcvd, sourceAddress);
	} while((tar.size()!=0 && sourceAddress != tar) || pType != targetPType);
	return bytesRcvd;
}
