#include <proton/connection.hpp>
#include <proton/container.hpp>

#include <proton/imperative/Container.hpp>
#include <proton/imperative/Connection.hpp>

#include <Broker.hpp>

#include <iostream>

#include <gtest/gtest.h>


TEST(ContainerTest, canCreateAndCloseContainer)
{
   proton::Container cont;
   cont.close();
}

TEST(ContainerTest, containerClosesCorrectlyOnDestruction)
{
   {
      proton::Container cont;
   }
   std::cout << "Destroyed correctly " << std::endl;
}

TEST(ContainerTest, canCreateConnectionFromContainer)
{
   Broker brk("//127.0.0.1:5672", "examples");

   proton::Container cont;
   proton::Connection conn = cont.openConnection("//127.0.0.1:5672", proton::connection_options());
   conn.close();
   cont.close();
}

TEST(ContainerTest, canCreateMultipleConnectionsFromContainer)
{
   Broker brk("//127.0.0.1:5672", "examples");

   proton::Container cont;
   proton::Connection conn1 = cont.openConnection("//127.0.0.1:5672", proton::connection_options());
   proton::Connection conn2 = cont.openConnection("//127.0.0.1:5672", proton::connection_options());
   conn1.close();
   conn2.close();
   cont.close();
}

TEST(ContainerTest, ContainerCantCreateConnectionsAfterClose)
{
   proton::Container cont;
   cont.close();
   try {
      proton::Connection conn = cont.openConnection("//127.0.0.1:5672", proton::connection_options());
      FAIL() << "Test should fail if it does not throw";
   }
   catch (std::exception& e) {
      std::cout << "Expected exception: " << e.what() << std::endl;
   }
}
