/**
 * ProcessCommunication.cpp
 *
 *  Created on: 2014/07/05
 *      Author: Chrysogelos Periklis
 */

#include <algorithm>
#include "ProcessCommunication.hpp"
#include "mpi.h"
#include "../BoardInterface/BoardInterface.hpp"
#include "ThreadBoardInterface.hpp"
#include "../Networking/socket_utils.hpp"

constexpr int UI_proc = 0;
int proc_pop  = 1;
int proc_rank = UI_proc;

//searches spawned to other processes 
unsigned int spawned_searches = 0;

struct search_message{
    Board                   b;          //thread id is invalid!!!
    int                     depth;
    int                     alpha;
    int                     beta;
    internal_move           move;
    int                     proc_source;
    task_id                 proc_task_id;
    unsigned int            pr_index;

    search_message(): move(0, 0){}
} incoming_search, outgoing_search;

bool outgoing_result_pending = false;
bool outgoing_search_pending = false;

MPI_Request outgoing_search_req[2];
MPI_Request outgoing_result_req;

struct search_result{
    int                     score;
    task_id                 proc_task_id;
    unsigned int            pr_index;
} result, out_result;

//special tasks, are task that can be identified only by their task id
//such as a bomb or an interrupt all
constexpr int special_tasks_pop = 3;
constexpr int interrupt_tag     = 0;
constexpr int bomb_tag          = 1;
constexpr int interrupt2_tag    = 2;
constexpr int interrupt_idx     = 0;
constexpr int bomb_idx          = 1;
constexpr int interrupt2_idx    = 2;

MPI_Request sp_req[special_tasks_pop];
// MPI_Status  sp_stt[special_tasks_pop];
task_id     sp_tsk[special_tasks_pop];

constexpr int recv_pop          = 3;
constexpr int search_tag        = 3;
constexpr int search_idx        = 0;
constexpr int result_tag        = 4;
constexpr int result_idx        = 1;
constexpr int window_tag        = 5;
constexpr int window_idx        = 2;

unsigned int search_ack;
int search_ack_ans;
MPI_Request search_ack_req;
constexpr int search_ack_tag    = 6;
// MPI_Request recv_res[recv_pop];

int dest = 0;

constexpr int uninterrupt_tag   = 4;
bool interrupt_responce_pending = false;
bool ending_interrupt = false;

ThreadBoardInterface* tbi = NULL;

struct communication_data{
    task_id             proc_task_id;
    int                 proc_source ;
    Window              w;
    bool                valid;
    unsigned int        pr_index;
} comm_data[task_pop];

uint64_t active_out_jobs = 0;
uint64_t nt = 0;

struct window_update{
    unsigned int index;
    int alpha;
    int beta ;
} win_update, win_out;

struct out_searches{
    Task *       t;
    int          alpha;
    int          beta;
    unsigned int i;
    int          p;
} out_jobs[64];

class MPI_Request_Wrapper{
    MPI_Request req;
    bool        req_active;

public:
    MPI_Request_Wrapper(): req_active(false){}

    void irecv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm = MPI_COMM_WORLD){
        assert(buf);
        assert(!req_active);
        MPI_Irecv(buf, count, datatype, source, tag, comm, &req);
        req_active = true;
    }

    bool test(MPI_Status * status = MPI_STATUS_IGNORE){
        if (!req_active) return false;
        int flag = 0;
        MPI_Test(&req, &flag, status);
        if (flag) req_active = false;
        return flag;
    }

    void cancel(){
        MPI_Cancel(&req);
    }
} recv_res[recv_pop];

