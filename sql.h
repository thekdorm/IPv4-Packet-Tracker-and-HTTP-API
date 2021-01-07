#include <sqlite3.h>
#include <iostream>
#include <sstream>
#include <unordered_map>

void create_db(const char* db_name){
    sqlite3 *db;
    std::string statement;
    char *err = 0;
    int rc;

    rc = sqlite3_open(db_name, &db);

    if(rc){
        std::cout << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
    }

    statement = "create table if not exists source_ips("  \
        "ip char(15) primary key not null,"               \
        "count int not null);";

    sqlite3_exec(db, "begin transaction;", NULL, 0, &err);

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

int get_count(const char* db_name, const std::string ip){
    sqlite3 *db;
    std::string statement;
    char *err = 0;
    int count;
    int rc;

    rc = sqlite3_open(db_name, &db);

    if(rc){
        std::cout << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
    }

    statement = "select count from source_ips where ip=?;";

    sqlite3_exec(db, "begin transaction;", NULL, 0, &err);

    /* Start compiling our query here */
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, statement.c_str(), -1, &stmt, 0);
    sqlite3_bind_text(stmt, 1, ip.c_str(), ip.length(), SQLITE_STATIC);

    /* Can make this better, add error handling etc but works for now */
    sqlite3_step(stmt);
    count = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);

    sqlite3_exec(db, "end transaction;", NULL, 0, &err);
    sqlite3_close(db);
    return count;
}

void update_src_count(const char* db_name, const std::string ip, int count){
    sqlite3 *db;
    std::string statement;
    char *err = 0;
    int rc;

    rc = sqlite3_open(db_name, &db);

    if(rc){
        std::cout << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
    }

    statement = "insert or replace into source_ips (ip, count) values (?, ?);";

    sqlite3_exec(db, "begin transaction;", NULL, 0, &err);

    /* Start compiling our query here */
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, statement.c_str(), -1, &stmt, 0);
    sqlite3_bind_text(stmt, 1, ip.c_str(), ip.length(), SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, count);

    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    sqlite3_exec(db, "end transaction;", NULL, 0, &err);
    sqlite3_close(db);
}

//std::unordered_map<std::string, std::unordered_map<std::string, int>> api_json(const char* db_name, const std::string ip=""){
std::string api_json(const char* db_name, const std::string ip=""){
    std::unordered_map<std::string, std::unordered_map<std::string, int>> json;
    std::stringstream response;
    sqlite3 *db;
    std::string statement;
    char *err = 0;
    int count = 0;
    int rc;

    rc = sqlite3_open(db_name, &db);

    if(rc){
        std::cout << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
    }

    sqlite3_exec(db, "begin transaction;", NULL, 0, &err);

    /* Prepare query; need a couple cases for if we want a specific IP or all where ip="" */
    sqlite3_stmt *stmt;
    if(ip == ""){
        statement = "select * from source_ips;";
        sqlite3_prepare_v2(db, statement.c_str(), -1, &stmt, 0);
    }
    else{
        statement = "select * from source_ips where ip=?;";
        sqlite3_prepare_v2(db, statement.c_str(), -1, &stmt, 0);
        sqlite3_bind_text(stmt, 1, ip.c_str(), ip.length(), SQLITE_STATIC);
    }

    /* Start building the JSON text */
    while(sqlite3_step(stmt) == SQLITE_ROW){
        std::cout << sqlite3_column_text(stmt, 0) << ": " << sqlite3_column_int(stmt, 1) << std::endl;
        if(count == 0){  /* If first iteration */
            response << "{\"" << sqlite3_column_text(stmt, 0) << "\": {\"COUNT\": " << sqlite3_column_int(stmt, 1) << "}";
        }
        else{
            response << ", \"" << sqlite3_column_text(stmt, 0) << "\": {\"COUNT\": " << sqlite3_column_int(stmt, 1) << "}";
        }
        count++;
    }
    response << "}";

    std::cout << response.str() << std::endl;

    sqlite3_finalize(stmt);
    sqlite3_exec(db, "end transaction;", NULL, 0, &err);
    sqlite3_close(db);

    return response.str();
}
