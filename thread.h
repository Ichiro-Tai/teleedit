#include <iostream>
#include <fstream>
#include <cstdlib>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string>
#include "thread.h"
#include "include/rapidjson/document.h"
#include "include/rapidjson/writer.h"
#include "include/rapidjson/stringbuffer.h"

#define TIMEOUT 600

string recv(int socket, int bytes);

string read_current_file(string filename);

string append_to_current_file(string filename, string to_append);

string insert_to_current_file(string filename, string to_insert, int pos);
//
// string delete_at_current_file(string filename, int pos, int length) {
//   string file_contents = read_current_file(filename);
//   if (pos > file_contents.length()) {
//     pos = file_contents.length();
//   }
//
//
//   file_contents.erase(pos, length);
//
//   ofstream file;
//   file.open(filename.c_str());
//   file << file_contents.c_str();
//   file.close();
//   return "successfully deleted " + to_insert + " to file: " + filename + " at position: " + std::to_string(pos) + "\n";
// }

void* handleConnection(void* sock);
