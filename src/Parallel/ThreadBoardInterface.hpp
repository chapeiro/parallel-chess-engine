/**
 * ThreadBoardInterface.hpp
 *
 *  Created on: 2014/06/22
 *      Author: Chrysogelos Periklis
 */

#ifndef THREADBOARDINTERFACE_HPP_
#define THREADBOARDINTERFACE_HPP_

#include <thread>
#include <mutex>
#include <deque>
#include <atomic>
// #include "../BoardInterface/BoardInterface.hpp"
#include "../BoardInterface/BareBoardInterface.hpp"

constexpr unsigned int thread_pop(8);
constexpr unsigned int UI_index(thread_pop+1);
constexpr unsigned int MASTER_index(thread_pop);
constexpr unsigned int task_pop(16); // per thread
constexpr unsigned int thrd_id_offset(16);
constexpr unsigned int thr_task_mask((1 << task_pop) - 1);
constexpr unsigned int job_max(256);

typedef unsigned int task_id;
#define MPI_CCHAPEIRO_TASK_ID MPI_UNSIGNED
typedef uint32_t     task_bitmask;
constexpr task_id no_task(-2);

static_assert(thread_pop*(task_pop + 2) <= MAX_BOARDS, "Low MAX_BOARDS");

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


void makeSends();
void makeReceives();
bool runProcessCommunicator(int argc, char* argv[]);
void sendWindowUpdates();

class thread_data;

class Task{
    friend class thread_data;
    friend class ThreadBoardInterface;
    friend void makeSends();
    friend void makeReceives();
    friend bool runProcessCommunicator(int argc, char* argv[]);
    friend void sendWindowUpdates();

private:
    std::atomic<Board*>     board;
    TaskType                type;
    time_control            tc;
    int                     depth;
    int                     alpha;
    int                     beta;
    int                     score;
    internal_move           move;
    Window*                 w;
    std::atomic<State>      st;
    std::mutex              st_m; //lock only to get in Executing or Completed
    uint64_t                job_id;

public:
    Task();

    bool executeAs(thread_data *thrd, task_id taskID, thread_data *owner);
    bool isPending();
};

class thread_data{
    friend class ThreadBoardInterface;
    friend class Task;
    friend void makeSends();
    friend void makeReceives();
    friend bool runProcessCommunicator(int argc, char* argv[]);
private:
    std::thread*    thrd;
    unsigned int    thrd_id;
    Task            tasks[task_pop];
    // std::atomic<task_bitmask>    used; //only thrd should have access
    uint64_t        job_id;
    uint64_t        job_id_last;
    std::atomic<task_bitmask> used_tot;
    std::atomic<task_bitmask> used_arr[job_max];
    std::atomic<task_bitmask> cmpl_arr[job_max];
    Window                    w[job_max];

public:
    bool isFull() const;
    bool isEmpty() const;
    thread_data();
    task_id createGoTask(Board *b, int depth, time_control tc);
    int collectNextScore();
    task_id search(Board * __restrict brd, int depth, int alpha, int beta, const internal_move &child);
    task_id search2(Board * __restrict brd, int depth, int alpha, int beta, const internal_move &child);

    bool collectNextScore(int &score, int depth, internal_move &child);
    bool collectNextScoreUB(int &score, int depth, internal_move &child);
    bool lazy_execute(int &score, int depth, internal_move &child);

    task_id getCompletedTask() const;
protected:
    task_id createTaskId(unsigned int t) const;
    unsigned int peek_task();
    unsigned int peek_task2();
    void increaseDepth();
    void decreaseDepth();
    void ui_garbage_collection();
    void updateWindows(int alpha, int beta);
};

/**
 * Base class methods should only be called by UI!!!
 */
class ThreadBoardInterface: public BareBoardInterface{
public:
    thread_data                     thr_dt[thread_pop+2]; //+1 is for roots
    std::mutex                      pending_tasks_m ;
    std::condition_variable         pending_tasks_cv;
    std::deque<task_id>             pending_tasks_queue;

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
    unsigned int search_rind(Board * __restrict brd, unsigned int thrd_id, int depth, int alpha, int beta, const internal_move &child);
    virtual bool search(Board * __restrict brd, unsigned int thrd_id, int depth, int alpha, int beta, const internal_move &child);
    // virtual void perft();
    virtual void stop();
    virtual void ui_garbage_collection();

    virtual bool collectNextScore(int &score, unsigned int thrd_id, int depth, internal_move &child);
    virtual bool collectNextScoreUB(int &score, unsigned int thrd_id, int depth, internal_move &child);
    virtual void increaseDepth(unsigned int thrd_id);
    virtual void decreaseDepth(unsigned int thrd_id);
    virtual void updateWindows(int alpha, int beta, unsigned int thrd_id);
    void block();
private:
    void run(unsigned int thrd_id);
    void execute(task_id t, unsigned int id);


public:
    bool isFull(unsigned int thrd_id) const;
    bool isEmpty(unsigned int thrd_id) const;

    task_id getCompletedTask(unsigned int thrd_id) const;

    static constexpr task_id createBomb(){
        return (~task_id(0));
    }

    static constexpr task_id createPrInterrupt(){
        return (~task_id(2));
    }

    static constexpr task_id createPrInterrupt2(){
        return (~task_id(3));
    }

    static constexpr bool isMasters(task_id t){
        return ((t >> thrd_id_offset) == MASTER_index);
    }

    bool isGo(task_id t){
        return ((t >> thrd_id_offset) == MASTER_index);
    }

    static constexpr bool isPrInterrupt(task_id t){
        return (t == createPrInterrupt());
    }

    static constexpr bool isPrInterrupt2(task_id t){
        return (t == createPrInterrupt2());
    }

    static constexpr bool isBomb(task_id t){
        return (t == createBomb());
    }
};

#endif /* THREADBOARDINTERFACE_HPP_ */