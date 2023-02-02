
#include <iostream>
#include <cstring>
#include <chrono>
#include <thread>
#include <random>
#include <mutex>
#include <random>
#include <csignal>
#include <mutex>
#include <condition_variable>

using namespace std;

mutex mtx_monitor;
condition_variable cond_monitor;
bool stopAll = false;
bool start = false;

std::mt19937 mt(1000); //mersene twister random
string nazivi_instrumental[] ={"racunalom","tipkovnicom","monitorom"};
string nazivi_akuzativ[] = {"racunalo","tipkovnicu","monitor"};

void signalnaRucica(int sig);
void generiranje(int stol[2]);
bool uvjet_nakupac(int predmet, int stol[2]);
bool uvjet_veletrgovac(int stol[]);

void nakupac_fun(int i, int stol[2]);
void veletrgovac_fun(int stol[2]);

int main()
{
    thread t[4];
    int stol[2]={-1,-1};
    int i;
    signal(SIGINT, signalnaRucica);

    cout << "Generiram jednog veletrgovca." << endl;
    t[0] = thread(veletrgovac_fun, stol);
    cout << "Generiram 3 nakupca." << endl;
    for(i=1; i<=3; i++)
        t[i] = thread(nakupac_fun, i, stol);

    cout << "Cekam dreatve da zavrse." << endl;
    start = true;
    cond_monitor.notify_all();

    t[0].join();
    cout << "Join: Veletrgovac" << endl;
    for(i=1; i<=3; i++)
    {
        t[i].join();
        cout << "Join: nakupac " << i << endl;
    }
    return 0;
}

void signalnaRucica(int sig)
{
    if(sig == SIGINT)
    {
        unique_lock<mutex> lock(mtx_monitor);
        cout << "  <--- primljen SIGINT..." << endl;
        stopAll = true;
    }
    cond_monitor.notify_all();
}

void generiranje(int stol[2])
{
    uniform_int_distribution<int> distrib1(0,2);
    uniform_int_distribution<int> distrib2(1,2);
    stol[0] = distrib1(mt);
    stol[1] = (stol[0]+distrib2(mt))%3;
}


bool uvjet_nakupac(int predmet, int stol[2])
{
    return(((predmet+stol[0]+stol[1]) == 3 && start) || stopAll);
}
bool uvjet_veletrgovac(int stol[])
{
    return((stol[0]+stol[1]==-2 && start) || stopAll);
}

void nakupac_fun(int i, int stol[2])
{
    chrono::milliseconds duration(1000);
    int predmet = i-1;
    while(!stopAll)
    {
        {
            unique_lock<mutex> lock(mtx_monitor);
            cond_monitor.wait(lock, [&predmet, stol]{return uvjet_nakupac(predmet,stol);});
            if(!stopAll)
            {
                cout << "Nakupac " << i << " sa " << nazivi_instrumental[predmet] << 
                " uzeo je " << nazivi_akuzativ[stol[0]] << " i " << nazivi_akuzativ[stol[1]] << "." << endl;
                stol[0] = stol[1] = -1;
            }
            cond_monitor.notify_all();
        }
        this_thread::sleep_for(duration);
    }
}

void veletrgovac_fun(int stol[2])
{
    chrono::milliseconds duration(1000);
    while(!stopAll)
    {
        {
            unique_lock<mutex> lock(mtx_monitor);
            cond_monitor.wait(lock, [stol]{return uvjet_veletrgovac(stol);});
            if(!stopAll)
            {
                generiranje(stol);
                cout << "Veletrgovac je na stol stavio " << nazivi_akuzativ[stol[0]] << " i " << nazivi_akuzativ[stol[1]] << "." << endl;
            }
            cond_monitor.notify_all();
        }
        this_thread::sleep_for(duration);
    }
}