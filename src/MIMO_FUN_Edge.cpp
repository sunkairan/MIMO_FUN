/*
 *   C++ sockets on Unix and Windows
 *   Copyright (C) 2002
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <iostream>           // For cout and cerr
#include <cstdlib>            // For atoi()
#include <string>
#include <vector>
#include <cassert>
#include <map>

#include "PracticalSocket.h"  // For UDPSocket and SocketException

#include <Eigen/Dense>	// For matrix

#include <kodocpp/kodocpp.hpp> // For de/encoders

#include <unistd.h>           // For sleep()

#include "MimoFun.h"

#include "ConfigRead.h"

#include "MimoFunTransmit.h"


using namespace std;

int MAXRETRY = 7;

int main(int argc, char *argv[]) {
  if(argc != 2){
	  cerr << "Usage: " << argv[0] << " <config file name>\n";
	  exit(1);
  }

  // parse config file
  ConfigRead config(argv[1]);
  assert(config.read_single("node_type") == "edge");
  MimoFun::nodeType nType = MimoFun::Edge_n;
  string my_ip = config.read_single("my_ip");
  string broadcast_ip = config.read_single("broadcast_ip");
  vector<string> upstream = config.read_vector("upstream");
  vector<string> downstream = config.read_vector("downstream");
  uint32_t terminal_num = config.read_single_i("terminal_num");
  uint32_t symbol_size = config.read_single_i("symbol_size");
  unsigned short appPort = config.read_single_i("appPort");
  struct timeval tv;
  tv.tv_sec = config.read_single_i("time_out_in_s");
  tv.tv_usec = config.read_single_i("time_out_in_us");//500000; // in microsecond: 10^-6 s
  MAXRETRY = config.read_single_i("max_retry");
  bool verbose = config.read_single_i("verbose");

  vector<string> all_neighbors;
  all_neighbors.insert(all_neighbors.end(),upstream.begin(),upstream.end());
  all_neighbors.insert(all_neighbors.end(),downstream.begin(),downstream.end());

  // initialize MIMO FUN coder
  MimoFun mimoFun(nType, terminal_num, symbol_size, my_ip, upstream, downstream, verbose);

  // initialize sockets and transmitter
  MimoFunTransmit mimoFunTransmit(my_ip, appPort, tv, &mimoFun);



  // S1 ~ S3: collect packets from terminals
  for(auto tar: upstream){
	  int retry_time = 0;
	  //uint8_t recvString[MAXRCVSTRING + 1]; // Buffer for echo string + \0
	  string sourceAddress;              // Address of datagram source
	  while(retry_time < MAXRETRY){
		  // send out POLL
		  mimoFunTransmit.genPktAndSendTo(MimoFun::Poll_p, tar);
		  cout<<"send poll to "<< tar <<endl;

		  // wait for DATA
		  int bytesRcvd = mimoFunTransmit.recvPktAndWaitFor(MimoFun::Data_p, tar);

		  // handle time out
		  if(bytesRcvd<0){
			  cout<<"COLLECT DATA FROM " << tar << " TIME OUT, TRY AGAIN." <<endl;
			  retry_time++;
			  continue; // retry POLL
		  }
		  else{
			  cout<<"pkt received from "<< tar <<endl;
			  break;	// get DATA from source
		  }
	  }
	  assert(retry_time < MAXRETRY); // not implemented yet
  }

  // S4: check if rank is full
  while(mimoFun.get_decoder_rank() < terminal_num)
  {
	  cout<<"Waiting for backbone"<<endl;
	  uint8_t recvString[MAXRCVSTRING + 1];
	  string sourceAddress;
	  int bytesRcvd = mimoFunTransmit.recvPkt(recvString, sourceAddress);
	  if(bytesRcvd < 0) continue;
	  MimoFun::pktType pType = mimoFun.recvPacket(recvString, bytesRcvd, sourceAddress);
	  switch(pType) {
	  case MimoFun::Data_p :
		  break;
	  case MimoFun::Ack_p :
		  break;
	  case MimoFun::Qack_p :
		  cout<<"respond qack to "<< sourceAddress << endl;
		  mimoFunTransmit.genPktAndSendTo(MimoFun::Ack_p, broadcast_ip);
		  break;
	  case MimoFun::Poll_p :
		  cout<<"respond poll to "<< sourceAddress << endl;
		  mimoFunTransmit.genPktAndSendTo(MimoFun::Data_p, broadcast_ip);
		  break;
	  default:
		  cerr << "Unidentified packet type received!" << endl;
		  exit(1);
	  }

	  /*int retry_time = 0;
	  int bytesRcvd = 0;
	  // S5, S6, S7: wait for poll of backbone and broadcast to backbone
	  while(mimoFun.can_contribute_to_all(downstream)){
		  mimoFunTransmit.genPktAndSendTo(MimoFun::Data_p, broadcast_ip);

		  for(auto tar: all_neighbors){
			  retry_time = 0;
			  while(retry_time < MAXRETRY){
				  // send out QACK
				  mimoFunTransmit.genPktAndSendTo(MimoFun::Qack_p, tar);

				  // wait for ACK
				  bytesRcvd = mimoFunTransmit.recvPktAndWaitFor(MimoFun::Ack_p, tar);

				  // handle time out
				  if(bytesRcvd<0){
					  cout<<"QACK FROM " << tar << " TIME OUT, TRY AGAIN." <<endl;
					  retry_time++;
					  continue; // retry QACK
				  }
				  else{
					  break;	// get ACK from source
				  }
			  }
			  assert(retry_time < MAXRETRY); // not implemented yet
		  }
	  }

	  // S8, S9: receive from backbone
	  // retry_time = 0;
	  // bytesRcvd = -1;
	  do{
		  bytesRcvd = mimoFunTransmit.recvPktAndWaitFor(MimoFun::Data_p, downstream.front());
	  }while(bytesRcvd <= 0);
	*/
  }

  // S10: check if all terminals are finished
  while(mimoFun.can_contribute_to_any(upstream))
  {
	  // S11: broadcast packets to terminals
	  // generate and send data packet
	  mimoFunTransmit.genPktAndSendTo(MimoFun::Data_p, broadcast_ip);
	  cout<<"broadcast pkt to edges"<<endl;

	  // Request ACK from every terminal
	  for(auto tar: upstream){
		  int retry_time = 0;
		  while(retry_time < MAXRETRY){
			  // send out QACK
			  mimoFunTransmit.genPktAndSendTo(MimoFun::Qack_p, tar);
			  cout<<"send qack to "<< tar << endl;

			  // wait for ACK
			  int bytesRcvd = mimoFunTransmit.recvPktAndWaitFor(MimoFun::Ack_p, tar);

			  // handle time out
			  if(bytesRcvd<0){
				  cout<<"QACK FROM " << tar << " TIME OUT, TRY AGAIN." <<endl;
				  retry_time++;
				  continue; // retry QACK
			  }
			  else{
				  cout<<"ack received from "<< tar << endl;
				  break;	// get ACK from source
			  }
		  }
		  assert(retry_time < MAXRETRY); // not implemented yet
	  }
  }

  return 0;
}
