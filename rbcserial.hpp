#ifndef RBC_SERIAL_H
#define RBC_SERIAL_H
#include <string>
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
#define SERVO_CHECK_END	  }
	

#define RBC_MOVE_CHECK_0(idx, idA, idB)		0xFF, 0xE0, 0x64, (0x00 + idx), (0x00 + idx), 0x04, 0xFF, 0x60, idA, idB, 0xFF, 0xC1, 0x10, 0x51, 0xFF, 0xC2, \
						0x10, 0x52, 0xFF, 0xC3, 0x10, 0x53, 0xFF, 0xC4, 0x10, 0x54, 0xFF, 0xC5, 0x10, 0x55, 0xFF, 0xC6, \
						0x10, 0x56, 0xFF, 0xC7, 0x10, 0x57, 0xFF, 0xC8, 0x10, 0x58, 0xFF, 0xC9, 0x10, 0x59, 0xFF, 0xCA, \
						0x10, 0x5A, 0xFF, 0xCB, 0x10, 0x5B, 0xFF, 0xCC, 0x10, 0x5C, 0xFF, 0xCD, 0x10, 0x5D, 0xFF, 0xCE, \
						0x10, 0x5E, 0xFF, 0xCF, 0x10, 0x5F
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
