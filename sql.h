#include <sqlite3.h>
#include <iostream>
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
    // else{  /* For debug */
    //     std::cout << "Database opened successfully!" << std::endl;
    // }

    statement = "create table if not exists source_ips("  \
        "ip char(15) primary key not null,"               \
        "count int not null);";

    sqlite3_exec(db, "begin transaction;", NULL, 0, &err);

    rc = sqlite3_exec(db, statement.c_str(), NULL, 0, &err);
    if (rc != SQLITE_OK){
        std::cerr << "Error creating table!" << std::endl;
        sqlite3_free(err);
    }
    // else{  /* For debug */
    //     std::cout << "Table created successfully!" << std::endl;
    // }

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
    // else{  /* For debug */
    //     std::cout << "Database opened successfully!" << std::endl;
    // }

    // statement = "select exists(select 1 from source_ips where ip=?);";
    statement = "select count from source_ips where ip=?;";

    sqlite3_exec(db, "begin transaction;", NULL, 0, &err);

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
    // else{  /* For debug */
    //     std::cout << "Database opened successfully!" << std::endl;
    // }

    /* First need to check if the IP exists in db */
    statement = "insert or replace into source_ips (ip, count) values (?, ?);";

    sqlite3_exec(db, "begin transaction;", NULL, 0, &err);

    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, statement.c_str(), -1, &stmt, 0);
    sqlite3_bind_text(stmt, 1, ip.c_str(), ip.length(), SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, count);

    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    sqlite3_exec(db, "end transaction;", NULL, 0, &err);
    sqlite3_close(db);
}

std::unordered_map<std::string, std::unordered_map<std::string, int>> api_json(const char* db_name const std::string ip=""){
    
}
