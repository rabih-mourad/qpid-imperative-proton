#ifndef PROTON_IMPERATIVE_DELIVERY_HPP
#define PROTON_IMPERATIVE_DELIVERY_HPP

#include <proton/message.hpp>
#include <proton/delivery.hpp>

#include <proton/imperative/config.hpp>
#include <proton/imperative/Receiver.hpp>

#include <list>

namespace proton {

class PROTON_IMPERATIVE_API Delivery
{
public:
   Delivery() = default;

   void accept();
   void reject();
   void release();

   message pn_message;
private:
   friend class Receiver;
   Delivery(message m, work_queue* w, delivery* d, std::list<delivery>* deliveries, std::mutex* listLock);
   void removeDeliveryFromList();
   void throwIfClosed();

   //the following params can't be reference because the future Api oblige us with a default constructor
   delivery* m_delivery;
   work_queue* m_work;
   std::list<delivery>* m_deliveries;
   std::mutex* m_listLock;
   bool m_closed = false;
};

}

#endif
