/*
 * tcpHander.h
 *
 *  Created on: 4 Jul 2012
 *      Author: thomas
 */

#ifndef TCPHANDER_H_
#define TCPHANDER_H_

#include <string>
#include <sstream>
#include <iostream>

#define ASIO_STANDALONE
//#include <boost/asio.hpp>
#include <asio.hpp>

class TcpHandler
{
    public:
        TcpHandler(int senderPort)
            : acceptor(io_service, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), senderPort))
          //: acceptor(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), senderPort))
        {

        }

        void sendData(const unsigned char * data, const size_t size)
        {
          //boost::asio::ip::tcp::socket socket(io_service);
            asio::ip::tcp::socket socket(io_service);
            acceptor.accept(socket);

            //boost::system::error_code ignored_error;
            std::error_code ignored_error;
            //boost::asio::write(socket, boost::asio::buffer(data, size), boost::asio::transfer_all(), ignored_error);

            asio::write(socket, asio::buffer(&size, sizeof(size)),
                        asio::transfer_all(), ignored_error);
            asio::write(socket, asio::buffer(data, size), asio::transfer_all(), ignored_error);
        }

    private:
        //boost::asio::io_service io_service;
        asio::io_service io_service;
        //boost::asio::ip::tcp::acceptor acceptor;
        asio::ip::tcp::acceptor acceptor;
};

#endif /* TCPHANDER_H_ */