bool runProcessCommunicator(int argc, char* argv[]){
    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD, &proc_pop); //get running MPI processes
    MPI_Comm_rank(MPI_COMM_WORLD, &proc_rank);     //get current rank

    if (proc_pop <= 1) {
        MPI_Finalize();
        return false;
    }

    tbi             = new ThreadBoardInterface(!proc_rank);
    board_interface = tbi;

    //set buffer to wait for special tasks (bomb)
    if (proc_rank != UI_proc){
        MPI_Irecv(&sp_tsk[interrupt_idx ], 1, MPI_CCHAPEIRO_TASK_ID, UI_proc, interrupt_tag , MPI_COMM_WORLD, &sp_req[interrupt_idx]);
        MPI_Irecv(&sp_tsk[bomb_idx      ], 1, MPI_CCHAPEIRO_TASK_ID, UI_proc, bomb_tag      , MPI_COMM_WORLD, &sp_req[bomb_idx     ]);
        MPI_Irecv(&sp_tsk[interrupt2_idx], 1, MPI_CCHAPEIRO_TASK_ID, UI_proc, interrupt2_tag, MPI_COMM_WORLD, &sp_req[interrupt2_idx]);
    } else {
        std::cout << "Num of Processes: " << proc_pop << std::endl;
    }

    recv_res[search_idx].irecv(&incoming_search, sizeof(incoming_search), MPI_BYTE, MPI_ANY_SOURCE, search_tag, MPI_COMM_WORLD);
    recv_res[window_idx].irecv(&win_update     , sizeof(window_update  ), MPI_BYTE, MPI_ANY_SOURCE, window_tag, MPI_COMM_WORLD);
    // MPI_Irecv(&result         , sizeof(search_result  ), MPI_BYTE, MPI_ANY_SOURCE, result_tag, MPI_COMM_WORLD, &recv_res[result_idx]);

    // for (unsigned int i = 0 ; i < task_pop ; ++i) comm_data[i].valid = false;

    while (true){
        // interrupt_requested | interrupt_responce_pending
        // 0 | 0    => all permitted
        // 1 | 0    => all receives permitted, only sending results, no searches allowed to be send from this process
        // 1 | 1    => all receives permitted, sending results, finishing already started search sends
        // 0 | 1    => INVALID
        if (!handleSpecialMessageTasks()) break;
        makeReceives();
        makeSends();
        if (ending_interrupt && interruption_requested && tbi->isEmpty(MASTER_index)){
            ending_interrupt = interruption_requested = false;
        }
        //Send search positions and receive results
    }

    //FIXME if interrupt is not send before bomb, pending searches may exist, leading to a deadlock
    recv_res[search_idx].cancel();
    recv_res[window_idx].cancel();
    // MPI_Cancel(&recv_res[result_idx]);

    if (proc_rank == UI_proc){         //UI process, send bombs to everyone!!!
        interrupt_all();
        task_id bomb = ThreadBoardInterface::createBomb();
        for (int i = 1 ; i < proc_pop ; ++i){
            std::cout << "Sending Bomb to process " << std::setw(4) << i << "of" << proc_pop << std::endl;
            MPI_Send(&bomb, 1, MPI_CCHAPEIRO_TASK_ID, i, bomb_tag, MPI_COMM_WORLD);
        }
        // MPI_Bcast(&bomb, 1, MPI_CCHAPEIRO_TASK_ID, 0, MPI_COMM_WORLD);
    } else {            //Non-UI process, stop everything and abort
        board_interface->stop();
        delete board_interface;
        std::cout << "Process " << std::setw(4) << proc_rank << "of" << proc_pop << " aborting" << std::endl;
    }
    MPI_Finalize();
    return true;
}

