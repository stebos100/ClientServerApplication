#ifndef SIGNAL_HANDLER_H
#define SIGNAL_HANDLER_H

#include <csignal>
#include <atomic>
#include <iostream>

extern std::atomic<bool> running;

void signal_handler(int signal);

#endif
