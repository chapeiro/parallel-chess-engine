/**
 * TimeManager.hpp
 *
 *  Created on: 2014/06/17
 *      Author: Chrysogelos Periklis
 */

#ifndef TIMEMANAGER_HPP_
#define TIMEMANAGER_HPP_
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include "../cchapeiro.hpp"

enum color{
    WHITE = 0,
    BLACK = 1
};

constexpr color toggle(const color a){
    return static_cast<color>(a ^ 1);
}

struct player_time_control{
    std::chrono::milliseconds remaining;
    std::chrono::milliseconds increment;
};

struct time_control{
    player_time_control whitep;
    player_time_control blackp;
    unsigned int movestogo;
    std::chrono::milliseconds search_time;
    bool auto_search_time;
    bool infinite_search;
};


template<class Clk = std::chrono::high_resolution_clock>
class TimeManager{
public:
    time_control tc;

private:
    std::chrono::time_point<Clk> search_start;
    std::chrono::time_point<Clk> search_lap;
    std::chrono::time_point<Clk> search_end;
    color player;
    std::mutex m;
    std::condition_variable cv;
    std::thread* alarm;
    std::atomic<bool> *interrupt_flag;
    int alarm_state;

public:
    TimeManager(time_control tc, color player, std::atomic<bool>* inter_flag): 
            tc(tc), player(player), interrupt_flag(inter_flag), alarm_state(0){
        *inter_flag = false;
        search_start = Clk::now();
        search_lap   = search_start;
        std::chrono::nanoseconds search_duration;
        if (tc.auto_search_time){
            int movestogo = tc.movestogo;

            if (movestogo == NO_NEXT_TIME_CONTROL) movestogo = 40;
            if (movestogo < 4) movestogo = 4;

            search_duration = (player == WHITE) ? 
                                        tc.whitep.remaining:tc.blackp.remaining;
            search_duration /= movestogo;
        } else {
            search_duration = tc.search_time;
        }
        search_end = search_start + search_duration;

        if (!tc.infinite_search){
            std::unique_lock<std::mutex> lk(m);
            alarm = new std::thread(&TimeManager::clock_alarm, this);
            cv.wait(lk, [this](){return this->alarm_state;});
            lk.unlock();
        } else {
            alarm = NULL;
        }
    }

    ~TimeManager(){
        if (alarm){
            abort_search();
            alarm->join();
            delete alarm;
        }
    }

    bool continue_search(float elapse_time_factor){
        if (    tc.infinite_search    ) return true;
        if (search_lap <= search_start) return true;
        std::chrono::nanoseconds elapsed_time = search_lap - search_start;
        return (search_end - Clk::now()) > (elapsed_time*elapse_time_factor);
    }

    void search_lap_tick(){
        search_lap = Clk::now();
    }

    std::chrono::milliseconds getElapsedTime(){
        return std::chrono::duration_cast<std::chrono::milliseconds>(Clk::now()-search_start);
    }

    void abort_search(){
        //call only after construction has been completed
        alarm_state = 2;
        cv.notify_all();
    }

private:
    void clock_alarm(){
        std::unique_lock<std::mutex> lk(m);
        alarm_state = 1;
        cv.notify_all();
        cv.wait_until(lk, search_end, [this](){return this->alarm_state > 1;});
        if (interrupt_flag) *interrupt_flag = true;
        lk.unlock();
    }
};

#endif /* TIMEMANAGER_HPP */