void interrupt_all(){
    assert(proc_rank == UI_proc);
    std::cout << "UI Process - Master Thread sends interrupts" << std::endl;
    task_id inter = ThreadBoardInterface::createPrInterrupt();
    MPI_Request reqs[2*(proc_pop-1)];
    task_id t[proc_pop-1];
    for (int i = 1 ; i < proc_pop ; ++i){
        std::cout << "Sending Interrupt to process " << std::setw(4) << i << "of" << proc_pop << std::endl;
        MPI_Isend(&inter , 1, MPI_CCHAPEIRO_TASK_ID, i, interrupt_tag, MPI_COMM_WORLD, &reqs[i-1]);
        MPI_Irecv(&t[i-1], 1, MPI_CCHAPEIRO_TASK_ID, i, interrupt_tag, MPI_COMM_WORLD, &reqs[(proc_pop-1)+i-1]);
    }
    std::cout << "UI Process - Master Thread waits for interrupts to get off" << std::endl;
    int flag = 0;
    do {
        //receive while 
        makeReceives();
        makeSends();
        MPI_Testall(2*(proc_pop-1), reqs, &flag, MPI_STATUSES_IGNORE);
    } while (!flag);
    std::cout << "UI Process - Master Thread sending interrupts completed" << std::endl;
    task_id uninter = ThreadBoardInterface::createPrInterrupt2();
    for (int i = 1 ; i < proc_pop ; ++i){
        std::cout << "Sending Interrupt to process " << std::setw(4) << i << "of" << proc_pop << std::endl;
        MPI_Isend(&uninter , 1, MPI_CCHAPEIRO_TASK_ID, i, interrupt2_tag, MPI_COMM_WORLD, &reqs[i-1]);
    }
    MPI_Waitall(proc_pop-1, reqs, MPI_STATUSES_IGNORE);
    do {
        //receive while 
        makeReceives();
        makeSends();
    } while (spawned_searches);
}

int handleSpecialMessageTasks(){
    if (proc_rank == UI_proc){
        std::unique_lock<std::mutex> lk(tbi->pending_tasks_m);
        //Dispatch special tasks
        if (tbi->pending_tasks_cv.wait_for(lk, std::chrono::nanoseconds(10), [](){
                return (
                            !(tbi->pending_tasks_queue.empty()) && 
                            (
                                ThreadBoardInterface::isBomb(
                                        tbi->pending_tasks_queue.front()
                                )
                            )
                        )
                        ;
            })){
            //wake by predicate, task should be collected
            task_id t = tbi->pending_tasks_queue.front();
            tbi->pending_tasks_queue.pop_front();
            std::cout << "UI Process - Master Thread opens its eyes" << std::endl;
            std::cout << "UI Process - Master Thread wakes up" << std::endl;
            lk.unlock();

            //process task
            if (ThreadBoardInterface::isBomb(t)) return 0; //Only UI process
        } else {
            //time out
            if (interruption_requested && spawned_searches) { 
                lk.unlock();
                //if there exist a working thread, its process is a 
                //distant child of this process in working tree
                interrupt_all();
            } else {
                lk.unlock();
            }
            //keep it until interrupts have been send 
            //this prevents starting a new search before other threads are
            //notified for the interrupt
            //NOTE recheck above idea
        }
    } else {
        //handle special tasks
        std::this_thread::sleep_for(std::chrono::nanoseconds(10));
        int flag  = 0;
        int index = 0;
        MPI_Status status;

        do{
            MPI_Testany(special_tasks_pop, sp_req, &index, &flag, &status);
            if (!flag) return 1;
            //a special task has been completed, handle it
            //task can be identified by index as well, but checking its type, is more portable...
            if (ThreadBoardInterface::isBomb(sp_tsk[index])){
                std::cout << "Process " << std::setw(4) << proc_rank << "of" << proc_pop << " received a bomb!" << std::endl;
                MPI_Cancel(&sp_req[interrupt_idx]);
                return 0;
            } else if (ThreadBoardInterface::isPrInterrupt(sp_tsk[index])){
                MPI_Irecv(&sp_tsk[interrupt_idx], 1, MPI_CCHAPEIRO_TASK_ID, UI_proc, interrupt_tag, MPI_COMM_WORLD, &sp_req[interrupt_idx]);
                std::cout << "Process " << std::setw(4) << proc_rank << "of" << proc_pop << " received an interrupt" << std::endl;
                interruption_requested = true;
                interrupt_responce_pending = true;
            } else if (ThreadBoardInterface::isPrInterrupt2(sp_tsk[index])){
                ending_interrupt = true;
                MPI_Irecv(&sp_tsk[interrupt2_idx], 1, MPI_CCHAPEIRO_TASK_ID, UI_proc, interrupt2_tag, MPI_COMM_WORLD, &sp_req[interrupt2_idx]);
            }
        } while (true);
    }
    return 2;
}

