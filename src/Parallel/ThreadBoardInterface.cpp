/**
 * ThreadBoardInterface.cpp
 *
 *  Created on: 2014/06/22
 *      Author: Chrysogelos Periklis
 */
#include <cassert>
#include "ThreadBoardInterface.hpp"


thread_data::thread_data(): job_id(0), used_tot(1 << task_pop){
    for (unsigned int i = 0 ; i < job_max ; ++i) {
        used_arr[i] = 1 << task_pop;
        cmpl_arr[i] = 0;
        w[i].alpha = -inf;
        w[i].beta  = +inf;
    }
}

inline task_id thread_data::createTaskId(unsigned int t) const{
   return t | (thrd_id << thrd_id_offset); 
}

inline bool thread_data::isFull() const {
    return !((~used_tot) & thr_task_mask);
}

inline bool thread_data::isEmpty() const {
    return !(used_tot & thr_task_mask);
}

inline unsigned int thread_data::peek_task(){
    unsigned int t = square((~used_tot) & -(~used_tot));
    if (t >= task_pop) return no_task;
    for (int i = 0 ; i < job_max ; ++i) assert(!(used_arr[i] & (1 << t)));
    for (int i = 0 ; i < job_max ; ++i) assert(!(cmpl_arr[i] & (1 << t)));
    used_tot         |= 1 << t;
    used_arr[job_id] |= 1 << t;
    return t;
}

task_id thread_data::createGoTask(Board *b, int depth, time_control tc){
    unsigned int t  = peek_task();
    if (t == no_task) return no_task;
    tasks[t].board  = new Board(b);
    tasks[t].type   = TaskType::IterativeDeepening;
    tasks[t].tc     = tc;
    tasks[t].depth  = depth;
    tasks[t].job_id = job_id;
    tasks[t].w      = w+job_id;
    tasks[t].st     = Pending;
    return createTaskId(t);
}

inline unsigned int thread_data::peek_task2(){
    unsigned int t = square((~used_tot) & -(~used_tot));
    if (t >= task_pop) return no_task;
    job_id = t;
    for (int i = 0 ; i < job_max ; ++i) assert(!(used_arr[i] & (1 << t)));
    for (int i = 0 ; i < job_max ; ++i) assert(!(cmpl_arr[i] & (1 << t)));
    used_tot         |= 1 << t;
    used_arr[t]      |= 1 << t;
    return t;
}

task_id thread_data::search2(Board * __restrict brd, int depth, int alpha, int beta, const internal_move &child){
#ifndef SIMULATE_SINGLETHREAD
    unsigned int t  = peek_task2();
    if (t == no_task) return no_task;
    Board * b       = new Board(brd);
    tasks[t].board  = b;
    tasks[t].type   = TaskType::PVS;
    tasks[t].depth  = depth;
    tasks[t].alpha  = alpha;
    tasks[t].beta   = beta;
    tasks[t].move   = child;
    tasks[t].job_id = job_id;
    tasks[t].w      = w+job_id;
    tasks[t].st     = Pending;
    return createTaskId(t);
#else
    return no_task;
#endif
}

task_id thread_data::search(Board * __restrict brd, int depth, int alpha, int beta, const internal_move &child){
#ifndef SIMULATE_SINGLETHREAD
    unsigned int t  = peek_task();
    if (t == no_task) return no_task;
    Board * b       = new Board(brd);
    tasks[t].board  = b;
    tasks[t].type   = TaskType::PVS;
    tasks[t].depth  = depth;
    tasks[t].alpha  = alpha;
    tasks[t].beta   = beta;
    tasks[t].move   = child;
    tasks[t].job_id = job_id;
    tasks[t].w      = w+job_id;
    tasks[t].st     = Pending;
    return createTaskId(t);
#else
    return no_task;
#endif
}
// BoardInterface overwrite

