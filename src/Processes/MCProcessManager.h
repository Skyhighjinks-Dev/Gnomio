//
// Created by amdro on 26/08/2024.
//
#pragma once

#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <string>
#include <memory>
#include "MCProcess.h"

class MCProcessManager {
public:
    MCProcessManager();
    ~MCProcessManager();

    void AddProcess(std::unique_ptr<MCProcess> process);
    void AddProcessSendKeys(MCProcess* nProcessPtr, const std::string& nKeys);
    MCProcess* GetProcessPtr(size_t index);
    int GetProcessCount() const;

private:
    struct Task {
        MCProcess* processPtr;
        std::string keys;
    };

    std::vector<std::unique_ptr<MCProcess>> processes;
    std::queue<Task> taskQueue;

    std::mutex queueMutex;
    std::condition_variable queueCondition;
    bool stopFlag = false;

    std::thread workerThread;

    void StartQueueThread();
    void Worker();
};