void deactivateTask(Task * tsk){
    uint64_t mask = active_out_jobs;
    while (mask){
        unsigned int t = square(pop_lsb(mask));
        if (out_jobs[t].t == tsk){
            active_out_jobs ^= 1 << t;
            return;
        }
    }
    assert(false);
}

void sendWindowUpdates(){
    uint64_t mask = active_out_jobs;
    while (mask){
        unsigned int t = square(pop_lsb(mask));
        if (out_jobs[t].i >= job_max) continue;
        if (out_jobs[t].t->w->alpha != out_jobs[t].alpha || out_jobs[t].t->w->beta != out_jobs[t].beta){
            out_jobs[t].alpha = out_jobs[t].t->w->alpha;
            out_jobs[t].beta  = out_jobs[t].t->w->beta ;
            win_out.index = out_jobs[t].i;
            win_out.alpha = out_jobs[t].alpha;
            win_out.beta  = out_jobs[t].beta ;
            MPI_Send(&win_out, sizeof(window_update), MPI_BYTE, out_jobs[t].p, window_tag, MPI_COMM_WORLD);
        }
    }
}

void makeReceives(){
    do {
        bool flag = false;
        int index = 0;
        MPI_Status status;
        for (int i = 0 ; i < recv_pop; ++i) {
            flag = recv_res[i].test(&status);
            if (flag) {
                index = i;
                break;
            }
        }
        if (!flag) break;
        switch(status.MPI_TAG){
            case search_tag:{
                // std::cout <<  "Process " << std::setw(4) << proc_rank << "of" << proc_pop << ": Received Search!!!" << std::endl;
                //Process Search
                unsigned int ind = tbi->search_rind(&(incoming_search.b), MASTER_index, incoming_search.depth, incoming_search.alpha, incoming_search.beta, incoming_search.move);
                if(task_id(ind) != no_task){
                    comm_data[ind].proc_task_id = incoming_search.proc_task_id;
                    comm_data[ind].proc_source  = incoming_search.proc_source ;
                    comm_data[ind].pr_index     = incoming_search.pr_index    ;
                    assert(tbi->thr_dt[MASTER_index].used_tot & (1 << ind));
                    search_ack_ans              = ind;
                } else {
                    search_ack_ans              = job_max;
                    std::cout <<  "Process " << std::setw(4) << proc_rank << "of" << proc_pop << ": Denied Search" << std::endl;
                }
                //send acknowledgement
                MPI_Send(&search_ack_ans, 1, MPI_INT, incoming_search.proc_source, search_ack_tag, MPI_COMM_WORLD);
                recv_res[search_idx].irecv(&incoming_search, sizeof(incoming_search), MPI_BYTE, MPI_ANY_SOURCE, search_tag, MPI_COMM_WORLD);
                //End Process Search
                    // std::cout <<  "Process " << std::setw(4) << proc_rank << "of" << proc_pop << ": Opened Receive Search Channel" << std::endl;
                break;
            }
            case result_tag:{
                // std::cout <<  "Process " << std::setw(4) << proc_rank << "of" << proc_pop << ": Received Result!!!" << std::endl;
                //Process Result
                task_id t = result.proc_task_id;
                Task * tsk = &(tbi->thr_dt[t >> thrd_id_offset].tasks[t & thr_task_mask]);
                assert(tsk->st == Executing);
                tsk->score = result.score;
                tsk->st    = Completed;
                tbi->thr_dt[t >> thrd_id_offset].cmpl_arr[tsk->job_id] ^= 1 << (t & thr_task_mask);
                assert(tbi->thr_dt[t >> thrd_id_offset].cmpl_arr[tsk->job_id] & (1 << (t & thr_task_mask)));
                assert(out_jobs[result.pr_index].t == tsk);
                deactivateTask(tsk);

                //End Process Result
                if (--spawned_searches){
                    // std::cout <<  "Process " << std::setw(4) << proc_rank << "of" << proc_pop << ": Opened Receive result Channel" << std::endl;
                    recv_res[result_idx].irecv(&result         , sizeof(search_result  ), MPI_BYTE, MPI_ANY_SOURCE, result_tag, MPI_COMM_WORLD);
                }
                break;
            }
            case window_tag:{
                // std::cout <<  "Process " << std::setw(4) << proc_rank << "of" << proc_pop << ": Window update received (" << win_update.index << ")" << std::endl;
                unsigned int ind = win_update.index;
                if ((tbi->thr_dt[MASTER_index].used_tot & (1 << ind)) && 
                        (!outgoing_result_pending || 
                            out_result.proc_task_id != comm_data[ind].proc_task_id ||
                            out_result.pr_index     != comm_data[ind].pr_index)){
                    tbi->thr_dt[MASTER_index].job_id = ind;
                    tbi->updateWindows(win_update.alpha, win_update.beta, MASTER_index);
                }
                recv_res[window_idx].irecv(&win_update        , sizeof(window_update   ), MPI_BYTE, MPI_ANY_SOURCE, window_tag, MPI_COMM_WORLD);
                break;
            }
            default:{
                std::cout <<  "Process " << std::setw(4) << proc_rank << "of" << proc_pop << ": Invalid TAG!!! (" << index << ")" << std::endl;
                //assert(false);
                break;
            }
        }
    } while(false);
}

