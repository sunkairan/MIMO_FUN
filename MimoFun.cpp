/*
 * MimoFun.cpp
 *
 *  Created on: Apr 17, 2016
 *      Author: mcn
 */

#include "MimoFun.h"
#include <vector>
#include <iostream>
#include <map>
#include <cassert>
#include <string>
#include <Eigen/Dense>
#include <kodocpp/kodocpp.hpp>

MimoFun::MimoFun(nodeType nType,
		uint32_t terminal,
		uint32_t symbol_size,
		std::string my_ip_in,
		std::vector<std::string> upstream_in,
		std::vector<std::string> downstream_in,
		bool verbose):
			nType(nType),
			verbose(verbose),
			terminal_num(terminal),
			upstream_num(upstream_in.size()),
			downstream_num(downstream_in.size()),
			neighbor_num(0),
			max_symbol_size(symbol_size),
			my_ip(my_ip_in),
			upstream(upstream_in),
			downstream(downstream_in),
			coding_coef(upstream_num + downstream_num + 1,Eigen::MatrixXd(terminal,terminal)),
			decoder_factory(
						kodocpp::codec::full_vector,
						kodocpp::field::binary8,
						terminal,
						symbol_size),
			encoder_factory(
						kodocpp::codec::full_vector,
						kodocpp::field::binary8,
						terminal,
						symbol_size),
			senderId(0)

{
	// initialize decoder's buffer
	decoder = decoder_factory.build();
	data_out = std::vector<uint8_t>(decoder.block_size());
	decoder.set_mutable_symbols(data_out.data(), decoder.block_size());

	// assign ids to neighbors: 0: self, 1~upstream_n: upstream node neighbors, rest: downstream neighbors
	ip2Id[my_ip] = 0;
	for(auto nb: upstream) ip2Id[nb] = ++neighbor_num;
	for(auto nb: downstream) ip2Id[nb] = ++neighbor_num;

	// initialize neighbor's decoding coef matrix
	for(uint32_t i = 0; i <= neighbor_num; i++){
		coding_coef[i].fill(0);
	}

	// generate the coef matrix getter
	auto callback = [this](const std::string& zone, const std::string& msg)
	{
		// record coding coefficient matrix
		if (zone != std::string("decoder_state"))
			return;

		std::istringstream message_stream(msg);
		std::string line;
		uint32_t row = 0;

		while (std::getline(message_stream, line))
		{
			if (line.empty())
				continue;

			std::istringstream line_stream(line);
			std::string data;
			uint32_t count = 0;
			uint32_t col = 0;
			while (std::getline(line_stream, data, ' '))
			{
				count++;
				if (count < 4)
					continue;
				this->set_coding_coef(0,row,col++,stoi(data)); // put new coef in buffer
			}
			row ++ ;
		}
		if(this->is_verbose()){
			std::cout << "Node " << this->get_my_ip() << " " << zone << ":" << std::endl;
			std::cout << this->get_coding_coef(0) << std::endl;
		}
	};
	decoder.set_trace_callback(callback);

}

MimoFun::pktType MimoFun::recvPacket(uint8_t* data, int bytesRcvd, std::string sourceAddress ){
	assert(bytesRcvd > 2);
	std::string t = {(char)(data[0]), (char)(data[1]), (char)(data[2])};
	assert(sourceAddress != my_ip && ip2Id.count(sourceAddress)>0);
	int id = ip2Id[sourceAddress];
	pktType ret;
	if(t == "DAT") {
		recvData(data+3,id);
		ret = Data_p;
	}
	else if(t == "ACK") {
		recvAck(data+3,id);
		ret = Ack_p;
	}
	else if(t == "POL") {
		recvPoll();
		ret = Poll_p;
	}
	else if(t == "QAC") {
		recvQAck();
		ret = Qack_p;
	}
	else {
		ret = Undef_p;
	}
	if(ret == Undef_p){
		std::cerr << "Unidentified packet type received!" << std::endl;
		exit(1);
	}
	return ret;
}

void MimoFun::recvData(uint8_t* data,int id){
	decoder.read_payload(data);
}

void MimoFun::recvAck(uint8_t* data, int id){
	vector2Matrix(data, id);
}

void MimoFun::recvPoll(){

}

void MimoFun::recvQAck(){

}

