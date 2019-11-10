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

#include <sys/types.h>
#include <sys/stat.h>

#define TIMEOUT 600
using namespace std;
static string root_dir = "root_dir";

string recv(int socket, int bytes) {
  string output(bytes, 0);
  read(socket, &output[0], bytes-1);
  return output;
}

string read_file(string filename, int bytes_to_read, int offset) {
  ifstream file;
  file.open(filename.c_str(), ios::binary);
  if (!file) {
    return "CANNOT OPEN FILE\n";
  }

  // file.seekg(offset, ios::end);
  std::string data(bytes_to_read, '\0');
  file.read(&data[0], bytes_to_read);
  return data;
}

string write_file(string filename, string data, int offset) {
  ofstream file;
  file.open(filename.c_str(), ios::binary);
  if (!file) {
    return "CANNOT OPEN FILE\n";
  }

  std::cout << std::to_string(offset) << std::endl;

  file.seekp(offset, ios::beg);
  file.write(data.c_str(), data.length());
  return to_string(data.length());
}

string read_dir(string dir_name) {
  struct dirent* de;
  DIR* dr = opendir(dir_name.c_str());
  if (!dr) {
    return "CANNOT OPEN DIR\n";
  }

  string directory_contents;
  while ((de = readdir(dr)) != NULL) {
    directory_contents += de->d_name;
    directory_contents += "\n";
  }
  if (directory_contents.empty()) {
    directory_contents = "EMPTY";
  }
  return directory_contents;
}

string read_stat(string filename) {
  std::cout << filename << "\n";
  struct stat file_stat;
  int status = stat(filename.c_str(), &file_stat);

  if (status == -1) {
    return "CANNOT OPEN FILE\n";
  }

  string res = "";
  res += "st_mode:" + std::to_string(file_stat.st_mode) + ";";
  res += "st_nlink:" + std::to_string(file_stat.st_nlink) + ";";
  res += "st_size:" + std::to_string(file_stat.st_size) + ";";
  res += "st_uid:" + std::to_string(file_stat.st_uid) + ";";
  res += "st_gid:" + std::to_string(file_stat.st_gid) + ";";
  res += "st_atime:" + std::to_string(file_stat.st_atime) + ";";
  res += "st_mtime:" + std::to_string(file_stat.st_mtime) + ";";
  res += "st_ctime:" + std::to_string(file_stat.st_ctime) + ";";

  return res;
}

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
    } else if (connection_type.compare("read") == 0){
      string path = root_dir + json_msg["path"].GetString();
      size_t size = json_msg["size"].GetInt();
      size_t offset = json_msg["offset"].GetInt();
      string feedback = read_file(path, size, offset);
      send(socket, feedback.c_str(), feedback.length(), 0);
    } else if (connection_type.compare("write") == 0){
      string path = root_dir + json_msg["path"].GetString();
      string data = json_msg["data"].GetString();
      size_t offset = json_msg["offset"].GetInt();
      string feedback = write_file(path, data, offset);
      send(socket, feedback.c_str(), feedback.length(), 0);
    } else if (connection_type.compare("readdir") == 0){
      string path = root_dir + json_msg["path"].GetString();
      string feedback = read_dir(path);
      send(socket, feedback.c_str(), feedback.length(), 0);
    } else if (connection_type.compare("disconnect") == 0) {
      send(socket, "disconnected from host", 22, 0);
      return NULL;
    } else if (connection_type.compare("getattr") == 0) {
      string path  = root_dir + json_msg["path"].GetString();
      string feedback = read_stat(path);
      send(socket, feedback.c_str(), feedback.length(), 0);
    }
  }
  return NULL;
}
