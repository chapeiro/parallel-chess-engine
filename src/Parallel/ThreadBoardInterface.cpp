/**
 * ThreadBoardInterface.cpp
 *
 *  Created on: 2014/06/22
 *      Author: Chrysogelos Periklis
 */
#include <cassert>
#include "ThreadBoardInterface.hpp"


thread_data::thread_data(): used(1 << task_pop), job_id(0){}

inline task_id thread_data::createTaskId(unsigned int t) const{
   return t | (thrd_id << thrd_id_offset); 
}

inline unsigned int thread_data::peek_task(){
    unsigned int t = square((~used) & -(~used));
    if (t >= task_pop) return no_task;
    used |= 1 << t;
    return t;
}

task_id thread_data::createGoTask(Board *b, int depth, time_control tc){
    unsigned int t = peek_task();
    if (t == no_task) return no_task;
    tasks[t].board = new Board(b);
    tasks[t].type  = TaskType::IterativeDeepening;
    tasks[t].tc    = tc;
    tasks[t].depth = depth;
    tasks[t].job_id = job_id;
    tasks[t].st     = Pending;
    return createTaskId(t);
}

task_id thread_data::search(Board * __restrict brd, int depth, int alpha, int beta, bitboard tf, int prom){
    unsigned int t = peek_task();
    if (t == no_task) return no_task;
    tasks[t].board  = new Board(brd);
    tasks[t].type   = TaskType::PVS;
    tasks[t].depth  = depth;
    tasks[t].alpha  = alpha;
    tasks[t].beta   = beta;
    tasks[t].tf     = tf;
    tasks[t].prom   = prom;
    tasks[t].job_id = job_id;
    tasks[t].st     = Pending;
    return createTaskId(t);
}
// BoardInterface overwrite

ThreadBoardInterface::ThreadBoardInterface(bool hasUI): 
                                idle_threads(0), exiting(false), blocked(0){
    for (unsigned int i = 0 ; i < thread_pop ; ++i) {
        thr_dt[i].thrd = new std::thread(&ThreadBoardInterface::run, this, i);
        thr_dt[i].thrd_id = i;
    }

    thr_dt[UI_index].thrd = NULL;

    if (hasUI) thr_dt[UI_index].thrd = new std::thread(&communaticate);
    
    thr_dt[UI_index].thrd_id = UI_index;
}

ThreadBoardInterface::~ThreadBoardInterface(){
    //Wait for threads to terminate current tasks
    std::unique_lock<std::mutex> lk(pending_tasks_m);
    while (idle_threads != thread_pop){ //this may take some time...
        pending_tasks_cv.wait_for(lk,static_cast<std::chrono::milliseconds>(1));
    }
    lk.unlock();

    //send bombs to kill threads
    for (unsigned int i = 0 ; i < thread_pop ; ++i) {
        pending_tasks_queue.push(createBomb());
    }
    pending_tasks_cv.notify_all();

    //wait for threads to exit
    for (unsigned int i = 0 ; i < thread_pop ; ++i) {
        thr_dt[i].thrd->join();
        delete thr_dt[i].thrd;
    }
    std::unique_lock<std::mutex> rlk(running_m);
    exiting = true;
    running_cv.notify_all();
    running_cv.wait(rlk, [this](){return (blocked == 0);});
    rlk.unlock();
    //FIXME UI calls this ? If not, should be joined... else? detach ?
}

void ThreadBoardInterface::block(){
    std::unique_lock<std::mutex> lk(running_m);
    ++blocked;
    running_cv.wait(lk, [this](){return (exiting);});
    --blocked;
    running_cv.notify_all();
    lk.unlock();
}

bool ThreadBoardInterface::go(int depth, time_control tc){
    //should only be called by UI!!!
    assert(thr_dt[UI_index].thrd);
    assert(std::this_thread::get_id() == thr_dt[UI_index].thrd->get_id());
    if (!board) return false;
    task_id t = thr_dt[UI_index].createGoTask(board, depth, tc);
    if (t == no_task) return false;
    std::unique_lock<std::mutex> lk(pending_tasks_m);
    pending_tasks_queue.push(t);
    lk.unlock();
    pending_tasks_cv.notify_all();
    return true;
}

void ThreadBoardInterface::ui_garbage_collection(){
    thr_dt[UI_index].ui_garbage_collection();
}

void thread_data::ui_garbage_collection(){
    if (!thrd) return;
    assert(thrd->get_id() == std::this_thread::get_id());
    for (unsigned int i = 0 ; i < task_pop ; ++i) {
        if (!tasks[i].board) used &= ~(1 << i);
    }
}

