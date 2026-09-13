// Stub functions for MySQL linkage
// This is a minimal stub to allow compilation without MySQL client library

#include <windows.h>
#include <stdio.h>

// Stub implementation for MySQL functions
// These return dummy values to allow compilation
// Using __stdcall to match MySQL calling convention

extern "C" {

typedef void* MYSQL;
typedef void* MYSQL_RES;
typedef char** MYSQL_ROW;

__declspec(dllexport) MYSQL* __stdcall mysql_init(MYSQL* mysql) {
    static char dummy[16] = {0};
    return (MYSQL*)dummy;
}

__declspec(dllexport) int __stdcall mysql_options(MYSQL* mysql, int option, const void* arg) {
    return 0;
}

__declspec(dllexport) int __stdcall mysql_real_connect(MYSQL* mysql, const char* host, const char* user, const char* passwd, const char* db, unsigned int port, const char* unix_socket, unsigned long client_flag) {
    return 1; // Return success
}

__declspec(dllexport) const char* __stdcall mysql_error(MYSQL* mysql) {
    static char err[] = "Stub error";
    return err;
}

__declspec(dllexport) int __stdcall mysql_query(MYSQL* mysql, const char* query) {
    return 0; // Return success
}

__declspec(dllexport) MYSQL_RES* __stdcall mysql_store_result(MYSQL* mysql) {
    static char dummy[16] = {0};
    return (MYSQL_RES*)dummy;
}

__declspec(dllexport) void __stdcall mysql_close(MYSQL* mysql) {
    // Stub
}

__declspec(dllexport) unsigned long __stdcall mysql_num_rows(MYSQL_RES* result) {
    return 0;
}

__declspec(dllexport) MYSQL_ROW __stdcall mysql_fetch_row(MYSQL_RES* result) {
    return NULL;
}

__declspec(dllexport) void __stdcall mysql_free_result(MYSQL_RES* result) {
    // Stub
}

__declspec(dllexport) unsigned long __stdcall mysql_escape_string(char* to, const char* from, unsigned long length) {
    return 0;
}

} // extern "C"
