

PGconn* dbConnect(char* dbname, char* dbhost, char* dbuser, char* dbpw) {
    char connection_string [512];
    snprintf( connection_string, 512, "dbname=%s host=%s user=%s password=%s", dbname, dbhost, dbuser, dbpw );
    PGconn* conn = PQconnectdb( connection_string );
    if (PQstatus(conn) == CONNECTION_BAD) {
        puts("fail");
        exit(0);
    }
    return conn;
}
