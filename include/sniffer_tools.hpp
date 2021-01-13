extern std::unordered_map<std::string, std::string> configs;

/* Define the packet sniffer class, behavior here */
class packet_sniffer {
    /* Set up some unordered maps to store packet details we want to track, start a timer for map dumps to SQL db */
    private:
        std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
        std::unordered_map<std::string, int> source_ips;
        std::unordered_map<std::string, int> dest_ips;

    public:
        void run(){
            /* Set up the sniffer config, create sniff_loop object */
            Tins::SnifferConfiguration config;
            config.set_promisc_mode(true);
            config.set_immediate_mode(true);

            Tins::Sniffer sniffer(configs["iface"].c_str(), config);
            sniffer.sniff_loop(Tins::make_sniffer_handler(this, &packet_sniffer::get_ip_packet));
        }

        bool get_ip_packet(Tins::PDU &some_pdu){
            /* Get an IP packet, collect stats and add to maps */
            const Tins::IP &ip = some_pdu.rfind_pdu<Tins::IP>();
            update_map(ip.src_addr().to_string(), source_ips);
            update_map(ip.dst_addr().to_string(), dest_ips);
            
            /* Start a timer, when it reaches over wait_time, in seconds, reset timer and dump hash map results to DB */
            auto check = std::chrono::system_clock::now();
            auto diff = std::chrono::duration_cast < std::chrono::seconds > (check - start).count();
            if (diff > std::stoi(configs["wait_time"])){
                int count;
                start = std::chrono::system_clock::now();
                dump_maps(configs["db"].c_str());
            }
            return true;
        }

        /* Take a map of whatever packet stat to track occurrences */
        void update_map(const std::string addr, std::unordered_map<std::string, int> &map){
            std::unordered_map<std::string, int>::iterator search = map.find(addr);

            /* If the stat doesn't exist yet in map, add it; else increment occurrence counter */
            if (search == map.end()){
                map.insert({addr, 0});
            }
            else{
                search->second++;
            }
        }

        /* Take maps, dump to SQL db */
        void dump_maps(const char* db){
            int count;

            if (configs["debug"] == "enable"){std::cout << "############Source IPs############" << std::endl;}
            for (auto vals = source_ips.begin(); vals != source_ips.end(); vals++){
                    count = get_count(configs["db"].c_str(), vals->first, "source_ips");

                    if (configs["debug"] == "enable"){
                        std::cout << vals->first << ": " << vals->second << " + " << count << std::endl;
                    }
                    
                    update_src_count(configs["db"].c_str(), vals->first, "source_ips", count + vals->second);
            }

            if (configs["debug"] == "enable"){std::cout << "############Dest IPs############" << std::endl;}
            for (auto vals = dest_ips.begin(); vals != dest_ips.end(); vals++){
                    count = get_count(configs["db"].c_str(), vals->first, "dest_ips");

                    if (configs["debug"] == "enable"){
                        std::cout << vals->first << ": " << vals->second << " + " << count << std::endl;
                    }
                    
                    update_src_count(configs["db"].c_str(), vals->first, "dest_ips", count + vals->second);
            }
            source_ips.clear();
            dest_ips.clear();
        }
};

/* Function to grab config parameters from predefined config file */
std::unordered_map<std::string, std::string> get_config(const char* filename){
    std::unordered_map<std::string, std::string> configs;
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
    return configs;
}