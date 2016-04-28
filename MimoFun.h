/*
 * MimoFun.h
 *
 *  Created on: Apr 17, 2016
 *      Author: mcn
 */

#ifndef MIMOFUN_H_
#define MIMOFUN_H_

#include <vector>
#include <map>
#include <string>
#include <Eigen/Dense>
#include <kodocpp/kodocpp.hpp>


class MimoFun {
public:
	enum pktType
	{
		Data_p = 1,
		Ack_p  = 2,
		Poll_p = 3,
		Qack_p = 4,
		Undef_p = -1
	};
	enum nodeType
	{
		Terminal_n 	= 1,
		Edge_n 		= 2,
		Backbone_n 	= 3,
		Undef_n 	= -1
	};
	MimoFun(nodeType nType,
			uint32_t terminal,
			uint32_t symbol_size,
			std::string my_ip_in,
			std::vector<std::string> upstream_in,
			std::vector<std::string> downstream_in,
			bool verbose);
	pktType recvPacket(uint8_t* data, int bytesRcvd, std::string sourceAddress );
	void recvData(uint8_t* data,int id);
	void recvAck(uint8_t* data, int id);
	void recvPoll();
	void recvQAck();

	std::vector<uint8_t> genPacket(pktType t);
	void init_terminal_sender(uint32_t id, std::vector<uint8_t> in_data );
	bool all_full_rank(std::vector<std::string> nb);
	bool can_contribute_to_all(std::vector<std::string> nb);
	bool can_contribute_to_any(std::vector<std::string> nb);

	// setters and getters
	void 		set_coding_coef(int i, int row, int col, int val){ coding_coef[i](row,col) = val;}
	void		set_verbose(bool v) {verbose = v;}
	std::string get_my_ip() { return my_ip;}
	Eigen::MatrixXd get_coding_coef(int i) {return coding_coef[i];}
	uint32_t 	get_decoder_rank() {return decoder.rank(); }
	bool 		is_verbose() { return verbose;}
	std::vector<uint8_t> get_output_data()	{return data_out;}


private:
	std::vector<uint8_t> matrix2Vector(int id);
	void vector2Matrix(uint8_t* vec, int id);

	nodeType nType;
	bool verbose;
	uint32_t terminal_num;
	uint32_t upstream_num;
	uint32_t downstream_num;
	uint32_t neighbor_num;
	uint32_t max_symbol_size;

	std::string my_ip;
	std::vector<std::string> upstream;
	std::vector<std::string> downstream;
	std::map<std::string, uint32_t> ip2Id;

	std::vector< Eigen::MatrixXd > coding_coef; // 0: self

	kodocpp::decoder_factory decoder_factory;
	kodocpp::decoder decoder;
	std::vector<uint8_t> data_out;

	uint32_t senderId;
	kodocpp::encoder_factory encoder_factory;
	kodocpp::encoder encoder;
	std::vector<uint8_t> data_in;
	std::vector<uint8_t> native_payload;

};

#endif /* MIMOFUN_H_ */
