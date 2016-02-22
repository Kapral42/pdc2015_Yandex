#!/bin/sh

g++ -Wall -g -std=c++11 -O2 -o main_series ./parser.cpp downloader.cpp main_series.cpp  -pthread -lcurl
