/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 David CARLIER
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 *THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
 *
 */
#ifndef RBC_SERIAL_H
#define RBC_SERIAL_H
#include <string>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/optional.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>

#define RBC_BUFFER_SIZE				128
#define COMMON_HEADER           		0xFF, 0xFF, 0xAA, 0x55, \
                                		0xAA, 0x55, 0x37, 0xBA,

#define SERVO_CHECK_BEGIN unsigned char commandServoCheck[] = { \
				0xFF, 0x00, 0x00, 0x00 \
			  }; \
			  \
			  unsigned char i = 0xA0, j = 0x20; \
			  unsigned int ID = 0; \
						\
			  for(; i <= 0xAF && j <= 0x2F; i ++, j ++) { \
				o << "Servo ID " << std::setbase(10) << ID ++ << std::endl; \
				commandServoCheck[1] = i; \
				commandServoCheck[3] = j; \
				unsigned char sresponse[2]; \
				r.async_write(commandServoCheck, 4); \
				r.read(sresponse, 2); \
				\
	  		        if(* (sresponse + 1) == 0x00) { \
					break; \
			  	} \
			  	\
				r.print_bytes(sresponse, 2, o); \
		          	r.flush();

#define SERVO_CHECK_END	 }
	
#define RBC_SERVO_MOVE(ID, speed, pos)	unsigned char commandServoPos[4] = { \
						0xFF, (unsigned char)(speed << 5 | ID), pos, (unsigned char )(((speed << 5 | ID) ^ pos) & 0x7F) \
					};

#define RBC_SERVO_PASSIVE(ID)		unsigned char commandServoPassive[4] = { \
						0xFF, (unsigned char)(0xC0 | ID), 0x10, (unsigned char)(((0xC0 | ID) ^ 0x10) & 0x7F) \
					};

class RbcSerial {
	private:
		std::string tty;
		unsigned int brate;
		char serial_number[14];
		char firmware[6];
		
		boost::shared_ptr<boost::asio::serial_port> serial;
		boost::asio::io_service io;
		boost::mutex mtx;
		
		boost::asio::serial_port_base::parity opt_parity;
		boost::asio::serial_port_base::character_size opt_csize;
		boost::asio::serial_port_base::flow_control opt_flow;
		boost::asio::serial_port_base::stop_bits opt_stop;
		
		void set_serial_number();
		void set_firmware();
	public:
		RbcSerial(const std::string _tty, unsigned int _brate, 
			boost::asio::serial_port_base::parity _opt_parity = boost::asio::serial_port_base::parity(boost::asio::serial_port_base::parity::none),
			boost::asio::serial_port_base::character_size _opt_csize = boost::asio::serial_port_base::character_size(8),
			boost::asio::serial_port_base::flow_control _opt_flow = boost::asio::serial_port_base::flow_control(boost::asio::serial_port_base::flow_control::none),
			boost::asio::serial_port_base::stop_bits _opt_stop = boost::asio::serial_port_base::stop_bits(boost::asio::serial_port_base::stop_bits::one)) :
				tty(_tty), brate(_brate), opt_parity(_opt_parity), opt_csize(_opt_csize), opt_flow(_opt_flow), opt_stop(_opt_stop) { open(); }

		virtual ~RbcSerial();

		void open();
		void close();
		void reset();
		void flush();
		void write(const unsigned char * data, size_t size);
		void write(const unsigned char * data) { write(data, RBC_BUFFER_SIZE); }
		void async_write(const unsigned char * data, size_t size);
		void async_write(const unsigned char * data) { async_write(data, RBC_BUFFER_SIZE); }
		void async_write_handler(const boost::system::error_code & e,size_t b);
		void read(unsigned char * data, size_t size);
		void async_read(unsigned char * data, size_t size);
		void async_read(unsigned char * data) { async_read(data, RBC_BUFFER_SIZE); }
		void async_read_handler(const boost::system::error_code & e, std::size_t b);
		void print_bytes(unsigned char * data, size_t size, std::ostream & o);
		const char * sn() const { return (const char *) serial_number; }
		const char * fw() const { return (const char *) firmware; }
};

#endif
