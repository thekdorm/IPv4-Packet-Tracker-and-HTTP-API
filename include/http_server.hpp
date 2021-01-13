void run_server(const char* address, const int port, const char* db, std::string debug){
    httplib::Server svr;

    /* Example Hello World */
    svr.Get("/hi", [](const httplib::Request &, httplib::Response &res){
        res.set_content("Hello World!", "text/plain");
    });

    /* 
     * Endpoint to query SQL .db values; worth breaking this up into multiple for readability?
     * 
     * Acceptable queries:
     * /api
     * /api/stat=[stat] where [stat] = {src, dest}
     * /api/stat=[stat]&ip=[ip] where [stat] = {src, dest} and [ip] is a specific IP to query
     */
    svr.Get(R"(/api(/stat=(\w+)&?(ip=([\w,\.]+))?)?)", [&](const httplib::Request &req, httplib::Response &res){
        if (configs["debug"] == "enable"){
            std::cout << req.matches[0] << std::endl;
            std::cout << req.matches[2] << std::endl;
            std::cout << req.matches[4] << std::endl;
        }

        if (req.matches[0] == "/api"){  // Default stat to query if none specified is source_ips
            res.set_content(api_json(db, "source_ips"), "text/plain");
            // api_json("../test.db");
        }
        else{
            std::string stat;
            if (req.matches[2] == "src"){
                stat = "source_ips";
            }
            else{
                stat = "dest_ips";
            }
            res.set_content(api_json(db, stat, req.matches[4]), "text/plain");
            // api_json("../test.db", req.matches[2]);
        }
    });

    svr.listen(address, port);
}
