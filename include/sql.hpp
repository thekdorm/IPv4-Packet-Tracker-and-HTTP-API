extern std::unordered_map<std::string, std::string> configs;

void create_db(const char* db_name){
    sqlite3 *db;
    std::string statement;
    char *err = 0;
    int rc;

    rc = sqlite3_open(db_name, &db);

    if (rc){
        std::cout << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
    }

    /* Create SOURCE_IPS table */
    statement = "create table if not exists source_ips("  \
        "ip char(15) primary key not null,"               \
        "count int not null);";

    sqlite3_exec(db, "begin transaction;", NULL, 0, &err);

    rc = sqlite3_exec(db, statement.c_str(), NULL, 0, &err);
    if (rc != SQLITE_OK){
        std::cerr << "Error creating table!" << std::endl;
        sqlite3_free(err);
    }

    /* Create DEST_IPS table */
    statement = "create table if not exists dest_ips("  \
        "ip char(15) primary key not null,"               \
        "count int not null);";

    rc = sqlite3_exec(db, statement.c_str(), NULL, 0, &err);
    if (rc != SQLITE_OK){
        std::cerr << "Error creating table!" << std::endl;
        sqlite3_free(err);
    }

    sqlite3_exec(db, "end transaction;", NULL, 0, &err);
    sqlite3_close(db);
    return;
}

/* WIP: Make functions to open/close DB so we don't have to do it in the actual query functions */
// sqlite3* open_db(const char* db_name){
//     sqlite3 *db;
//     char *err;
//     int rc;

//     rc = sqlite3_open(db_name, &db);
//     if(rc){
//         std::cout << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
//     }

//     sqlite3_exec(db, "begin transaction;", NULL, 0, &err);
//     return db;
// }

int get_count(const char* db_name, const std::string ip, const std::string table){
    sqlite3 *db;
    std::string statement;
    char *err = 0;
    int count;
    int rc;

    if (configs["debug"] == "enable"){std::cout << "get_count called with: {" << db_name << ", " << ip << ", " << table << "}" << std::endl;}

    rc = sqlite3_open(db_name, &db);

    if (rc){
        std::cout << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
    }

    statement = "select count from " + table + " where ip=?;";
    sqlite3_exec(db, "begin transaction;", NULL, 0, &err);

    /* Start compiling our query here */
    sqlite3_stmt *stmt;
    rc = sqlite3_prepare_v2(db, statement.c_str(), -1, &stmt, 0);
    if (rc){
        std::cout << "Prepare error: " << sqlite3_errmsg(db) << std::endl;
    }

    rc = sqlite3_bind_text(stmt, 1, ip.c_str(), ip.length(), SQLITE_STATIC);
    if (rc){
        std::cout << "Bind error: " << sqlite3_errmsg(db) << std::endl;
    }

    /* Can make this better, add error handling etc but works for now */
    if (sqlite3_step(stmt) == SQLITE_FAIL){
        std::cout << "Query error: " << sqlite3_errmsg(db) << std::endl;
        return 0;
    }

    count = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);

    sqlite3_exec(db, "end transaction;", NULL, 0, &err);
    sqlite3_close(db);
    return count;
}

void update_src_count(const char* db_name, const std::string ip, const std::string table, int count){
    sqlite3 *db;
    std::string statement;
    char *err = 0;
    int rc;

    rc = sqlite3_open(db_name, &db);

    if (rc){
        std::cout << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
    }

    statement = "insert or replace into " + table + " (ip, count) values (?, ?);";
    sqlite3_exec(db, "begin transaction;", NULL, 0, &err);

    /* Start compiling our query here */
    sqlite3_stmt *stmt;
    rc = sqlite3_prepare_v2(db, statement.c_str(), -1, &stmt, 0);
    if (rc){
        std::cout << "Prepare error: " << sqlite3_errmsg(db) << std::endl;
    }

    rc = sqlite3_bind_text(stmt, 1, ip.c_str(), ip.length(), SQLITE_STATIC);
    if (rc){
        std::cout << "Bind error: " << sqlite3_errmsg(db) << std::endl;
    }

    rc = sqlite3_bind_int(stmt, 2, count);
    if (rc){
        std::cout << "Bind error: " << sqlite3_errmsg(db) << std::endl;
    }

    if (sqlite3_step(stmt) == SQLITE_FAIL){
        std::cout << "Query error: " << sqlite3_errmsg(db) << std::endl;
    }
    sqlite3_finalize(stmt);
    sqlite3_exec(db, "end transaction;", NULL, 0, &err);
    sqlite3_close(db);
}

std::string api_json(const char* db_name, const std::string table, const std::string ip=""){
    std::stringstream response;
    sqlite3 *db;
    std::string statement;
    char *err = 0;
    int count = 0;
    int rc;

    rc = sqlite3_open(db_name, &db);

    if (rc){
        std::cout << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
    }

    sqlite3_exec(db, "begin transaction;", NULL, 0, &err);

    /* Prepare query; need a couple cases for if we want a specific IP or all where ip="" */
    sqlite3_stmt *stmt;
    if (ip == ""){
        statement = "select * from " + table + ";";
        sqlite3_prepare_v2(db, statement.c_str(), -1, &stmt, 0);
    }
    else{
        statement = "select * from " + table + " where ip=?;";
        sqlite3_prepare_v2(db, statement.c_str(), -1, &stmt, 0);
        sqlite3_bind_text(stmt, 1, ip.c_str(), ip.length(), SQLITE_STATIC);
    }

    /* 
     * Start building the JSON text 
     * Should definitely look into a cleaner way of doing this for dynamic JSON building
     * i.e. if SQL db has more than just COUNT we'd like to be able to provide that too
     */
    while (sqlite3_step(stmt) == SQLITE_ROW){
        if (configs["debug"] == "enable"){
            std::cout << sqlite3_column_text(stmt, 0) << ": " << sqlite3_column_int(stmt, 1) << std::endl;
        }

        if (count == 0){  // If first iteration, start with {
            response << "{\"" << sqlite3_column_text(stmt, 0) << "\": {\"COUNT\": " << sqlite3_column_int(stmt, 1) << "}";
        }
        else{  // If not first iteration, start with ,
            response << ", \"" << sqlite3_column_text(stmt, 0) << "\": {\"COUNT\": " << sqlite3_column_int(stmt, 1) << "}";
        }
        count++;
    }

    if (response.str().length() > 0){
        response << "}";
    }
    else{  // Build response string for an empty response/invalid query
        response << "{}";
    }
    
    std::cout << response.str() << std::endl;

    sqlite3_finalize(stmt);
    sqlite3_exec(db, "end transaction;", NULL, 0, &err);
    sqlite3_close(db);

    return response.str();
}