ThreadBoardInterface::ThreadBoardInterface(bool hasUI): 
                                idle_threads(0), exiting(false), blocked(0){
    for (unsigned int i = 0 ; i < thread_pop ; ++i) {
        thr_dt[i].thrd = new std::thread(&ThreadBoardInterface::run, this, i);
        thr_dt[i].thrd_id = i;
    }

    thr_dt[UI_index].thrd = NULL;
    thr_dt[MASTER_index].thrd = NULL;

    if (hasUI) thr_dt[UI_index].thrd = new std::thread(&communaticate);
    
    thr_dt[UI_index].thrd_id = UI_index;
    thr_dt[MASTER_index].thrd_id = MASTER_index;
}

ThreadBoardInterface::~ThreadBoardInterface(){
    //Wait for threads to terminate current tasks
    std::unique_lock<std::mutex> lk(pending_tasks_m);
    while (idle_threads != thread_pop){ //this may take some time...
        pending_tasks_cv.wait_for(lk,static_cast<std::chrono::milliseconds>(1));
    }
    lk.unlock();

    //send bombs to kill threads
    for (unsigned int i = 0 ; i < thread_pop+1 ; ++i) { //+1 for master
        pending_tasks_queue.push_back(createBomb());
    }
    pending_tasks_cv.notify_all();

    //wait for threads to exit
    for (unsigned int i = 0 ; i < thread_pop ; ++i) {
        thr_dt[i].thrd->join();
        delete thr_dt[i].thrd;
    }

    //Posible configurations:
    // Master | UI
    //    1   |  1   => blocked = 0 (  ui   calls destructor)
    //    0   |  1   => blocked = 1 (  ui   calls destructor)
    //    1   |  0   => blocked = 0 (master calls destructor)

    //if UI thread exists, UI is the only one allowed to call this destructor
    //in this case, MASTER (if exists) is notified by the bomb send above.
    //if UI thread does not exists, MASTER is the only one allowed to call
    //this destructor. Master will not be in blocked count, so running_cv.wait
    //will immediatly exit
    std::unique_lock<std::mutex> rlk(running_m);
    exiting = true;
    running_cv.notify_all();
    running_cv.wait(rlk, [this](){return (blocked == 0);});
    rlk.unlock();
    //do NOTHING after this unlock!!!!!!
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
    pending_tasks_queue.push_back(t);
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
        if (!tasks[i].board) {
            used_tot         &= ~(1 << i);
            used_arr[job_id] &= ~(1 << i);
            cmpl_arr[job_id] &= ~(1 << i);
            for (int t = 0 ; t < job_max ; ++t) assert(!(used_arr[t] & (1 << i)));
            for (int t = 0 ; t < job_max ; ++t) assert(!(cmpl_arr[t] & (1 << i)));
        }
    }
}

void ThreadBoardInterface::stop(){
    // //should only be called by UI!!!
    // assert(thr_dt[UI_index].thrd);
    // assert(std::this_thread::get_id() == thr_dt[UI_index].thrd->get_id());
    std::unique_lock<std::mutex> lk(pending_tasks_m);
    while (idle_threads != thread_pop || !(thr_dt[MASTER_index].isEmpty())){ //this may take some time...
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
            //std::cout << "Thread " << std::setw(2) << id << " goes to sleep." << std::endl;
            pending_tasks_cv.wait(lk, 
                                [this](){return !pending_tasks_queue.empty();}
                            );
            assert(!pending_tasks_queue.empty());
            //std::cout << "Thread " << std::setw(2) << id << " wakes up." << std::endl;
            t = pending_tasks_queue.front();
            pending_tasks_queue.pop_front();
        }while(!(isBomb(t) || thr_dt[t >> thrd_id_offset].tasks[t & thr_task_mask].isPending()));
        --idle_threads;
        lk.unlock();
        if (isBomb(t)) break;
        execute(t, id);
    }
}

