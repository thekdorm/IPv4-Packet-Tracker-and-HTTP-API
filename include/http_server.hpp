void run_server(const char* address, const int port, const char* db, std::string debug){
    httplib::Server svr;

    /* Example Hello World */
    svr.Get("/hi", [](const httplib::Request &, httplib::Response &res){
        res.set_content("Hello World!", "text/plain");
    });

    /* Endpoint to query SQL .db values */
    svr.Get(R"(/api(/ip=([\w,\.]+))?)", [&](const httplib::Request &req, httplib::Response &res){
        if (req.matches[0] == "/api"){
            res.set_content(api_json(db, "", debug), "text/plain");
            // api_json("../test.db");
        }
        else{
            res.set_content(api_json(db, req.matches[2], debug), "text/plain");
            // api_json("../test.db", req.matches[2]);
        }
    });

    svr.listen(address, port);
}
