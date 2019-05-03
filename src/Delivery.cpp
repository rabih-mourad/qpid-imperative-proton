#include <proton/imperative/Delivery.hpp>

#include <iostream>

using namespace proton;

void Delivery::accept()
{
   throwIfClosed();
   //using work queue from connection, if we use m_delivery->work_queue() it will crash randomly
   m_work->add([=]() {
      m_delivery->accept();
      removeDeliveryFromList(); 
   });
   
}

void Delivery::reject()
{
   throwIfClosed();
   //using work queue from connection, if we use m_delivery->work_queue() it will crash randomly
   m_work->add([=]() {
      m_delivery->reject();
      removeDeliveryFromList();
   });
}

void Delivery::release()
{
   throwIfClosed();
   //using work queue from connection, if we use m_delivery->work_queue() it will crash randomly
   m_work->add([=]() {
      m_delivery->release();
      removeDeliveryFromList();
   });
}

Delivery::Delivery(message m, work_queue* w, delivery* d, std::list<delivery>* deliveries, std::mutex* listLock)
   :pn_message(m), m_work(w), m_delivery(d), m_deliveries(deliveries), m_listLock(listLock)
{}

void Delivery::removeDeliveryFromList()
{
   std::lock_guard<std::mutex> l(*m_listLock);
   m_deliveries->remove(*m_delivery);
   m_closed = true;
}

void Delivery::throwIfClosed()
{
   if (m_closed)
      throw std::runtime_error("Error: Delivery state was already sent.");
}
