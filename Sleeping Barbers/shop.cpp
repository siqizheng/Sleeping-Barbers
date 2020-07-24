// --------------------- shop.cpp -----------------------------------------
//
// Siqi Zheng, CSS 503
// Created:         May. 2, 2020
// Last Modified:   May. 12, 2020
// --------------------------------------------------------------
// Purpose: serve as a monitor, manage barbers and customers arrangment. 
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


#include <iostream> // cout
#include <sstream>  // stringstream
#include <string>   // string
#include "shop.h"

//shop destructor used to clear up all the data siting on the heap
Shop::~Shop()
{
   pthread_cond_destroy(&this->cond_customers_waiting);

   for(int i = 0; i < this->maxBarbers; ++i)
   {
       pthread_cond_destroy(&this->cond_customer_served[i]);
       pthread_cond_destroy(&this->cond_barber_sleeping[i]);
       pthread_cond_destroy(&this->cond_barber_paid[i]);
   }
    
   pthread_mutex_destroy(&mutex);

   delete []in_service; 
   delete []money_paid;
}

//initiate all the condition varibales, mutex variable and containers used in this program
void Shop::init( ) 
{
    pthread_mutex_init( &mutex, NULL );
    pthread_cond_init( &cond_customers_waiting, NULL );

    this->cond_customer_served = new pthread_cond_t[this->maxBarbers];
    for(int i = 0; i < this->maxBarbers; ++i)
    {
        pthread_cond_init(&this->cond_customer_served[i], NULL);
    }

    this->cond_barber_sleeping = new pthread_cond_t[this->maxBarbers];
    for(int i = 0; i < this->maxBarbers; ++i)
    {
        pthread_cond_init(&this->cond_barber_sleeping[i], NULL);
    }

    this->cond_barber_paid = new pthread_cond_t[this->maxBarbers];
    for(int i = 0; i < this->maxBarbers; ++i)
    {
        pthread_cond_init(&this->cond_barber_paid[i], NULL);
    }


    seats.resize(maxBarbers, -1);

    in_service = new bool[maxBarbers];
    money_paid = new bool[maxBarbers];
}

//integer data converted to string
string Shop::int2string( int i ) 
{
    stringstream out;
    out << i;
    return out.str( );
}

//formatted printing
void Shop::print( int person, string message )
{
    cout << ( person > 0 ? "customer[" : "barber  [" )
         << (person > 0 ? person : 0 - person) << "]: " << message << endl;
}

//Is called by a customer thread to visit the shop
int Shop::visitShop(int customerId)
{
    pthread_mutex_lock(&mutex);   // lock

    bool waiting = false;

    // 1.waiting chairs > 0 and  are all occupied or 2. waiting chairs =0 and barbers are all working 
    if (waiting_chairs.size() == maxChairs&& (maxChairs > 0 || freeBarbers.empty()))  
    { 
        print(customerId, "leaves the shop because of no available waiting chairs.");
        ++nDropsOff;
        pthread_mutex_unlock(&mutex);
        return -1;                 // leave the shop
    }

    if (freeBarbers.empty())
    {
        waiting_chairs.push(customerId);    // have a waiting chair
        waiting = true;
        print(customerId, "takes a waiting chair. # waiting seats available = "
            + int2string(maxChairs - waiting_chairs.size()));
    }

    // someone is being served or transitting from a waiting to a service chair
    while (freeBarbers.empty()) 
    { 
        pthread_cond_wait(&cond_customers_waiting, &mutex);
    }

    int barberId = freeBarbers.front();

    freeBarbers.pop_front();

    print(customerId, "moves to the service chair[" + int2string(barberId) + "]. # waiting seats available = "
        + int2string(maxChairs - waiting_chairs.size()));

    if (waiting)
    {
        waiting_chairs.pop();        // stand up
    }

    in_service[barberId] = true;  // siting for barber to serve


    // wake up the barber just in case if he is sleeping
    pthread_cond_signal(&cond_barber_sleeping[barberId]);

    pthread_mutex_unlock(&mutex); // unlock

    return barberId;
}

//Is called by a customer thread to visit the shop.
void Shop::leaveShop( int customerId, int barberId ) 
{
    pthread_mutex_lock( &mutex );   // lock

    this->seats[barberId] = customerId;

    print(customerId, "wait for barber[" + int2string(barberId) + "] to be done with hair-cut" );
    pthread_cond_signal(&this->cond_barber_sleeping[barberId]);

    while (in_service[barberId] == true)// while being served
    {
        pthread_cond_wait(&cond_customer_served[barberId], &mutex);  // just sit.
    }
      
    money_paid[barberId] = true;//pay barber

    pthread_cond_signal( &cond_barber_paid[barberId] );
  
    print(customerId, "says good-bye to the barber["+ int2string(barberId)+"]" );

    pthread_mutex_unlock( &mutex ); // unlock
}

//Is called by a barber thread
void Shop::helloCustomer(int barberId)
{
    pthread_mutex_lock( &mutex );   // lock

    //If I have no customer and all the waiting chairs are empty
    if(seats[barberId] == -1 && waiting_chairs.empty())
    {
        bool f = false; 
        //check if I'm in the freeBarbers queue
        for(deque<int>::iterator it = freeBarbers.begin(); it != freeBarbers.end(); ++it) //check whether the barber thread free 
        {
            if (*it == barberId)
            {
                f = true;
                break;
            }
        }
        //if I'm not in the freeBarbers queue, add me in
        if (!f)
        {
            this->freeBarbers.push_back(barberId);
        }

        print( 0 - barberId, "sleeps because of no customers." );

        pthread_cond_wait( &cond_barber_sleeping[barberId], &mutex ); // then, let's sleep
    }

    if ( seats[barberId] == -1)
    {  
        pthread_cond_wait( &cond_barber_sleeping[barberId], &mutex ); 
    }
    print( 0 - barberId,
    "starts a hair-cut service for customer[" + int2string( seats[barberId] ) + "]" );

    pthread_mutex_unlock( &mutex );  // unlock
}

//Is called by a barber thread
void Shop::byeCustomer( int id) 
{
    pthread_mutex_lock( &mutex );    // lock

    in_service[id] = false;
    print( 0 - id, "says he's done with a hair-cut service for a customer[" + 
	   int2string( seats[id] ) + "]");

    money_paid[id] = false;
    pthread_cond_signal( &cond_customer_served[id] );   // tell the customer "done"

    while (money_paid[id] == false)
    {
        pthread_cond_wait(&cond_barber_paid[id], &mutex);
    }
      
    seats[id] = -1;  //empty service chair
    freeBarbers.push_back(id);  // back to free status

    print( 0 - id, "calls in another customer" );
    pthread_cond_signal( &cond_customers_waiting ); // call in another one

    pthread_mutex_unlock( &mutex );  // unlock
}