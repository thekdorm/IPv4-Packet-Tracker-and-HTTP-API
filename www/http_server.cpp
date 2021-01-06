// TODO Find clean way to define SQL .db file here from sniffer.cpp above
// Possible that getting data from DB while main program is dumping will break something? Corner case.

#include "httplib.h"
#include "../sql.h"
#include <iostream>

int main(){
    httplib::Server svr;

    // Example Hello World
    svr.Get("/hi", [](const httplib::Request &, httplib::Response &res){
        res.set_content("Hello World!", "text/plain");
    });

    // Endpoint to query SQL .db values
    // 
    svr.Get(R"(/api(/ip=([\w,\.]+))?)", [&](const httplib::Request &req, httplib::Response &res){
        if(req.matches[0] == "/api"){
            api_json("../test.db");
        }
        else{
            std::cout << get_count("../test.db", req.matches[2]) << std::endl;
        }
        // res.set_content(std::to_string(get_count("../test.db", "192.168.56.1")), "text/plain");
    });

    svr.listen("192.168.56.101", 8080);
}
