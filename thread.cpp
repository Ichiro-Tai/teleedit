#include <iostream>
#include <fstream>
#include <cstdlib>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string>
#include <dirent.h>
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

// string read_current_file(string filename, size_t size, size_t offset){
//   string allContents = read_current_file(filename);
//   return string.substr(offset, size);
// }

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

string read_file(string filename, int bytes_to_read, int offset) {
  ifstream file;
  file.open(filename.c_str(), ios::binary);
  if (!file) {
    return "cannot open file: " + filename + "\n";
  }

  file.seekg(offset, ios::end);
  std::string data(bytes_to_read, '\0');
  file.read(&data[0], bytes_to_read);
  return data;
}

string write_file(string filename, string data, int offset) {
  fstream file;
  file.open(filename.c_str(), ios::binary);
  if (!file) {
    return "cannot open file: " + filename + "\n";
  }

  file.seekp(offset, ios::beg);
  file.write(data.c_str(), data.length());
  return "successfully written to " + filename + '\n';
}

string read_dir(string dir_name) {
  struct dirent* de;
  DIR* dr = opendir(dir_name.c_str());
  if (!dr) {
    return "cannot open directory: " + dir_name + "\n";
  }

  string directory_contents;
  while ((de == readdir(dr)) != NULL) {
    directory_contents += de->d_name;
    directory_contents += "\n";
  }
  return directory_contents;
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
    int socket = ((int)(long) sock);
    string msg = recv(socket, 1024);
    cout << "Raw msg: " << msg << endl;
    //decode to utf8
    rapidjson::Document json_msg;
    json_msg.Parse(msg.c_str());
    rapidjson::Value& type = json_msg["type"];
    string connection_type = type.GetString();

    if (connection_type.compare("connect") == 0) {
      string greetings = "you are connected";
      send(socket, greetings.c_str(), greetings.length(), 0);
    } else if (connection_type.compare("append") == 0) {
      string to_append = json_msg["data"].GetString();
      string path = json_msg["path"].GetString();
      string feedback = append_to_current_file(path, to_append);
      send(socket, feedback.c_str(), feedback.length(), 0);
    } else if (connection_type.compare("read") == 0){
      string path = json_msg["path"].GetString();
      size_t size = json_msg["size"].GetInt();
      size_t offset = json_msg["offset"].GetInt();
      string feedback = read_file(path, size, offset);
      send(socket, feedback.c_str(), feedback.length(), 0);
    } else if (connection_type.compare("write") == 0){
      string path = json_msg["path"].GetString();
      string data = json_msg["data"].GetString();
      size_t offset = json_msg["offset"].GetInt();
      string feedback = write_file(path, data, offset);
      send(socket, feedback.c_str(), feedback.length(), 0);
    } else if (connection_type.compare("readdir") == 0){
      string path = json_msg["path"].GetString();
      string feedback = read_dir(path);
      send(socket, feedback.c_str(), feedback.length(), 0);
    }
    //
    // if (type.GetString() == "disconnect") {
    //   send(socket, "disconnected from host", 22, 0);
    //   return NULL;
    // }
  }
  return NULL;
}
//
// int main() {
//   cout << insert_to_current_file("fuck", "ccc", 3);
//   return 0;
// }
