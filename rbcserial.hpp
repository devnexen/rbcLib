#ifndef RBC_SERIAL_H
#define RBC_SERIAL_H
#include <string>
#include <boost/asio.hpp>
#include <boost/optional.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>

#define RBC_BUFFER_SIZE		128
#define COMMON_HEADER           0xFF, 0xFF, 0xAA, 0x55, \
                                0xAA, 0x55, 0x37, 0xBA, \

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
