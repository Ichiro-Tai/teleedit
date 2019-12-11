#pragma once
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string>
#include <dirent.h>
#include <queue>
#include <unordered_map>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/epoll.h>
#include <direct.h>
#include <utility>

#include <mutex>              // std::mutex, std::unique_lock
#include <condition_variable> // std::condition_variable


#include "queue.cpp"

#define TIMEOUT 600
using namespace std;
static const char* root_dir = "root_dir";

std::mutex file_segment_mutex;
std::condition_variable file_segment_cv;
static unordered_map<string, unordered_map<int, int>> file_segment_map;
const int BYTES_PER_SEGMENT = 4096;

std::mutex file_usage_mutex;
std::condition_variable file_usage_cv;
static unordered_map<string, int> file_usage_map;

struct thread_starter_kit {
  Queue* taskQueue;
  int epoll_fd;
};

string recv(int socket, int bytes) {
  string output(bytes, 0);
  read(socket, &output[0], bytes);
  return output;
}

//map helper functions
void file_usage_map_start_read(string filename) {
  std::unique_lock<std::mutex> lk(file_usage_mutex);
  while (file_usage_map.find(filename) != file_usage_map.end() && file_usage_map[filename] == -1) {
    file_usage_cv.wait(lk);
  }
  file_usage_map[filename]++;
  file_usage_mutex.unlock();
}

void file_usage_map_finish_read(string filename) {
  std::unique_lock<std::mutex> lk(file_usage_mutex);
  file_usage_map[filename]--;
  if (file_usage_map[filename] == 0) {
    file_usage_map.erase(filename);
  }
  file_usage_mutex.unlock();
  file_usage_cv.notify_all();
}

void file_usage_map_start_write(string filename) {
  std::unique_lock<std::mutex> lk(file_usage_mutex);
  while (file_usage_map.find(filename) != file_usage_map.end()) {
    file_usage_cv.wait(lk);
  }
  file_usage_map[filename] = -1;
  file_usage_mutex.unlock();
}

void file_usage_map_finish_write(string filename) {
  std::unique_lock<std::mutex> lk(file_usage_mutex);
  file_usage_map.erase(filename);
  file_usage_mutex.unlock();
  file_usage_cv.notify_all();
}

void file_segment_map_start_read(string filename, int segment) {
  std::unique_lock<std::mutex> lk(file_segment_mutex);
  while (file_segment_map.find(filename) != file_segment_map.end() &&
    file_segment_map[filename].find(segment) != file_segment_map[filename].end() &&
    file_segment_map[filename][segment] == -1) {
    file_segment_cv.wait(lk);
  }
  file_segment_map[filename][segment]++;
  file_segment_mutex.unlock();
}

void file_segment_map_finish_read(string filename, int segment) {
  std::unique_lock<std::mutex> lk(file_segment_mutex);
  file_segment_map[filename][segment]--;
  if (file_segment_map[filename][segment] == 0) {
    file_segment_map[filename].erase(segment);
  }
  if (file_segment_map[filename].size() == 0) {
    file_segment_map.erase(filename);
  }
  file_segment_mutex.unlock();
  file_segment_cv.notify_all();
}

void file_segment_map_start_write(string filename, int segment) {
  std::unique_lock<std::mutex> lk(file_segment_mutex);
  while (file_segment_map.find(filename) != file_segment_map.end() &&
    file_segment_map[filename].find(segment) != file_segment_map[filename].end()) {
    file_segment_cv.wait(lk);
  }
  file_segment_map[filename][segment] = -1;
  file_segment_mutex.unlock();
}

void file_segment_map_finish_write(string filename, int segment) {
  std::unique_lock<std::mutex> lk(file_segment_mutex);
  file_segment_map[filename].erase(segment);
  if (file_segment_map[filename].size() == 0) {
    file_segment_map.erase(filename);
  }
  file_segment_mutex.unlock();
  file_segment_cv.notify_all();
}
//////////////////////////////////////////////////////////////////

void read_file(string filename, int bytes_to_read, int offset, int client_fd) {
  file_usage_map_start_read(filename);

  ifstream file;
  file.open(filename.c_str(), ios::binary);
  if (!file) {
    file_usage_map_finish_read(filename);
    send(client_fd, "CANNOT OPEN FILE\n", strlen("CANNOT OPEN FILE\n"), 0);
  }

  char* buffer = new char[BYTES_PER_SEGMENT];
  while (bytes_to_read > 0) {
    cout << "bytes to read: " << std::to_string(bytes_to_read) << "\n";
    file_segment_map_start_read(filename, offset / BYTES_PER_SEGMENT);
    file.seekg(offset, ios::beg);
    int bytes_to_read_current_itr = bytes_to_read < (BYTES_PER_SEGMENT - offset % BYTES_PER_SEGMENT) ? bytes_to_read : (BYTES_PER_SEGMENT - offset % BYTES_PER_SEGMENT);
    file.read(buffer, bytes_to_read_current_itr);
    cout << "data: " << buffer << "\n";
    send(client_fd, buffer, bytes_to_read_current_itr, 0);
    file_segment_map_finish_read(filename, offset / BYTES_PER_SEGMENT);
    offset += bytes_to_read_current_itr;
    bytes_to_read -= bytes_to_read_current_itr;
  }
  delete[] buffer;

  file.close();

  file_usage_map_finish_read(filename);
}

