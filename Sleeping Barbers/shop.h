// --------------------- shop.h -----------------------------------------
//
// Siqi Zheng, CSS 503
// Created:         May. 2, 2020
// Last Modified:   May. 12, 2020
// --------------------------------------------------------------
// Purpose: serve as a monitor, manage barbers and customers arrangment
// A barbershop consists of a waiting room with n waiting chairsand a barber 
// room with m barber chairs.If there are no customers to be served, all the 
// barbers go to sleep.If a customer enters the barbershopand all chairs
// (including both, waitingand barber chairs) are occupied, then the customer leaves the shop
// If all the barbers are busy but chairs are available, then the customer 
// sits in one of the free waiting chairs. If the barbers are asleep, 
// the customer wakes up one of the barbers. 
// --------------------------------------------------------------
// Assumptions: All input data is assumed to be correctly formatted
// and valid. 
// ---------------------------------------------------------------------------
#ifndef _SHOP_H_
#define _SHOP_H_
#include <pthread.h>
#include <queue>

using namespace std;

#define DEFAULT_CHAIRS 3
#define DEFAULT_BARBERS 1

class Shop {
 public:
     Shop(int nBarbers, int nChairs) :
         maxChairs((nChairs >= 0) ? nChairs : DEFAULT_CHAIRS),
         maxBarbers((nBarbers > 0) ? nBarbers : DEFAULT_BARBERS), nDropsOff(0)
     {
         init();
     };

     Shop() : maxChairs(DEFAULT_CHAIRS), maxBarbers(DEFAULT_BARBERS), nDropsOff(0)
     {
         init();
     };

     ~Shop();

     int visitShop(int id); // return barber's id when a customer got a service
     void leaveShop(int customerId, int barberId); //Is called by a customer thread to visit the shop. 
     void helloCustomer(int id);//Is called by a barber thread
     void byeCustomer(int id);//Is called by a barber thread
     int nDropsOff; // the number of customers dropped off

private:
    const int maxChairs; // the max number of threads that can wait
    const int maxBarbers;  // total number of barbers

    bool* in_service;
    bool* money_paid;

    queue<int> waiting_chairs;  // includes the ids of all waiting threads
    deque<int> freeBarbers;  // includs the ids of all free barbers
    vector<int> seats;  // service seats contain current serving custom id

    pthread_mutex_t mutex;//lock
    // conditions which can support different scenarios,  since there can be multiple barbers 
    //working on multiple customers at the same time, I use arrays of condition variables, 
    pthread_cond_t  cond_customers_waiting;
    pthread_cond_t* cond_customer_served;
    pthread_cond_t* cond_barber_paid;
    pthread_cond_t* cond_barber_sleeping;

    //utility functions
    void init();
    string int2string(int i);
    void print(int person, string message);
  
};

#endif