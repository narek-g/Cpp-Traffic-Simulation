#include <iostream>
#include <random>
#include <chrono> 
#include <thread>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */


template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 
    std::lock_guard<std::mutex> ulock(_mutex); 
    _conditionVariable.wait(ulock, [this]{
        return !_queue.empty(); 
    });
    T message = std::move(_queue.back());
    _queue.emplace_back(); 
    return message; 
}


template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    std::lock_guard<std::mutex> lock(_mutex); 
    _queue.push_back(std::move(msg));
    _conditionVariable.notify_one();
}


/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    while(true){
        // sleep to prevent overclock 
        std::this_thread::sleep_for(std::chrono::milliseconds(1)); // this can also be longer duration
        if(_messageQueue.receive() == TrafficLightPhase::green){
            return; 
        }
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    /* FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread 
    when the public method „simulate“ is called. To do this, use the thread queue in the base class. 
    */ 
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. 
    // The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 

    // Random number gen 
    std::random_device dev;
    std::mt19937 rnd(dev()); 
    std::uniform_int_distribution<uint32_t> dist(4000,6000);
    float duration dist(rnd); 

    // std::random_device rd;
    // std::mt19937 gen(rd());
    // std::uniform_int_distribution<int> dist(4000, 6000);
    // int duration = dist(gen);

    // auto startTime = std::chrono::high_resolution_clock::now();
    auto startTime = std::chrono::system_clock::now();

    while(true){
        // sleep to prevent overclock 
        std::this_thread::sleep_for(std::chrono::milliseconds(1)); // this can also be longer duration

        // elapsed time check 
        auto elapsedTime = std::chrono::system_clock::now() - startTime ; 
        if(elapsedTime.count() >= duration){
            // update startTime to restart timer 
            startTime = std::chrono::system_clock::now();
            // flip traffic light phase
            if(_currentPhase == TrafficLightPhase::green){
                _currentPhase = TrafficLightPhase::red; 
            } else {
                _currentPhase = TrafficLightPhase::green;
            }
            // send update to _messageQueue
            TrafficLightPhase phase = _currentPhase; 
            _messageQueue.send(std::move(phase)); 
        } 
    } // close while(true) 
}