string write_file(string filename, int offset, int client_fd) {
  size_t bytes_to_write = stoul(recv(client_fd, 16));
  std::cout << "bytes to write: " << std::to_string(bytes_to_write) << endl;
  file_usage_map_start_read(filename);

  ofstream file;
  file.open(filename.c_str(), ios::binary);
  if (!file) {
    file_usage_map_finish_read(filename);
    return "-1\n";
  }

  std::cout << std::to_string(offset) << std::endl;

  while (bytes_to_write > 0) {
    file_segment_map_start_write(filename, offset / BYTES_PER_SEGMENT);
    file.seekp(offset, ios::beg);
    int bytes_to_write_current_itr = bytes_to_write < (BYTES_PER_SEGMENT - offset % BYTES_PER_SEGMENT) ? bytes_to_write : (BYTES_PER_SEGMENT - offset % BYTES_PER_SEGMENT);
    cout << "bytes to write: " << std::to_string(bytes_to_write_current_itr) << endl;
    std::string data = recv(client_fd, bytes_to_write_current_itr);
    cout << "data: " << data << endl;
    file.write(data.c_str(), data.length());
    file_segment_map_finish_write(filename, offset / BYTES_PER_SEGMENT);
    offset += bytes_to_write_current_itr;
    bytes_to_write -= bytes_to_write_current_itr;
  }

  file.close();

  file_usage_map_finish_read(filename);
  return std::to_string(offset) + "\n";
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
  file_usage_map_start_read(filename);
  struct stat file_stat;

  int status = stat(filename.c_str(), &file_stat);
  cout << "status: " << std::to_string(status) << endl;

  if (status == -1) {
    file_usage_map_finish_read(filename);
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

  file_usage_map_finish_read(filename);
  return res;
}

void* handleConnection(void* kit) {
  thread_starter_kit* items = (thread_starter_kit*) kit;
  Queue* taskQueue = (Queue*) (items->taskQueue);
  int epoll_fd = (int) (items->epoll_fd);
  while (true) {
    Task* next_task = taskQueue->pop();
    int socket = next_task->sock;

    std::string connection_type = recv(socket, 8);
    cout << "Connection type: " << connection_type << endl;

    if (connection_type.compare("connect ") == 0) {
      string greetings = "you are connected";
      send(socket, greetings.c_str(), greetings.length(), 0);

    } else if (connection_type.compare("read    ") == 0){
      string aaa = recv(socket, 16);
      cout << "raw size: " << aaa << "\n";
      size_t size = stoul(aaa);
      cout << "received read size: " << size << "\n";
      size_t offset = stoul(recv(socket, 16));
      std::string path = root_dir + recv(socket, stoul(recv(socket, 16)));

      read_file(path, size, offset, socket);

    } else if (connection_type.compare("write   ") == 0){
      size_t offset = stoul(recv(socket, 16));
      std::string path = root_dir + recv(socket, stoul(recv(socket, 16)));

      string feedback = write_file(path, offset, socket);
      send(socket, feedback.c_str(), feedback.length(), 0);

    } else if (connection_type.compare("readdir ") == 0){
      std::string path = root_dir + recv(socket, stoul(recv(socket, 16)));

      string feedback = read_dir(path);
      send(socket, feedback.c_str(), feedback.length(), 0);

    } else if (connection_type.compare("dconnect") == 0) {
      send(socket, "disconnected from host", 22, 0);
      return NULL;

    } else if (connection_type.compare("getattr ") == 0) {
      std::string path = root_dir + recv(socket, stoul(recv(socket, 16)));
      cout << "path: " << path << endl;

      string feedback = read_stat(path);
      send(socket, feedback.c_str(), feedback.length(), 0);

    } else if (connection_type.compare("truncate") == 0) {
      size_t length = stoul(recv(socket, 16));
      std::string path = root_dir + recv(socket, stoul(recv(socket, 16)));

      file_usage_map_start_write(path);
      string feedback = to_string(truncate(path.c_str(), length));
      file_usage_map_finish_write(path);

    } else if (connection_type.compare("chmod   ") == 0) {
      int mode = stoi(recv(socket, 16));
      std::string path = root_dir + recv(socket, stoul(recv(socket, 16)));

      file_usage_map_start_write(path);
      string feedback = to_string(chmod(path.c_str(), mode));
      cout << "status: " << feedback << endl;
      file_usage_map_finish_write(path);

    } else if (connection_type.compare("chown   ") == 0) {
      uid_t uid = stoul(recv(socket, 16));
      uid_t gid = stoul(recv(socket, 16));
      std::string path = root_dir + recv(socket, stoul(recv(socket, 16)));

      file_usage_map_start_write(path);
      string feedback = to_string(chown(path.c_str(), uid, gid));
      file_usage_map_finish_write(path);
      send(socket, feedback.c_str(), feedback.length(), 0);

    } else if (connection_type.compare("create  ") == 0) {
      int mode = stoi(recv(socket, 16));
      std::string path = root_dir + recv(socket, stoul(recv(socket, 16)));

      file_usage_map_start_write(path);
      ofstream output(path);
      cout << "creating file: " << path << endl;
      string feedback = to_string(chmod(path.c_str(), mode));
      file_usage_map_finish_write(path);
    } else if (connection_type.compare("mkdir   ") == 0) {
      int mode = stoi(recv(socket, 16));
      std::string path = root_dir + recv(socket, stoul(recv(socket, 16)));

      mkdir(path.c_str());
      cout << "creating file: " << path << endl;
      string feedback = to_string(chmod(path.c_str(), mode));
    } else if (connection_type.compare("delete  ") == 0) {
      std::string path = root_dir + recv(socket, stoul(recv(socket, 16)));

      file_usage_map_start_write(path);
      cout << "deleting file: " << path << endl;
      string feedback = to_string(remove(path.c_str()));
      file_usage_map_finish_write(path);
    }

    //add to epoll
    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = socket;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket, &ev);
  }
  return NULL;
}
