
#include <iostream>
#include <chrono>
#include <thread>
#include <random>
#include <deque>
#include <csignal>
#include <mutex>
#include <condition_variable>
#define BUFFER_LEN 10


using namespace std;

bool stopAll = false;

deque<int> buffer;

mutex mtx_monitor;
condition_variable monCond;

std::mt19937 mt(1000); //mersene twister random

void signalHandler(int sig)
{
    if(sig == SIGINT)
    {
        unique_lock<mutex> lock(mtx_monitor);
        cout << "  <--- received SIGINT..." << endl;
        stopAll = true;
    }
}

void producer_fun(int n)
{
    mtx_monitor.lock();
    cout << "Producer " << n << " generated." << endl;
    mtx_monitor.unlock();
    uniform_int_distribution<int> distrib(800, 1200);
    chrono::milliseconds duration(distrib(mt));
    uniform_int_distribution<int> distrib1(0,20); //producing random
    while(!stopAll)
    {
        int k = distrib1(mt);
        {
            unique_lock<mutex> lock(mtx_monitor);
            monCond.wait(lock, []{return buffer.size()<BUFFER_LEN || stopAll;});
            if(!stopAll)
            {
                cout << "Producer " << n << " produced " << k << "." << endl;
                buffer.push_back(k);
                monCond.notify_one();
            }
        }
        this_thread::sleep_for(duration);
    }
}

void consumer_fun(int n)
{
    mtx_monitor.lock();
    cout << "Consumer " << n << " generated." << endl;
    mtx_monitor.unlock();
    uniform_int_distribution<int> distrib(660, 2000);
    chrono::milliseconds duration(distrib(mt));
    while(!stopAll)
    {
        {
            unique_lock<mutex> lock(mtx_monitor);
            monCond.wait(lock, []{return buffer.size()>0 || stopAll;});
            if(!stopAll)
            {
                int k = buffer.front();
                buffer.pop_front();
                cout << "Consumer " << n << " consumed " << k << "." << endl;
                monCond.notify_one();
            }
        }
        this_thread::sleep_for(duration);
    }
}


int main()
{
    int n = thread::hardware_concurrency()/2;
    thread producers[n];
    thread consumers[n];
    uniform_int_distribution<int> distrib(1000, 2000);
    chrono::milliseconds duration(distrib(mt));
    int i;
    signal(SIGINT, signalHandler);
    cout << "Generating " << n << " consumers and producers..." << endl;
    for(i=0; i<n; i++)
    {
        consumers[i] = thread(consumer_fun, i);
        producers[i] = thread(producer_fun, i);
    }

    while(!stopAll)
    {
        mtx_monitor.lock();
        cout << "Buffer(" << buffer.size() << "): ";
        for(auto it=buffer.begin(); it!=buffer.end(); ++it)
            cout << *it << " ";
        cout << endl;
        mtx_monitor.unlock();
        this_thread::sleep_for(duration);
    };
    cout << "Start finishing program..." << endl;
    monCond.notify_all();

    for(i=0; i<n; i++)
    {
        producers[i].join();
        mtx_monitor.lock();
        cout << "Producer " << i << " joined." << endl;
        mtx_monitor.unlock();
        consumers[i].join();
        mtx_monitor.lock();
        cout << "Consumer " << i << " joined." << endl;
        mtx_monitor.unlock();
    }
    return 0;
}