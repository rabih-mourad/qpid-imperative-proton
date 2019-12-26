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
   void accept();
   void reject();
   void release();

   ~Delivery();
   Delivery() = default;
   Delivery(Delivery&& other) = default;
   Delivery& operator=(Delivery&& other) = default;
   Delivery(const Delivery&) = delete;
   Delivery& operator=(const Delivery&) = delete;

   message pn_message;

private:
   friend class Receiver;
   Delivery(message m, work_queue* w, delivery* d, std::list<delivery>* deliveries, std::mutex* listLock, CloseRegistry* receiverCloseHandler);
   void pnClose();
   // Can't use it for now because the object is moved and the old address remains in the manager
   void releasePnObjects();

   //the following params can't be reference because the future Api oblige us with a default constructor
   delivery* m_delivery;
   work_queue* m_work;
   std::list<delivery>* m_deliveries;
   std::mutex* m_listLock;
   std::future<void> m_closeFuture;
   std::unique_ptr<PnObjectLifeManager> m_manager;
};

}

#endif