void makeSends(){
    if (outgoing_search_pending){
        int flag = 0;
        MPI_Testall(2, outgoing_search_req, &flag, MPI_STATUSES_IGNORE);
        outgoing_search_pending = !flag;
        if (flag){
            task_id t = outgoing_search.proc_task_id;
            Task * tsk = &(tbi->thr_dt[t >> thrd_id_offset].tasks[t & thr_task_mask]);
            assert(search_ack <= job_max);
            if (search_ack == job_max){ //Denied
                tsk->st = Pending;
                std::unique_lock<std::mutex> lk(tbi->pending_tasks_m);
                tbi->pending_tasks_queue.push_back(t);
                lk.unlock();
                tbi->pending_tasks_cv.notify_all();
                active_out_jobs ^= nt;
                assert(!(active_out_jobs & (UINT64_C(1) << square(nt))));
            } else { //Accepted
                delete tsk->board.exchange(NULL);
                out_jobs[square(nt)].i       = search_ack;
            }
        }
    }
    if (proc_rank != UI_proc && interrupt_responce_pending && !outgoing_search_pending){
        interrupt_responce_pending = false;
        task_id inter = ThreadBoardInterface::createPrInterrupt();
        MPI_Send(&inter , 1, MPI_CCHAPEIRO_TASK_ID, UI_proc, interrupt_tag, MPI_COMM_WORLD);
    }
    if (!(outgoing_search_pending || interruption_requested) && ~active_out_jobs){
        //find search to send to another process
        std::unique_lock<std::mutex> lk(tbi->pending_tasks_m);
        auto t_it = std::find_if(std::begin(tbi->pending_tasks_queue), 
                            std::end(tbi->pending_tasks_queue), 
                            [](const task_id &ti){
                                return !(
                                        ThreadBoardInterface::isBomb(ti)    || 
                                        ThreadBoardInterface::isMasters(ti) || 
                                        tbi->thr_dt[ti >> thrd_id_offset].tasks[ti & thr_task_mask].type == IterativeDeepening ||
                                        tbi->thr_dt[ti >> thrd_id_offset].tasks[ti & thr_task_mask].depth <= 3
                                        );
                            });
        if (t_it != std::end(tbi->pending_tasks_queue)){
        // if (tbi->idle_threads == 0 && !tbi->pending_tasks_queue.empty()){ //
            task_id t = *t_it;
            lk.unlock();
            Task * tsk = &(tbi->thr_dt[t >> thrd_id_offset].tasks[t & thr_task_mask]);
            // if (!(ThreadBoardInterface::isBomb(t) || ThreadBoardInterface::isMasters(t) || (tsk->type == IterativeDeepening))){
            //     tbi->pending_tasks_queue.pop_front();
                State t2 = Pending;
                if (tsk->st.compare_exchange_strong(t2, Executing)){
                    Board *brd                   = tsk->board;
                    memcpy(&(outgoing_search.b), brd, sizeof(Board));
                    outgoing_search.depth        = tsk->depth;
                    outgoing_search.alpha        = tsk->alpha;
                    outgoing_search.beta         = tsk->beta ;
                    outgoing_search.move         = tsk->move ;
                    outgoing_search.proc_source  = proc_rank;
                    outgoing_search.proc_task_id = t;
                    dest = rand()%(proc_pop-1);
                    if (dest == proc_rank) dest = proc_pop-1;
                    if (!spawned_searches){
                        // std::cout <<  "Process " << std::setw(4) << proc_rank << "of" << proc_pop << ": Opened Receive result Channel" << std::endl;
                        recv_res[result_idx].irecv(&result         , sizeof(search_result  ), MPI_BYTE, MPI_ANY_SOURCE, result_tag, MPI_COMM_WORLD);
                    }
                    // std::cout << "Process " << std::setw(4) << proc_rank << "of" << proc_pop << " sends search (" << spawned_searches << ") to " << dest << std::endl;
#ifndef NDEBUG
                    search_ack = job_max + 1;
#endif
                    nt = (~active_out_jobs) & -(~active_out_jobs);
                    active_out_jobs ^= nt;
                    int ind_nt       = square(nt);
                    out_jobs[ind_nt].t       = tsk;
                    out_jobs[ind_nt].alpha   = tsk->alpha;
                    out_jobs[ind_nt].beta    = tsk->beta ;
                    out_jobs[ind_nt].i       = job_max;
                    out_jobs[ind_nt].p       = dest;
                    assert(active_out_jobs & (UINT64_C(1) << ind_nt));
                    outgoing_search.pr_index = ind_nt;

                    MPI_Irecv(&search_ack, 1, MPI_INTEGER, dest, search_ack_tag, MPI_COMM_WORLD, &outgoing_search_req[1]);
                    MPI_Issend(&outgoing_search, sizeof(outgoing_search), MPI_BYTE, dest, search_tag, MPI_COMM_WORLD, &outgoing_search_req[0]);
                    outgoing_search_pending = true;
                    ++spawned_searches;
                }
            // } else {
            //     lk.unlock();
            // }
        } else {
            lk.unlock();
        }
    }
    if (outgoing_result_pending){
        int flag = 0;
        MPI_Test(&outgoing_result_req, &flag, MPI_STATUS_IGNORE);
        outgoing_result_pending = !flag;
    }
    task_id t;
    if (!outgoing_result_pending && (t = tbi->getCompletedTask(MASTER_index)) != no_task){
        Task * tsk = &(tbi->thr_dt[MASTER_index].tasks[t]);
        assert(tsk->st == Completed);
        out_result.score                = tsk->score;
        out_result.proc_task_id         = comm_data[t].proc_task_id;
        out_result.pr_index             = comm_data[t].pr_index;
        tsk->st                         = Invalid;
        // if (tbi->isFull(MASTER_index)){
        //     // std::cout <<  "Process " << std::setw(4) << proc_rank << "of" << proc_pop << ": Opened Receive Search Channel" << std::endl;
        //     //there is available task space, create slot to receive a new one
        //     recv_res[search_idx].irecv(&incoming_search, sizeof(incoming_search), MPI_BYTE, MPI_ANY_SOURCE, search_tag, MPI_COMM_WORLD);
        // }
        tbi->thr_dt[MASTER_index].used_tot ^= 1 << t;
        tbi->thr_dt[MASTER_index].used_arr[t] ^= 1 << t;
        tbi->thr_dt[MASTER_index].cmpl_arr[t] ^= 1 << t;
        // std::cout << "Process " << std::setw(4) << proc_rank << "of" << proc_pop << " sends results to " << comm_data[t].proc_source << std::endl;
        // std::cout <<  "Process " << std::setw(4) << proc_rank << "of" << proc_pop << ": Sends results (" << out_result.pr_index << ")" << std::endl;
        MPI_Issend(&out_result, sizeof(search_result), MPI_BYTE, comm_data[t].proc_source, result_tag, MPI_COMM_WORLD, &outgoing_result_req);
        outgoing_result_pending = true;
    }
    sendWindowUpdates();
}