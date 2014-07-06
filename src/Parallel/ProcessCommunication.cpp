/**
 * ProcessCommunication.cpp
 *
 *  Created on: 2014/07/05
 *      Author: Chrysogelos Periklis
 */

#include "ProcessCommunication.hpp"
#include "mpi.h"
#include "../BoardInterface/BoardInterface.hpp"
#include "ThreadBoardInterface.hpp"

constexpr int UI_proc = 0;
int proc_pop  = 1;
int proc_rank = UI_proc;

//searches spawned to other processes 
unsigned int spawned_searches = 0;

struct search_message{
    Board                   b;          //thread id is invalid!!!
    TaskType                type;
    int                     depth;
    int                     alpha;
    int                     beta;
    int                     score;
    internal_move           move;
    State                   st;
    int                     proc_source;
    task_id                 proc_task_id;

    search_message(): move(0, 0), st(Invalid){}
} incoming_search, outgoing_search;

struct search_result{
    int                     score;
    int                     proc_task_id;
} result;

//special tasks, are task that can be identified only by their task id
//such as a bomb or an interrupt all
constexpr int special_tasks_pop = 2;
constexpr int interrupt_tag = 0;
constexpr int bomb_tag      = 1;
constexpr int interrupt_idx = 0;
constexpr int bomb_idx      = 1;
MPI_Request sp_req[special_tasks_pop];
// MPI_Status  sp_stt[special_tasks_pop];
task_id     sp_tsk[special_tasks_pop];

constexpr int recv_pop = 2;
constexpr int search_tag    = 2;
constexpr int search_idx    = 0;
constexpr int result_tag    = 3;
constexpr int result_idx    = 1;
MPI_Request recv_res[recv_pop];

ThreadBoardInterface* tbi = NULL;

void runProcessCommunicator(int argc, char* argv[]){
    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD, &proc_pop); //get running MPI processes
    MPI_Comm_rank(MPI_COMM_WORLD, &proc_rank);     //get current rank

    tbi             = new ThreadBoardInterface(!proc_rank);
    board_interface = tbi;

    //set buffer to wait for special tasks (bomb)
    if (proc_rank != UI_proc){
        MPI_Irecv(&sp_tsk[interrupt_idx], 1, MPI_CCHAPEIRO_TASK_ID, UI_proc, interrupt_tag, MPI_COMM_WORLD, &sp_req[interrupt_idx]);
        MPI_Irecv(&sp_tsk[bomb_idx     ], 1, MPI_CCHAPEIRO_TASK_ID, UI_proc, bomb_tag     , MPI_COMM_WORLD, &sp_req[bomb_idx     ]);
    }
    
    MPI_Irecv(&incoming_search, sizeof(incoming_search), MPI_BYTE, MPI_ANY_SOURCE, search_tag, MPI_COMM_WORLD, &recv_res[search_idx]);
    MPI_Irecv(&result         , sizeof(search_result  ), MPI_BYTE, MPI_ANY_SOURCE, result_tag, MPI_COMM_WORLD, &recv_res[result_idx]);

    while (true){
        if (!handleSpecialMessageTasks()) break;
        int flag  = 0;
        int index = 0;
        MPI_Status status;
        do {
            MPI_Testany(recv_pop, recv_res, &index, &flag, &status);
            if (!flag) break;
            switch(status.MPI_TAG){
                case search_tag:
                    std::cout <<  "Process " << std::setw(4) << rank << "of" << proc_pop << ": Received Search!!!" << std::endl;
                    //Process Search


                    //End Process Search
                    MPI_Irecv(&incoming_search, sizeof(incoming_search), MPI_BYTE, MPI_ANY_SOURCE, search_tag, MPI_COMM_WORLD, &recv_res[index]);
                    break;
                case result_tag:
                    std::cout <<  "Process " << std::setw(4) << rank << "of" << proc_pop << ": Received Result!!!" << std::endl;
                    //Process Result


                    //End Process Result
                    MPI_Irecv(&result         , sizeof(search_result  ), MPI_BYTE, MPI_ANY_SOURCE, search_tag, MPI_COMM_WORLD, &recv_res[index]);
                    break;
                default:
                    assert(false);
                    std::cout <<  "Process " << std::setw(4) << rank << "of" << proc_pop << ": Invalid TAG!!!" << std::endl;
                    break;
            }
        } while(false);
        //Send search positions and receive results
    }

    //FIXME if interrupt is not send before bomb, pending searches may exist, leading to a deadlock
    MPI_Cancel(&recv_res[search_idx]);
    MPI_Cancel(&recv_res[result_idx]);

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
        std::cout << "Process " << std::setw(4) << rank << "of" << proc_pop << " aborting" << std::endl;
    }
    MPI_Finalize();
}

void interrupt_all(){
    std::cout << "UI Process - Master Thread sends interrupts" << std::endl;
    task_id inter = ThreadBoardInterface::createPrInterrupt();
    MPI_Request reqs[proc_pop-1];
    for (int i = 1 ; i < proc_pop ; ++i){
        std::cout << "Sending Interrupt to process " << std::setw(4) << i << "of" << proc_pop << std::endl;
        MPI_Isend(&inter, 1, MPI_CCHAPEIRO_TASK_ID, i, interrupt_tag, MPI_COMM_WORLD, &reqs[i-1]);
    }
    std::cout << "UI Process - Master Thread waits for interrupts to get off" << std::endl;
    MPI_Waitall(proc_pop-1, reqs, MPI_STATUSES_IGNORE);
    std::cout << "UI Process - Master Thread sending interrupts completed" << std::endl;
}

int handleSpecialMessageTasks(){
    if (proc_rank == UI_proc){
        std::unique_lock<std::mutex> lk(tbi->pending_tasks_m);
        //Dispatch special tasks
        if (tbi->pending_tasks_cv.wait_for(lk, std::chrono::milliseconds(1), [](){
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
            tbi->pending_tasks_queue.pop();
            std::cout << "UI Process - Master Thread opens its eyes" << std::endl;
            std::cout << "UI Process - Master Thread wakes up" << std::endl;
            lk.unlock();

            //process task
            if (ThreadBoardInterface::isBomb(t)) return 0; //Only UI process
        } else {
            //time out
            if (interruption_requested && spawned_searches) { 
                //if there exist a working thread, its process is a 
                //distant child of this process in working tree
                interrupt_all();
                spawned_searches = 0;
            }
            lk.unlock(); //keep it until interrupts have been send 
            //this prevents starting a new search before other threads are
            //notified for the interrupt
            //NOTE recheck above idea
        }
    } else {
        //handle special tasks
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
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
                tbi->stop();
                std::cout << "Process " << std::setw(4) << proc_rank << "of" << proc_pop << " stopped its workers" << std::endl;
            }
        } while (true);
    }
    return 2;
}