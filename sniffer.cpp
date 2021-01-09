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
#include "include/httplib.h"
#include "include/sql.hpp"
#include "include/http_server.hpp"

using namespace Tins;

/* Define some globals here */ // Bad practice? 
std::unordered_map<std::string, std::string> configs;
std::unordered_map<std::string, int> ip_map;
const char* config = "config.txt";

/* Get start time of program to be used for hash map dumps to SQL DB, periodically per config file */
std::chrono::system_clock::time_point start = std::chrono::system_clock::now();

/* Function to grab config parameters from predefined config file */
void get_config(const char* filename){
    std::ifstream file(filename, std::ifstream::in);
    std::string line;

    /* Open config file, parse it for parameters to use here */
    while (std::getline(file, line)){
        if (*line.begin() == '#' || line.length() == 0){ // If comment or empty line, skip it
            continue;
        }
        else{
            std::istringstream ss(line);
            std::string param, param_val;
            std::getline(ss, param, '=');
            std::getline(ss, param_val, '=');

            configs.insert({param, param_val});
        }
    }
}

/* Define SIGINT behavior here */
void INThandler(int sig){
    /* For now we just want to update the SQL DB upon SIGINT */
    int count = 0;
    std::cout << std::endl;

    /* Run through all of the hash map items, store in DB */
    for (auto vals = ip_map.begin(); vals != ip_map.end(); vals++){
        count = get_count(configs["db"].c_str(), vals->first);

        if (configs["debug"] == "enable"){
            std::cout << vals->first << ": " << count << std::endl;
        }
        
        update_src_count(configs["db"].c_str(), vals->first, count + vals->second);
    }
    exit(0);
}

struct packet_sniffer {
    void run(){
        /* Set up the sniffer config, create sniff_loop object */
        SnifferConfiguration config;
        config.set_promisc_mode(true);
        config.set_immediate_mode(true);

        Sniffer sniffer(configs["iface"].c_str(), config);
        sniffer.sniff_loop(make_sniffer_handler(this, &packet_sniffer::get_ip_packet));
    }

    bool get_ip_packet(PDU &some_pdu){
        /* Get an IP packet, initialize into hash map if it's a new source address */
        const IP &ip = some_pdu.rfind_pdu<IP>();
        std::unordered_map<std::string, int>::iterator search = ip_map.find(ip.src_addr().to_string());
        
        /*  If we run through to the end of the hash map without finding our IP, it doesn't exist yet so add it */
        if (search == ip_map.end()){
            ip_map.insert({ip.src_addr().to_string(), 0});
        }
        else{
            /* If IP already exists in hash map, increment its count by 1 */
            search->second++;
        }
        
        /* Start a timer, when it reaches over wait_time, in seconds, reset timer and dump hash map results to DB */
        auto check = std::chrono::system_clock::now();
        auto diff = std::chrono::duration_cast < std::chrono::seconds > (check - start).count();
        if (diff > std::stoi(configs["wait_time"])){
            int count;
            start = std::chrono::system_clock::now();

            /* Loop through all hash map elements, dump to DB */
            for (auto vals = ip_map.begin(); vals != ip_map.end(); vals++){
                count = get_count(configs["db"].c_str(), vals->first);

                if (configs["debug"] == "enable"){
                    std::cout << vals->first << ": " << count << std::endl;
                }
                
                update_src_count(configs["db"].c_str(), vals->first, count + vals->second);
            }
            ip_map.clear();
        }
        return true;
    }
};

int main() {
    /* Do some initial setup; define SIGINT behavior, parse config file, make sure SQL db is set up */
    signal(SIGINT, INThandler);
    get_config(config);
    create_db(configs["db"].c_str());

    /* Start new thread to run HTTP server here */
    std::thread http_server(run_server, configs["address"].c_str(), std::stoi(configs["port"]), configs["db"].c_str(), configs["debug"]);
    
    /* Create and run sniffer; sniffer.run() blocks so program will only end on SIGINT or some fatal exception/memory leak */
    packet_sniffer sniff;
    sniff.run();
}
