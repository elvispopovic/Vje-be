
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
bool start = false;

deque<int> buffer;

mutex mtx_monitor;
condition_variable monCond;

std::mt19937 mt(1000); //mersene twister random

void signalHandler(int sig);
void producer_fun(int n);
void consumer_fun(int n);

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
    start = false;
    for(i=0; i<n; i++)
    {
        consumers[i] = thread(consumer_fun, i);
        cout << "Consumer " << i << " created." << endl;
        producers[i] = thread(producer_fun, i);
        cout << "Producer " << i << " created." << endl;
    }
    start = true;
    monCond.notify_all();

    while(!stopAll)
    {
        {
            unique_lock<mutex> lock(mtx_monitor);
            cout << "Buffer(" << buffer.size() << "): ";
            for(auto it=buffer.begin(); it!=buffer.end(); ++it)
                cout << *it << " ";
            cout << endl;
        }
        this_thread::sleep_for(duration);
    };
    cout << "Start finishing program..." << endl;
    monCond.notify_all();

    for(i=0; i<n; i++)
    {
        producers[i].join();
        cout << "Producer " << i << " joined." << endl;
        consumers[i].join();
        cout << "Consumer " << i << " joined." << endl;
    }
    return 0;
}

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
    uniform_int_distribution<int> distrib(800, 1200);
    chrono::milliseconds duration(distrib(mt));
    uniform_int_distribution<int> distrib1(0,20); //producing random
    while(!stopAll)
    {
        int k = distrib1(mt);
        {
            unique_lock<mutex> lock(mtx_monitor);
            monCond.wait(lock, []{return (buffer.size()<BUFFER_LEN || stopAll) && start;});
            if(!stopAll)
            {
                cout << "Producer " << n << " produced " << k << "." << endl;
                buffer.push_back(k);
            }
            monCond.notify_one();
        }
        this_thread::sleep_for(duration);
    }
}

void consumer_fun(int n)
{
    uniform_int_distribution<int> distrib(660, 2000);
    chrono::milliseconds duration(distrib(mt));
    while(!stopAll)
    {
        {
            unique_lock<mutex> lock(mtx_monitor);
            monCond.wait(lock, []{return (buffer.size()>0 || stopAll) && start;});
            if(!stopAll)
            {
                int k = buffer.front();
                buffer.pop_front();
                cout << "Consumer " << n << " consumed " << k << "." << endl;
            }
            monCond.notify_one();
        }
        this_thread::sleep_for(duration);
    }
}