// --------------------- driver.cpp -----------------------------------------
//
// Siqi Zheng, CSS 503
// Created:         May. 2, 2020
// Last Modified:   May. 12, 2020
// --------------------------------------------------------------
// Purpose: test shop's functionality. It creates the shop, the barbers and the clients. 
// Receives the following command line arguments, Instantiates a shop which is an object 
// from the Shop class that I've implemented. Spawns the nBarbers number of barber threads.
// Each individual thread gets passed a pointer to the shop object(shared), the unique identifier
// (i.e.  0 ~nBarbers ¨C 1), and serviceTime. Loops spawning nCustomers, waiting a random interval 
// in ? seconds, (i.e., usleep(rand() % 1000)) between each new customer being spawned.All customer 
// threads are passed in a pointer to the shop objectand the identifier(i.e., 1 ~nCustomers).
// then waits until all the customer threads are service and terminated.
// At last, Terminates all the barber threads.
// --------------------------------------------------------------
// Assumptions: All input data is assumed to be correctly formatted
// and valid. 
// ---------------------------------------------------------------------------

#include <iostream>    // cout
#include <sys/time.h> // get time of day
#include "shop.h"
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

using namespace std;

//pre-declare threads function
void *barber( void * );
void *customer( void * );

//used for a set of parameters to be passed to each thread 
class ThreadParam {
public:
    ThreadParam(Shop* shop, int id, int serviceTime) :
        shop(shop), id(id), serviceTime(serviceTime) { };

    Shop* shop;// a pointer to the Shop object
    int id;// a thread identifier
    int serviceTime;// service time (in usec) to a barber, whereas 0 to a customer 
};

int main( int argc, char *argv[] ) {
    if (argc != 5)
    {
        cout << "Please input nBarbers, nChairs, nCustomers, serviceTime." << endl;
        exit(0);
    }

    //Receives command line arguments
    int nBarbers = atoi(argv[1]);
    int nChairs = atoi(argv[2]);
    int nCustomers = atoi(argv[3]);
    int serviceTime = atoi(argv[4]);

    //Instantiates a shop object, customer threads array, barber threads array
    pthread_t barber_thread[nBarbers];
    pthread_t customer_threads[nCustomers];
    Shop shop(nBarbers, nChairs);

    //create barber threads
    for (int i = 0; i < nBarbers; i++)
    {
        int id = i;
        ThreadParam* param = new ThreadParam(&shop, i, serviceTime);
        pthread_create(&barber_thread[i], NULL, barber, (void*)param);
    }

    //create customer threads
    for (int i = 0; i < nCustomers; i++) {
        usleep(rand() % 1000);
        int id = i + 1;
        ThreadParam* param = new ThreadParam(&shop, i + 1, 0);
        pthread_create(&customer_threads[i], NULL, customer, (void*)param);
    }

    // wait for customer threads done
    for (int i = 0; i < nCustomers; i++)
        pthread_join(customer_threads[i], NULL);

    //terminate barber threads
    for (int i = 0; i < nBarbers; i++)
    {
        pthread_cancel(barber_thread[i]);
    }

    cout << "# customers who didn't receive a service = " << shop.nDropsOff
        << endl;

    return 0;
  
}

//barber thread function
void *barber( void *arg )
{
    // extract parameters
    ThreadParam& param = *(ThreadParam*)arg;
    Shop& shop = *(param.shop);
    int id = param.id;
    int serviceTime = param.serviceTime;
    delete& param;

    while (true)
    {
        shop.helloCustomer(id);
        usleep(serviceTime);
        shop.byeCustomer(id);
    }

    return NULL;  
}

//customer thread function
void *customer( void *arg ) 
{
    // extract parameters
    ThreadParam& param = *(ThreadParam*)arg;
    Shop& shop = *(param.shop);
    int id = param.id;
    delete& param;

    int barberId = shop.visitShop(id);
    if (barberId != -1)
        shop.leaveShop(id, barberId);

    return NULL;  
}