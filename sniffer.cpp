/* 
 * TODO 
 * Add more stat parsing/pushing to SQL db
 * Look into packet parsing directly rather than using tins library for faster/easier compilation
 */

#include <iostream>
#include <sstream>
#include <fstream>
#include <chrono>
#include <unordered_map>
#include <thread>
#include <sqlite3.h>
#include <signal.h>
#include <tins/tins.h>
#include "include/sql.hpp"
#include "include/httplib.h"
#include "include/http_server.hpp"
#include "include/sniffer_tools.hpp"

/* Define some globals here */
std::unordered_map<std::string, std::string> configs;
packet_sniffer sniff;  // Make this global so we can interact with it in INThandler signal handler

/* Define SIGINT behavior here */
void INThandler(int sig){
    std::cout << std::endl;

    /* Dump packet stats to SQL db before exiting */
    sniff.dump_maps(configs["db"].c_str());
    exit(0);
}

int main() {
    /* Do some initial setup; define SIGINT behavior, parse config file, make sure SQL db is set up */
    configs = get_config("config.txt");
    signal(SIGINT, INThandler);
    create_db(configs["db"].c_str());

    /* Start new thread to run HTTP server here */
    std::thread http_server(run_server, configs["address"].c_str(), std::stoi(configs["port"]), configs["db"].c_str(), configs["debug"]);
    
    /* Run sniffer; sniffer.run() blocks so program will only end on SIGINT or some fatal exception/memory leak */
    sniff.run();
}
