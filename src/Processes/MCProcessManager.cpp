//
// Created by amdro on 26/08/2024.
//

#include "../../include/Processes/MCProcessManager.h"
#include <chrono>
#include <thread>

MCProcessManager::MCProcessManager() {
    StartQueueThread();
}

MCProcessManager::~MCProcessManager() {
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        stopFlag = true;
    }
    queueCondition.notify_one();
    if (workerThread.joinable()) {
        workerThread.join();
    }
}

void MCProcessManager::AddProcess(std::unique_ptr<MCProcess> process) {
    processes.push_back(std::move(process));
}

void MCProcessManager::AddProcessSendKeys(MCProcess* nProcessPtr, const std::string& nKeys) {
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        taskQueue.push({nProcessPtr, nKeys});
    }
    queueCondition.notify_one();
}

MCProcess* MCProcessManager::GetProcessPtr(size_t index) {
    if (index < processes.size()) {
        return processes[index].get();
    }
    return nullptr;
}

void MCProcessManager::StartQueueThread() {
    workerThread = std::thread(&MCProcessManager::Worker, this);
}

void MCProcessManager::Worker() {
    while (true) {
        Task task;
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            queueCondition.wait(lock, [this] { return !taskQueue.empty() || stopFlag; });
            if (stopFlag && taskQueue.empty()) {
                return;
            }
            task = taskQueue.front();
            taskQueue.pop();
        }
        if (task.processPtr) {
            task.processPtr->SendKeys(task.keys, 100);
        }
    }
}


int MCProcessManager::GetProcessCount() const {
    return processes.size();
}
