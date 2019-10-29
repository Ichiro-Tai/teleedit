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
using namespace std;

string recv(int socket, int bytes) {
  string output(bytes, 0);
  read(socket, &output[0], bytes-1);
  return output;
}

string read_current_file(string filename) {
  ifstream file;
  file.open(filename.c_str());
  if (!file) {
    return "cannot open file: " + filename + "\n";
  }

  string to_return;
  string temp;
  while (file) {
    getline(file, temp);
    to_return += temp + "\n";
  }

  if (to_return.length()) {
    to_return.pop_back();
  }

  file.close();

  return to_return;
}

string append_to_current_file(string filename, string to_append) {
  ofstream file;
  file.open(filename.c_str(), std::ios_base::app);
  if (!file) {
    return "cannot open file: " + filename + "\n";
  }

  file << to_append.c_str();
  file.close();
  return "successfully appended " + to_append + " to file: " + filename + "\n";
}

string insert_to_current_file(string filename, string to_insert, int pos) {
  string file_contents = read_current_file(filename);
  if (pos > file_contents.length()) {
    pos = file_contents.length();
  }

  file_contents.insert(pos, to_insert);

  ofstream file;
  file.open(filename.c_str());
  file << file_contents.c_str();
  file.close();
  return "successfully inserted " + to_insert + " to file: " + filename + " at position: " + std::to_string(pos) + "\n";
}
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

void* handleConnection(void* sock) {
  while (true) {
    int timer = time(0);
    int socket = *((int*) sock);
    string msg = recv(socket, 1024);
    //decode to utf8
    rapidjson::Document json_msg;
    json_msg.Parse(msg.c_str());
    rapidjson::Value& type = json_msg["type"];

    if (type.GetString() == "append") {
      rapidjson::Value& to_append = json_msg["data"];
      string feedback = append_to_current_file("test_file", to_append.GetString());
      send(socket, feedback.c_str(), feedback.length(), 0);
    }
    //
    // if (type.GetString() == "disconnect") {
    //   send(socket, "disconnected from host", 22, 0);
    //   return NULL;
    // }
  }
}
//
// int main() {
//   cout << insert_to_current_file("fuck", "ccc", 3);
//   return 0;
// }