std::vector<uint8_t> MimoFun::genPacket(pktType t){
	std::vector<uint8_t> packet;
	std::vector<uint8_t> payload;
	switch(t) {
	case Data_p: // DAT
		packet = std::vector<uint8_t>({'D','A','T'});
		if(nType != Terminal_n){
			// For non-terminal, send a coded packet
			payload = std::vector<uint8_t>(decoder.payload_size());
			decoder.write_payload(payload.data());
		}
		else {
			// For terminal, send native packet
			payload = native_payload;
		}
		packet.insert(packet.end(),payload.begin(),payload.end());
		break;
	case Ack_p:  // ACK
		packet = std::vector<uint8_t>({'A','C','K'});
		payload = std::vector<uint8_t>(matrix2Vector(0));
		packet.insert(packet.end(),payload.begin(),payload.end());
		break;
	case Poll_p: // POLL
		packet = std::vector<uint8_t>({'P','O','L'});
		break;
	case Qack_p: // QACK
		packet = std::vector<uint8_t>({'Q','A','C'});
		break;
	default:
		std::cerr << "Unidentified packet type sending!" << std::endl;
		exit(1);
	}
	return packet;
}

void MimoFun::init_terminal_sender(uint32_t id, std::vector<uint8_t> in_data ){
	assert(nType == Terminal_n);
	encoder = encoder_factory.build();
	senderId = id;
	assert(senderId < terminal_num);
	assert(in_data.size() == encoder.symbol_size());
	assert(terminal_num*max_symbol_size == encoder.block_size());
	// set input vector to all 0, but only the corresponding row to be input data
	data_in = std::vector<uint8_t>(encoder.block_size() - max_symbol_size,0);
	data_in.insert(data_in.begin()+senderId*max_symbol_size, in_data.begin(), in_data.end());
	assert(data_in.size() == encoder.block_size());

	encoder.set_const_symbols(data_in.data(), encoder.block_size());
	encoder.set_systematic_on();
	// locate the senderId's native packet
	std::vector<uint8_t> payload(encoder.payload_size());
	for(uint32_t i = 0; i <= senderId; i++){
		// Encode a (systematic) packet into the payload buffer
		//uint32_t bytes_used =
		encoder.write_payload(payload.data());
	}
	native_payload = payload;
	// feed native packet directly to decoder
	decoder.read_payload(payload.data());
}

bool MimoFun::all_full_rank(std::vector<std::string> nb){
	bool result = true;
	int i;
	for(std::string t: nb){
		i = ip2Id[t];
		Eigen::FullPivLU<Eigen::MatrixXd> lu_decomp(coding_coef[i]);
		uint32_t rank = lu_decomp.rank();
		if(verbose) std::cout<<i<<":"<<rank<<",";
		if(rank < terminal_num){
			result = false;
		}
	}
	if(verbose) std::cout<<std::endl;
	return result;
}

bool MimoFun::can_contribute_to_all(std::vector<std::string> nb){
	Eigen::MatrixXd temp(2*terminal_num,terminal_num);
	for(std::string st: nb){
		uint32_t dest = ip2Id[st];
		Eigen::FullPivLU<Eigen::MatrixXd> lu_decomp_dest(coding_coef[dest]);
		uint32_t dst_rank = lu_decomp_dest.rank();

		temp << coding_coef[dest], coding_coef[0];
		Eigen::FullPivLU<Eigen::MatrixXd> lu_decomp_comb(temp);
		uint32_t com_rank = lu_decomp_comb.rank();
		if(com_rank == dst_rank) return false;
	}
	return true;
}

bool MimoFun::can_contribute_to_any(std::vector<std::string> nb){
	Eigen::MatrixXd temp(2*terminal_num,terminal_num);
	for(std::string st: nb){
		uint32_t dest = ip2Id[st];
		Eigen::FullPivLU<Eigen::MatrixXd> lu_decomp_dest(coding_coef[dest]);
		uint32_t dst_rank = lu_decomp_dest.rank();

		temp << coding_coef[dest], coding_coef[0];
		Eigen::FullPivLU<Eigen::MatrixXd> lu_decomp_comb(temp);
		uint32_t com_rank = lu_decomp_comb.rank();
		if(com_rank > dst_rank) return true;
	}
	return false;
}

// private functions below

std::vector<uint8_t> MimoFun::matrix2Vector(int id){
	Eigen::MatrixXd mat = coding_coef[id];
	std::vector<uint8_t> result;
	result.reserve(terminal_num*terminal_num);
	for(uint32_t i = 0; i < terminal_num; i++){
		for(uint32_t j = 0; j < terminal_num; j++){
			result.push_back((uint8_t)(mat(i,j)));
		}
	}
	return result;
}
void MimoFun::vector2Matrix(uint8_t* vec, int id){
	Eigen::MatrixXd &mat = coding_coef[id];
	int k = 0;
	for(uint32_t i = 0; i < terminal_num; i++){
		for(uint32_t j = 0; j < terminal_num; j++){
			mat(i,j) = vec[k++];
		}
	}
}
