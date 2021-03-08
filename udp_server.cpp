// #include <stdio.h>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <chrono>
// #include <string.h>
// #include <errno.h>
// #include <string>
// #include <vector>
// #include <sstream>
// #include <map>
// #include <algorithm>

#include <boost/program_options.hpp>

namespace po = boost::program_options;

const int          MAX_MSG_LEN          = 512;

const std::string  OPTION_HELP          = "help";

const std::string  OPTION_SRV_PORT      = "port";
const int          DEFAULT_SRV_PORT     =  1122;

const std::string  OPTION_ACTIVE_TIME   = "time";
const int          DEFAILT_ACTIVE_TIME  = 120;

const std::string  OPTION_SORT_CMD      = "sort";
const bool         DEFAULT_SORT         = false;

typedef std::map<std::string, int>      msg_map_t;
typedef msg_map_t::iterator             msg_map_it_t;
typedef std::vector<std::string>        msg_list_t;

class app_options {
    public:
        int   port;
        int   active;
        bool  sort;
};

static app_options     g_app_params;
static msg_map_t       g_msg_map;

static bool _process_cmd_line ( const int argc, const char * const argv[] ) {

    bool ret_val = true;

    boost::program_options::options_description desc("udp_server");

    desc.add_options()  ( OPTION_HELP.c_str(), "Usage" );

    desc.add_options()  ( OPTION_SRV_PORT.c_str(),
                          po::value<int>(&g_app_params.port)->default_value(DEFAULT_SRV_PORT),
                          "Server port.");

    desc.add_options()  ( OPTION_ACTIVE_TIME.c_str(),
                          po::value<int>(&g_app_params.active)->default_value(DEFAILT_ACTIVE_TIME),
                          "Monitoring time in seconds.");

    desc.add_options()  ( OPTION_SORT_CMD.c_str(),
                          po::value<bool>(&g_app_params.sort)->default_value(DEFAULT_SORT),
                          "Sort before print.");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if ( vm.count(OPTION_HELP) != 0 ) {
        ret_val = false;
    }

    if ( ! ret_val ) {
        std::cout << desc << "\n";
    }

    return ret_val;
}

static bool _read_command ( int srv_sock, std::string& name, std::string& value ) {

    bool ret_val = false;
    char buf[MAX_MSG_LEN];

    int cnt = recvfrom( srv_sock, &buf, sizeof(buf), MSG_WAITALL, nullptr, nullptr);

    if ( cnt <= 0 ) {
        ret_val = true;
    }

    std::stringstream ss;

    ss << buf;
    ss >> name;
    ss >> value;

    ret_val = true;

    return ret_val;
}

static void _process_command ( std::string& name, std::string& value ) {

    msg_map_it_t it;

    it = g_msg_map.find(name);

    if ( it == g_msg_map.end() ) {
        g_msg_map[name] = 1;
    } else {
        it->second++;
    }

    msg_list_t out_list;
    int cnt = 0;

    for (const auto& it : g_msg_map) {

        std::string val = it.first;
        val += ": ";
        val += std::to_string(it.second);

        out_list.emplace_back(val);

        cnt += it.second;
    }

    if ( g_app_params.sort ) {
        std::sort ( out_list.begin(), out_list.end() );
    }

    for (const auto& it : out_list) {
        std::cout << it << std::endl;
    }
    std::cout << "total: " << cnt << std::endl;
    std::cout << std::endl;
    std::cout << std::endl;

    return;
}

int main ( int argc, char* argv[] ) {

    bool io_res;
    io_res = _process_cmd_line (argc, argv);

    if ( ! io_res ) {
        return -1;
    }

    int srv_sock = socket(AF_INET, SOCK_DGRAM, 0);

    if ( srv_sock < 0 ) {
        perror("Socket creation failed.");
        return -2;
    }

    struct timeval tv;
    tv.tv_sec  = 30;
    tv.tv_usec = 0;

    if (setsockopt(srv_sock, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0) {
        perror("Cannot modify timeout on socket,");
        return -3;
    }

    struct sockaddr_in servaddr = {0};
    servaddr.sin_family       = AF_INET;
    servaddr.sin_addr.s_addr  = INADDR_ANY;
    servaddr.sin_port         = htons(g_app_params.port);

    if ( bind(srv_sock, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0 ) {
        perror("Cannot bind socket on port.");
        return -4;
    }

    auto  start = std::chrono::high_resolution_clock::now();
    auto  now   = std::chrono::high_resolution_clock::now();
    int   diff;

    std::string   name;
    std::string   value;

    for (;;) {

        now  = std::chrono::high_resolution_clock::now();

        diff = std::chrono::duration_cast<std::chrono::seconds>(now - start).count();
        if ( diff >= g_app_params.active ) {
            break;
        }

        io_res = _read_command ( srv_sock, name, value );

        if ( io_res ) {
            _process_command (name, value);
        }
    }

    std::cout << "Server terminmated." << std::endl;
    return 0;
}