void ThreadBoardInterface::execute(task_id t, unsigned int id){
    assert((t >> thrd_id_offset) != id);
    thr_dt[id].increaseDepth();
    thr_dt[t >> thrd_id_offset].tasks[t & thr_task_mask].executeAs(thr_dt+id, t & thr_task_mask, thr_dt+(t >> thrd_id_offset));
    thr_dt[id].decreaseDepth();
}

Task::Task(): board(NULL), move(0, 0), st(Invalid){};

bool Task::isPending(){
    return (st == Pending);
}

bool Task::executeAs(thread_data *thrd, task_id taskID, thread_data *owner){
    // std::lock_guard<std::mutex> lk(st_m);
   State t = Pending;
    if (!st.compare_exchange_strong(t, Executing)) return false;
    // st = Executing;
    Board * b = board.exchange(NULL);
    assert(b);
    b->setThreadID(thrd->thrd_id);
    b->setWindow(w);
    // std::cout << std::setw(2) << thrd_id << " got task @" << std::setw(4) << depth << "(" << std::hex << ((void *) b) << ")" << std::dec<< std::endl;
    switch (type){
        case IterativeDeepening: //no result, clean it...
            b->go(depth, tc);
            break;
        case PVS:
            score = b->search(depth, alpha, beta);
            break;
        // default:
        //     assert(false);
        //     break;
    }
    delete b;
    assert(st == Executing);
    st = Completed;
    for (int i = 0 ; i < job_max ; ++i) assert(!(owner->cmpl_arr[i] & (1 << taskID)));
    unsigned int p = job_id;
    owner->cmpl_arr[p] ^= 1 << taskID;
    assert((owner->cmpl_arr[p].load() & (1 << taskID)) || st == Invalid);
    return true;
}

bool ThreadBoardInterface::isFull(unsigned int thrd_id) const{
    return thr_dt[thrd_id].isFull();
}

bool ThreadBoardInterface::isEmpty(unsigned int thrd_id) const{
    return thr_dt[thrd_id].isEmpty();
}

unsigned int ThreadBoardInterface::search_rind(Board * __restrict brd, unsigned int thrd_id, int depth, int alpha, int beta, const internal_move &child){
    task_id t = thr_dt[thrd_id].search2(brd, depth, alpha, beta, child);
    if (t == no_task) return no_task;

    std::unique_lock<std::mutex> lk(pending_tasks_m);
    // std::cout << std::setw(2) << thrd_id << " set task @" << std::setw(4) << depth << "(" << std::hex << ((void *) brd) << ")" << std::dec<< std::endl;
    pending_tasks_queue.push_back(t);
    lk.unlock();
    pending_tasks_cv.notify_all();
    return (t & thr_task_mask);
}

bool ThreadBoardInterface::search(Board * __restrict brd, unsigned int thrd_id, int depth, int alpha, int beta, const internal_move &child){
    task_id t = thr_dt[thrd_id].search(brd, depth, alpha, beta, child);
    if (t == no_task) return false;

    std::unique_lock<std::mutex> lk(pending_tasks_m);
    // std::cout << std::setw(2) << thrd_id << " set task @" << std::setw(4) << depth << "(" << std::hex << ((void *) brd) << ")" << std::dec<< std::endl;
    pending_tasks_queue.push_back(t);
    lk.unlock();
    pending_tasks_cv.notify_all();
    return true;
}

bool ThreadBoardInterface::collectNextScore(int &score, unsigned int thrd_id, int depth, internal_move &child){
    return (thr_dt[thrd_id].collectNextScore(score, depth, child));
}

bool ThreadBoardInterface::collectNextScoreUB(int &score, unsigned int thrd_id, int depth, internal_move &child){
    return (thr_dt[thrd_id].collectNextScoreUB(score, depth, child));
}

void ThreadBoardInterface::updateWindows(int alpha, int beta, unsigned int thrd_id){
    return (thr_dt[thrd_id].updateWindows(alpha, beta));
}

