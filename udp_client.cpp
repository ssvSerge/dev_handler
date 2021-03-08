#include <iostream>
#include <chrono>
#include <thread>
#include <stdint.h>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/program_options.hpp>

const std::string  OPTION_HELP          = "help";

const std::string  OPTION_MY_NAME       = "name";

const std::string  OPTION_SRV_ADDR      = "addr";
const std::string  DEFAULT_SRV_NAME     = "localhost";

const std::string  OPTION_SRV_PORT      = "port";
const std::string  DEFAULT_SRV_PORT     = "1122";

const std::string  OPTION_SEND_PERIOD   = "period";
const int          DEFAULT_SEND_PREIOD  = 1000;

const std::string  OPTION_SENDS_CNT     = "cnt";
const int          DEFAULT_SENDS_CNT    = 100;


using boost::asio::ip::udp;
namespace po = boost::program_options;

class app_options {
    public:
        std::string   dev_name;
        std::string   ip_addr;
        std::string   port;
        int           delay;
        int           cnt;
};

class udp_client {
    public:
        udp_client( boost::asio::io_service& io_service, const std::string& host, const std::string& port ) : io_service_(io_service), socket_(io_service, udp::endpoint(udp::v4(), 0)) {

            udp::resolver resolver(io_service_);
            udp::resolver::query query(udp::v4(), host, port);
            udp::resolver::iterator iter = resolver.resolve(query);
            endpoint_ = *iter;
        }

        ~udp_client() {
            socket_.close();
        }

        void send(const std::string& msg) {
            socket_.send_to(boost::asio::buffer(msg, msg.size()), endpoint_);
        }

    private:
        boost::asio::io_service& io_service_;
        udp::socket socket_;
        udp::endpoint endpoint_;
};

static app_options g_app_params;

static bool _process_cmd_line ( const int argc, const char * const argv[] ) {

    bool ret_val = false;

    boost::program_options::options_description desc("udp_client");

    desc.add_options()  ( OPTION_HELP.c_str(), "Usage" );

    desc.add_options()  ( OPTION_MY_NAME.c_str(),
                          po::value<std::string>(),
                          "Sensor name. Mandatory.");

    desc.add_options()  ( OPTION_SRV_ADDR.c_str(),
                          po::value<std::string>(&g_app_params.ip_addr)->default_value(DEFAULT_SRV_NAME),
                          "Destination IP address.");

    desc.add_options()  ( OPTION_SRV_PORT.c_str(),
                          po::value<std::string>(&g_app_params.port)->default_value(DEFAULT_SRV_PORT),
                          "Destination port.");

    desc.add_options()  ( OPTION_SEND_PERIOD.c_str(),
                          po::value<int>(&g_app_params.delay)->default_value(DEFAULT_SEND_PREIOD),
                          "Period in milliseconds.");

    desc.add_options()  ( OPTION_SENDS_CNT.c_str(),
                          po::value<int>(&g_app_params.cnt)->default_value(DEFAULT_SENDS_CNT),
                          "Count of packets to send.");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if ( vm.count(OPTION_HELP) > 0 ) {
        // Help required
        ret_val = false;
    } else
    if ( vm.count(OPTION_MY_NAME) == 0 ) {
        // Device name is not provided.
        ret_val = false;
    } else {
        g_app_params.dev_name = vm[OPTION_MY_NAME].as<std::string>();
        ret_val = true;
    }

    if ( ! ret_val ) {
        std::cout << desc << "\n";
    }

    return ret_val;
}

int main ( int argc, char* argv[] ) {

    bool     io_res;
    uint32_t msg_id = 100;

    io_res = _process_cmd_line (argc, argv);
    if ( ! io_res ) {
        return -1;
    }

    std::string out_msg;

    boost::asio::io_service io_service;
    udp_client client(io_service, g_app_params.ip_addr, g_app_params.port);

    for ( int i=0; i<g_app_params.cnt; i++ ) {

        msg_id++;

        out_msg  = g_app_params.dev_name;
        out_msg += "\n";
        out_msg += std::to_string(msg_id);
        out_msg += "\n";

        client.send(out_msg);

        std::cout << "Processed: " << i+1 << " from " << g_app_params.cnt << "      \r" << std::flush;

        std::this_thread::sleep_for( std::chrono::milliseconds(g_app_params.delay) );
    }

    std::cout << "\n";

    return 0;
}
