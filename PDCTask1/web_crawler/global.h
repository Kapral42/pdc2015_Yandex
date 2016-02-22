#ifndef GLOBAL_H
#define GLOBAL_H

#include <iostream>
#include <fstream>
#include <thread>
#include <string>
#include <atomic>
#include <chrono>
#include <boost/lockfree/queue.hpp>

#include "downloader.h"
#include "parser.h"
#include "thread_safe_hash_set.hpp"

#define Q_P_S 1000 //fixed size parse queue
#define Q_D_S 1000 //fixed size download queue

//conf, read only
size_t N_THREADS;
size_t MAX_PARSE_THREAD;
size_t Q_D_MIN;
size_t Q_D_MAX;
size_t Q_P_MIN;
size_t Q_P_MAX;
size_t HASH_SIZE;
int TIMEOUT;

//input params
unsigned int MAX_DEPTH_GRAPH;
unsigned int MAX_DOWNLOAD_PAGES;
std::string PATH_CONTENTS;
bool DEBUGFLAG;

typedef std::pair<std::shared_ptr<std::string>, unsigned int> Tpair_s_ui;
typedef boost::lockfree::queue<Tpair_s_ui*,
		boost::lockfree::fixed_sized<true>> Tqueue_pairs;
//download queue
//first - url second - depth
Tqueue_pairs q_download{Q_D_S};
//pars queue
//first - content second - depth
Tqueue_pairs q_pars{Q_P_S};

//hash table reps
thread_safe_hash_set<std::string> table_reps(1);

std::atomic<unsigned int> n_parse_thread;
std::atomic<unsigned int> n_download_thread;
std::atomic<unsigned int> q_download_size;
std::atomic<unsigned int> q_pars_size;
std::atomic<unsigned int> n_save;
std::atomic<bool> stopflag;
std::atomic<bool> save_only;
std::atomic<unsigned int> pause_count;
std::atomic<bool> pauseflag;

std::chrono::high_resolution_clock::time_point time_start;
std::chrono::high_resolution_clock::time_point time_end;

#endif
