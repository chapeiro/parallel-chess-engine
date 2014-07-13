/**
 * socket_utils.hpp
 *
 *  Created on: 2014/06/22
 *      Author: Chrysogelos Periklis
 */

#ifndef SOCKET_UTILS_HPP_
#define SOCKET_UTILS_HPP_


int connect(char * extrn_addr, int port);
void dup2sock(int argc, char* argv[]);
void duplicate2socket(char * extrn_addr, int outport, int inport);
void closeSockets();

#endif /* SOCKET_UTILS_HPP_ */
