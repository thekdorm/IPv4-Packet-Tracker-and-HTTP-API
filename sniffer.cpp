#include <iostream>
#include <chrono>
#include <unordered_map>
#include <signal.h>
#include <tins/tins.h>
#include "sql.h"

// WAIT_TIME time in between hash map dumps into SQL DB, seconds
#define WAIT_TIME 10

using namespace Tins;
std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
std::unordered_map<std::string, int> ip_map;
const char *db = "test.db";


void INThandler(int sig){
    /* For now we just want to update the SQL DB upon SIGINT */
    int count = 0;
    std::cout << std::endl;

    /* Run through all of the hash map items, store in DB */
    for (auto vals = ip_map.begin(); vals != ip_map.end(); vals++){
        count = get_count(db, vals->first);
        std::cout << vals->first << ": " << count << std::endl;
        update_src_count(db, vals->first, count + vals->second);
    }
    exit(0);
}

struct packet_sniffer {
    void run(){
        /* Set up the sniffer config, create sniff_loop object */
        SnifferConfiguration config;
        config.set_promisc_mode(true);
        config.set_immediate_mode(true);

        Sniffer sniffer("enp0s8", config);
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
        
        /* Start a timer, when it reaches over 10 seconds reset timer and dump hash map results to DB */
        auto check = std::chrono::system_clock::now();
        auto diff = std::chrono::duration_cast < std::chrono::seconds > (check - start).count();
        if (diff > WAIT_TIME){
            int count;
            start = std::chrono::system_clock::now();

            /* Loop through all hash map elements, dump to DB */
            for (auto vals = ip_map.begin(); vals != ip_map.end(); vals++){
                count = get_count(db, vals->first);
                std::cout << vals->first << ": " << count << std::endl;
                update_src_count(db, vals->first, count + vals->second);
            }
            ip_map.clear();
        }
        return true;
    }
};

int main() {
    signal(SIGINT, INThandler);
    create_db(db);

    packet_sniffer sniff;
    sniff.run();
}