void ThreadBoardInterface::stop(){
    // //should only be called by UI!!!
    // assert(thr_dt[UI_index].thrd);
    // assert(std::this_thread::get_id() == thr_dt[UI_index].thrd->get_id());
    std::unique_lock<std::mutex> lk(pending_tasks_m);
    while (idle_threads != thread_pop){ //this may take some time...
        interruption_requested = true;
        pending_tasks_cv.wait_for(lk,static_cast<std::chrono::milliseconds>(1));
    }
    lk.unlock();
}


// Interface specific


void ThreadBoardInterface::run(unsigned int id){
    assert(id >= 0);
    assert(id <  thread_pop);

    task_id t = 0;
    while (!exiting){
        std::unique_lock<std::mutex> lk(pending_tasks_m);
        ++idle_threads;
        do {
            // std::cout << "Thread " << std::setw(2) << id << " goes to sleep." << std::endl;
            pending_tasks_cv.wait(lk, 
                                [this](){return !pending_tasks_queue.empty();}
                            );
            // std::cout << "Thread " << std::setw(2) << id << " wakes up." << std::endl;
            t = pending_tasks_queue.front();
            pending_tasks_queue.pop();
        }while(!(isBomb(t) || thr_dt[t >> thrd_id_offset].tasks[t & thr_task_mask].isPending()));
        --idle_threads;
        lk.unlock();
        if (isBomb(t)) break;
        execute(t, id);
    }
}

void ThreadBoardInterface::execute(task_id t, unsigned int id){
    thr_dt[t >> thrd_id_offset].tasks[t & thr_task_mask].executeAs(id);
}

Task::Task(): board(NULL), st(Invalid){};

bool Task::isPending(){
    return (st == Pending);
}

bool Task::executeAs(unsigned int thrd_id){
    // std::lock_guard<std::mutex> lk(st_m);
    State t = Pending;
    if (!st.compare_exchange_strong(t, Executing)) return false;
    // st = Executing;
    Board * b = board.exchange(NULL);
    assert(b);
    b->setThreadID(thrd_id);
    // std::cout << std::setw(2) << thrd_id << " got task @" << std::setw(4) << depth << "(" << std::hex << ((void *) b) << ")" << std::dec<< std::endl;
    switch (type){
        case IterativeDeepening: //no result, clean it...
            b->go(depth, tc);
        case PVS:
            score = b->search(depth, alpha, beta);
        // default:
        //     assert(false);
        //     break;
    }
    delete b;
    st = Completed;
    return true;
}

bool ThreadBoardInterface::search(Board * __restrict brd, unsigned int thrd_id, int depth, int alpha, int beta, bitboard tf, int prom){
    task_id t = thr_dt[thrd_id].search(brd, depth, alpha, beta, tf, prom);
    if (t == no_task) return false;

    std::unique_lock<std::mutex> lk(pending_tasks_m);
    // std::cout << std::setw(2) << thrd_id << " set task @" << std::setw(4) << depth << "(" << std::hex << ((void *) brd) << ")" << std::dec<< std::endl;
    pending_tasks_queue.push(thr_dt[thrd_id].createTaskId(t));
    lk.unlock();
    pending_tasks_cv.notify_all();
    return true;
}

bool ThreadBoardInterface::collectNextScore(int &score, unsigned int thrd_id){
    return (thr_dt[thrd_id].collectNextScore(score));
}

bool thread_data::collectNextScore(int &score){
    task_bitmask mask  = thr_task_mask & used;
    task_bitmask mask2 = thr_task_mask & used;
    unsigned int t;
    do {
        task_bitmask tmp = used & mask2;
        if (!tmp) return lazy_execute(mask, score);
        task_bitmask lsb = tmp & -tmp;
        t = square(lsb);
        if (tasks[t].job_id == job_id){
            if (tasks[t].st == Completed) break;
        } else {
            mask ^= lsb;
        }
        mask2 ^= lsb;
    } while (true);
    score = tasks[t].score;
    tasks[t].st = Invalid;
    used ^= 1 << t;
    assert(!(used & (1 << t)));
    return true;
}

bool thread_data::lazy_execute(task_bitmask mask, int &score){
    bool ex = false;
    while(mask){
        unsigned int i = square(pop_lsb(mask));
        if (tasks[i].job_id == job_id){ 
            ++job_id;
            assert((used & (1 << i)));
            assert(tasks[i].st != Invalid);
            if(tasks[i].executeAs(thrd_id)){
                score = tasks[i].score;
                tasks[i].st = Invalid;
                used ^= 1 << i;
                assert(!(used & (1 << i)));
                --job_id;
                return true;
            }
            --job_id;
        }
    }
    return ex;
}