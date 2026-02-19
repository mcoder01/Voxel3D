#ifndef THREAD_POOL_HPP
#define THREAD_POOL_HPP

#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <queue>

typedef struct {
    std::thread* thread;
    std::mutex* mut;
    std::condition_variable* cond;
    bool can_work;
} Worker;

typedef struct {
    int job_size, done_parts;
    std::function<void(int, int)> job;
    std::mutex* mut;
    std::condition_variable* cond;
    std::vector<Worker> workers;
} Pool;

void __subdivide_load(int loadSize, int workers, int id, int& start, int& size) {
    size = loadSize/workers;
    int remainder = loadSize%workers;
    int step = 0;
    if (id < remainder) size++;
    else step = remainder;
    start = size*id+step;
}

void __run(Pool* pool, int id) {
    while(true) {
        // Wait for start signal
        {
            std::unique_lock<std::mutex> lock(*pool->workers[id].mut);
            pool->workers[id].cond->wait(lock, [=] () {
                return pool->workers[id].can_work;
            });

            pool->workers[id].can_work = false;
        }

        // Execute current worker's part of the job
        int start, size;
        __subdivide_load(pool->job_size, pool->workers.size(), id, start, size);
        pool->job(start, size);

        // Signal that this part is done
        {
            std::unique_lock<std::mutex> lock(*pool->mut);
            pool->done_parts++;
        }
        pool->cond->notify_one();
    }
}

void PoolSubmit(Pool* pool, int load, std::function<void(int, int)> job) {
    pool->job_size = load;
    pool->job = job;
    pool->done_parts = 0;
    for (Worker& worker : pool->workers) {
        {
            std::unique_lock<std::mutex> lock(*worker.mut);
            worker.can_work = true;
        }
        worker.cond->notify_one();
    }
}

void WaitForJobFinish(Pool* pool) {
    std::unique_lock<std::mutex> lock(*pool->mut);
    pool->cond->wait(lock, [=] () {
        return pool->done_parts == pool->workers.size();
    });
}

Pool* CreatePool(int workers) {
    Pool* pool = new Pool;
    pool->mut = new std::mutex;
    pool->cond = new std::condition_variable;
    for (int i = 0; i < workers; i++)
        pool->workers.push_back({
            new std::thread {__run, pool, i},
            new std::mutex,
            new std::condition_variable,
            false
        });
    return pool;
}

#endif