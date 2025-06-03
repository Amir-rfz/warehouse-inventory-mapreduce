#ifndef MANUAL_HPP
#define MANUAL_HPP

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstdlib>
#include <cstring>
#include <map>
#include <algorithm>
#include <utility>
#include <memory>
#include <errno.h>
#include <csignal>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <chrono>
#include <thread>
#include <sys/wait.h>
#include <bits/stdc++.h>
#include <dirent.h>
#include <cerrno>


#define WRITE 1
#define READ 0

constexpr char STORE_EXE[] = "store.out";
constexpr char PART_EXE[] = "part.out";

constexpr char PART_PATH[] = "stores/Parts.csv";
constexpr char PIPES_PATH[] = "namedpipes/";

constexpr char STORE_TO_PART[] = "store_to_part";

constexpr int BUFFER_SIZE = 2048;

#endif