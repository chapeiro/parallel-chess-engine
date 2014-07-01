/**
 * TimeManager.hpp
 *
 *  Created on: 2014/06/22
 *      Author: Chrysogelos Periklis
 */

#ifndef THREADBOARDINTERFACE_HPP_
#define THREADBOARDINTERFACE_HPP_

#include <thread>
#include <mutex>
#include <queue>
#include <atomic>
// #include "../BoardInterface/BoardInterface.hpp"
#include "../BoardInterface/BareBoardInterface.hpp"
#include "../Board.hpp"

constexpr unsigned int thread_pop(4);
constexpr unsigned int UI_index(thread_pop+1);
constexpr unsigned int task_pop(8); // per thread
constexpr unsigned int thrd_id_offset(16);
constexpr unsigned int thr_task_mask((1 << task_pop) - 1);

typedef unsigned int task_id;
typedef uint32_t     task_bitmask;
constexpr task_id no_task(-2);

enum TaskType{
    PVS,
    IterativeDeepening
};

enum State{
    Invalid,
    Pending,
    Executing,
    Completed
};

class Task{
    friend class thread_data;
private:
    std::atomic<Board*>     board;
    TaskType                type;
    time_control            tc;
    int                     depth;
    int                     alpha;
    int                     beta;
    int                     score;
    bitboard                tf;
    int                     prom;
    State                   st;
    std::mutex              st_m; //lock only to get in Executing or Completed
    uint64_t                job_id;

public:
    Task();

    bool executeAs(unsigned int thrd_id);
    bool isPending();
};

class thread_data{
    friend class ThreadBoardInterface;
private:
    std::thread*    thrd;
    unsigned int    thrd_id;
    Task            tasks[task_pop];
    task_bitmask    used; //only thrd should have access
    uint64_t        job_id;
    uint64_t        job_id_last;

public:
    thread_data();
    task_id createGoTask(Board *b, int depth, time_control tc);
    int collectNextScore();
    task_id search(Board * __restrict brd, int depth, int alpha, int beta, bitboard tf, int prom);

    bool collectNextScore(int &score);
    bool lazy_execute(task_bitmask mask, int &score);
protected:
    task_id createTaskId(unsigned int t) const;
    unsigned int peek_task();
    void ui_garbage_collection();
};

/**
 * Base class methods should only be called by UI!!!
 */
class ThreadBoardInterface: public BareBoardInterface{
public:
    thread_data                     thr_dt[thread_pop+2]; //+1 is for roots
    std::mutex                      pending_tasks_m ;
    std::condition_variable         pending_tasks_cv;
    std::queue<task_id>             pending_tasks_queue;

    // Board *                        board;        //root
    unsigned int                    idle_threads; //protected by pending_tasks_m
private:
    bool                            exiting;
    unsigned int                    blocked;
    std::mutex                      running_m ;
    std::condition_variable         running_cv;

public:
    ThreadBoardInterface(bool hasUI=false);
    virtual ~ThreadBoardInterface();

    virtual bool go(int depth, time_control tc);
    virtual bool search(Board * __restrict brd, unsigned int thrd_id, int depth, int alpha, int beta, chapeiro::bitboard tf, int prom);
    // virtual void perft();
    virtual void stop();
    virtual void ui_garbage_collection();

    virtual bool collectNextScore(int &score, unsigned int thrd_id);
    void block();
private:
    void run(unsigned int thrd_id);
    void execute(task_id t, unsigned int id);

    static constexpr task_id createBomb(){
        return (~task_id(0));
    }

    static constexpr bool isBomb(task_id t){
        return (t == createBomb());
    }
};

#endif /* THREADBOARDINTERFACE_HPP_ */