void thread_data::updateWindows(int alpha, int beta){
    w[job_id].alpha = alpha;
    w[job_id].beta  = beta ;
}

bool thread_data::collectNextScoreUB(int &score, int depth, internal_move &child){
    task_bitmask mask = cmpl_arr[job_id];
    // while (mask){
    if (!mask) return false;
    unsigned int t = square(pop_lsb(mask));
    assert(tasks[t].st == Completed); // if
    assert(tasks[t].job_id == job_id);
    assert(tasks[t].depth == depth);
    score = tasks[t].score;
    child = tasks[t].move;
    tasks[t].st = Invalid;
    used_tot         ^= 1 << t;
    used_arr[job_id] ^= 1 << t;
    cmpl_arr[job_id] ^= 1 << t;
    assert(!(used_tot         & (1 << t)));
    for (int i = 0 ; i < job_max ; ++i) assert(!(used_arr[i] & (1 << t)));
    for (int i = 0 ; i < job_max ; ++i) assert(!(cmpl_arr[i] & (1 << t)));
    return true;
    //     }
    // }
    // return false;
}

bool thread_data::collectNextScore(int &score, int depth, internal_move &child){
    return lazy_execute(score, depth, child);
}

bool thread_data::lazy_execute(int &score, int depth, internal_move &child){
    if (!(thr_task_mask & used_arr[job_id])) return false;
    while(true){
        if (collectNextScoreUB(score, depth, child)) return true;
        task_bitmask mask = thr_task_mask & used_arr[job_id];
        increaseDepth();
        while (mask){
            unsigned int t = square(pop_lsb(mask));
        // decreaseDepth();
            // assert(tasks[t].job_id == job_id);
            assert((used_tot         & (1 << t)));
            // assert((used_arr[job_id] & (1 << t)));
        // increaseDepth();
            assert(tasks[t].st != Invalid);
            if(tasks[t].executeAs(this, t, this)){
                assert(tasks[t].depth == depth);
                score = tasks[t].score;
                child = tasks[t].move;
                tasks[t].st = Invalid;
                decreaseDepth();
                used_tot         ^= 1 << t;
                used_arr[job_id] ^= 1 << t;
                cmpl_arr[job_id] ^= 1 << t;
                assert(!(used_tot         & (1 << t)));
                for (int i = 0 ; i < job_max ; ++i) assert(!(used_arr[i] & (1 << t)));
                for (int i = 0 ; i < job_max ; ++i) assert(!(cmpl_arr[i] & (1 << t)));
                return true;
            }
        }
        decreaseDepth();
        // std::this_thread::yield();
    }
    assert(false);
    return false;
}

void thread_data::increaseDepth() {
    assert(std::this_thread::get_id() == this->thrd->get_id());
    ++job_id;
    w[job_id].alpha = -inf;
    w[job_id].beta  = +inf;
    assert(job_id < job_max);
}

void thread_data::decreaseDepth() {
    assert(std::this_thread::get_id() == this->thrd->get_id());
    --job_id;
    assert(job_id >= 0);
}

void ThreadBoardInterface::increaseDepth(unsigned int thrd_id) {
    thr_dt[thrd_id].increaseDepth();
}

void ThreadBoardInterface::decreaseDepth(unsigned int thrd_id) {
    thr_dt[thrd_id].decreaseDepth();
}

task_id ThreadBoardInterface::getCompletedTask(unsigned int thrd_id) const{
    return thr_dt[MASTER_index].getCompletedTask();
}

task_id thread_data::getCompletedTask() const{
    //only by master thread
    task_bitmask mask  = thr_task_mask & used_tot;
    while (mask){
        unsigned int t = square(pop_lsb(mask));
        if (tasks[t].st == Completed && (cmpl_arr[t] & (task_bitmask(1) << t))) return t;
    }
    return no_task;
}