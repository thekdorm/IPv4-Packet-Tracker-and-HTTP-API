/* TODO Find clean way to define SQL .db file here from sniffer.cpp above
Possible that getting data from DB while main program is dumping will break something? Corner case. */

#include "httplib.h"
#include "../sql.h"
#include <iostream>

std::unordered_map<std::string, std::string> configs;
const char* config = "../config.txt";

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

int main(){
    get_config(config);
    httplib::Server svr;

    std::string db_path_mod = "../";
    db_path_mod.append(configs["db"]);

    /* Example Hello World */
    svr.Get("/hi", [](const httplib::Request &, httplib::Response &res){
        res.set_content("Hello World!", "text/plain");
    });

    /* Endpoint to query SQL .db values */
    svr.Get(R"(/api(/ip=([\w,\.]+))?)", [&](const httplib::Request &req, httplib::Response &res){
        if (req.matches[0] == "/api"){
            res.set_content(api_json(db_path_mod.c_str(), "", configs["debug"]), "text/plain");
            // api_json("../test.db");
        }
        else{
            res.set_content(api_json(db_path_mod.c_str(), req.matches[2], configs["debug"]), "text/plain");
            // api_json("../test.db", req.matches[2]);
        }
    });

    svr.listen(configs["address"].c_str(), std::stoi(configs["port"]));
}
