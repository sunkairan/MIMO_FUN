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
  if(argc != 2 && argc !=3){
	  cerr << "Usage: " << argv[0] << " <config file name> [<input file name>]\n";
	  exit(1);
  }

  // parse config file
  ConfigRead config(argv[1]);
  assert(config.read_single("node_type") == "terminal");
  MimoFun::nodeType nType = MimoFun::Terminal_n;
  string my_ip = config.read_single("my_ip");
  uint32_t senderId = config.read_single_i("senderId");
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

  // read input file
  std::vector<uint8_t> data_in;
  if(argc == 2){
	  // set to default dummy messages, AAAAAAA, BBBBBBB, ....
	  data_in = std::vector<uint8_t>(symbol_size, 'A' + senderId);
  }
  else{
	  // TODO: read from file
	  cerr << "Cannot read input from file for now!" << endl;
	  exit(1);
  }

  // initialize MIMO FUN coder
  MimoFun mimoFun(nType, terminal_num, symbol_size, my_ip, upstream, downstream, verbose);

  // initialize encoder for terminal
  mimoFun.init_terminal_sender(senderId, data_in);

  // initialize sockets
  MimoFunTransmit mimoFunTransmit(my_ip, appPort, tv, &mimoFun);

  bool printed = false;
  // S1
  while(1)
  {
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

	  // output received packets
	  if(!printed && mimoFun.get_decoder_rank() == terminal_num ){
		  	std::cout << "Terminal #" << senderId << " received all packets:" << std::endl;
			std::vector<uint8_t> data_out(mimoFun.get_output_data());
			auto c = data_out.begin();
			for(uint32_t i = 0; i < terminal_num; i++){
			  std::cout << "From #" << i << ": ";
			  for(uint32_t j = 0; j < symbol_size; j++){
				  std::cout << (char)(*(c++));
			  }
			  std::cout << std::endl;
			}
			mimoFun.set_verbose(false);
			printed = true;
	  }
  }


  return 0;
}
