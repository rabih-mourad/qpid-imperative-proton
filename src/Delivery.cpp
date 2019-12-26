#include <proton/imperative/Delivery.hpp>

#include <iostream>

using namespace proton;

void Delivery::accept()
{
   m_manager->checkForExceptions();
   m_closeFuture = m_manager->close();
   //using work queue from connection, if we use m_delivery->work_queue() it will crash randomly
   m_work->add([=]() {
      m_delivery->accept();
      pnClose();
   });
}

void Delivery::reject()
{
   m_manager->checkForExceptions();
   m_closeFuture = m_manager->close();
   //using work queue from connection, if we use m_delivery->work_queue() it will crash randomly
   m_work->add([=]() {
      m_delivery->reject();
      pnClose();
   });
}

void Delivery::release()
{
   m_manager->checkForExceptions();
   m_closeFuture = m_manager->close();
   //using work queue from connection, if we use m_delivery->work_queue() it will crash randomly
   m_work->add([=]() {
      m_delivery->release();
      pnClose();
   });
}

proton::Delivery::~Delivery()
{
   if(!m_manager)
      return;
   
   if (m_manager->hasBeenClosed())
   {
      m_closeFuture.wait();
   }
   else
   {
      pnClose();
   }
}

Delivery::Delivery(message m, work_queue* w, delivery* d, std::list<delivery>* deliveries, std::mutex* listLock, CloseRegistry* receiverCloseHandler)
   :pn_message(m), m_work(w), m_delivery(d), m_deliveries(deliveries), m_listLock(listLock), m_manager(new PnObjectLifeManager(receiverCloseHandler, [&](const std::string&) {}))
{}

void Delivery::pnClose()
{
   m_manager->handlePnClose();
}


// Can't use it for now because the object is moved and the old address remains in the manager
void Delivery::releasePnObjects()
{
   std::lock_guard<std::mutex> l(*m_listLock);
   m_deliveries->remove(*m_delivery);
}
