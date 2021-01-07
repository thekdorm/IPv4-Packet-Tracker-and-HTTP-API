/* TODO Find clean way to define SQL .db file here from sniffer.cpp above
Possible that getting data from DB while main program is dumping will break something? Corner case. */

#include "httplib.h"
#include "../sql.h"
#include <iostream>

int main(){
    httplib::Server svr;

    /* Example Hello World */
    svr.Get("/hi", [](const httplib::Request &, httplib::Response &res){
        res.set_content("Hello World!", "text/plain");
    });

    /* Endpoint to query SQL .db values */
    svr.Get(R"(/api(/ip=([\w,\.]+))?)", [&](const httplib::Request &req, httplib::Response &res){
        if(req.matches[0] == "/api"){
            res.set_content(api_json("../test.db"), "text/plain");
            // api_json("../test.db");
        }
        else{
            res.set_content(api_json("../test.db", req.matches[2]), "text/plain");
            // api_json("../test.db", req.matches[2]);
        }
    });

    svr.listen("192.168.56.101", 8080);
}
