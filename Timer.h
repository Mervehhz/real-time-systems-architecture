#include "ITimer.h"
#include <thread>
#include <iostream>
#include <unordered_map>
#include <mutex> 
#include <atomic>

namespace mervesTimer{

    class Timer : public ITimer{
        public:

        struct event{
            CLOCK::time_point clk;
            TTimerCallback callback;
            bool is_periodic = false;
            Millisecs period;
            const TPredicate* predicate = nullptr;
            Timepoint until;
            bool is_until_set = false;
        };

        Timer(){
            
        }

        ~Timer(){
            timer_thread_.detach();
        }

        long timepoint_to_long(const Timepoint& tp){
            auto tp_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(tp);
            return std::chrono::duration_cast<std::chrono::milliseconds>(tp_ms.time_since_epoch()).count();
        }

        // run the callback once at time point tp.
        void registerTimer(const Timepoint& tp, const TTimerCallback& cb){

            std::unique_lock<std::mutex> lock(mtx);
            event e;
            e.clk = tp;
            e.callback = cb;
            map[timepoint_to_long(tp)].push_back(e);
        }

        // run the callback periodically forever. The first call will be executed after the first period.
        void registerTimer(const Millisecs& period, const TTimerCallback& cb){

            std::unique_lock<std::mutex> lock(mtx);
            event e;
            e.clk = CLOCK::now()+period;
            e.is_periodic = true;
            e.period = period;
            e.callback = cb;
            map[timepoint_to_long(e.clk)].push_back(e);
        }
        
        // Run the callback periodically until time point tp. The first call will be executed after the first period.
        void registerTimer(const Timepoint& tp, const Millisecs& period, const TTimerCallback& cb){

            std::unique_lock<std::mutex> lock(mtx);
            event e;
            e.clk = CLOCK::now()+period;
            e.is_periodic = true;
            e.period = period;
            e.callback = cb;
            e.until = tp;
            e.is_until_set = true;
            map[timepoint_to_long(e.clk)].push_back(e);
        
        }

        // Run the callback periodically. After calling the callback every time, call the predicate to check if the
        //termination criterion is satisfied. If the predicate returns false, stop calling the callback.
        void registerTimer(const TPredicate& pred, const Millisecs& period, const TTimerCallback& cb){

            std::unique_lock<std::mutex> lock(mtx);
            event e;

            e.clk = CLOCK::now()+period;
            e.callback = cb;
            e.is_periodic = true;
            e.period = period;
            e.predicate = &pred;
            map[timepoint_to_long(e.clk)].push_back(e);
        }
    

        private:
            std::thread timer_thread_ = std::thread(&Timer::loop, this);
            std::mutex mtx;
            std::unordered_map<long, std::vector<event>> map;

            void loop(){
                
                for(;;){
                    for(event e : map[timepoint_to_long(CLOCK::now())]){
                        std::unique_lock<std::mutex>mtx;
                        e.callback();
                        if(e.is_periodic){
                            if(e.is_until_set && e.until < CLOCK::now()+e.period){
                                continue;
                            }
                            if(e.predicate != nullptr && !(*e.predicate)()){
                                continue;
                            }
                            map[timepoint_to_long(CLOCK::now()+e.period)].push_back(e);
                        }
                    }
                    map[timepoint_to_long(CLOCK::now())].clear();
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }
            }
    };
}