/*
 * MimoFunTransmit.h
 *
 *  Created on: Apr 20, 2016
 *      Author: mcn
 */

#ifndef MIMOFUNTRANSMIT_H_
#define MIMOFUNTRANSMIT_H_

#include <string>
#include <vector>
#include "PracticalSocket.h"
#include "MimoFun.h"

const int MAXRCVSTRING = 4096; // Longest string to receive

class MimoFunTransmit {
public:
	MimoFunTransmit(std::string my_ip, unsigned short appPort,struct timeval tv, MimoFun *mimoFun);
	void sendPkt(std::vector<uint8_t> pkt, std::string tar);
	int recvPkt(uint8_t* recvString, std::string &sourceAddress); // ignore my_ip and return pkt size
	void genPktAndSendTo(MimoFun::pktType pType, std::string tar);
	int recvPktAndWaitFor(MimoFun::pktType targetPType, std::string &tar);	// wait for pkt of target type from target source,
																			// return -1 if time out
private:
	std::string my_ip;
	unsigned short appPort;
	UDPSocket sendSock; // for send
	UDPSocket recvSock; // for receive
	MimoFun *mimoFun;
};

#endif /* MIMOFUNTRANSMIT_H_ */
