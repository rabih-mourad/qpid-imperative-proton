#include <proton/imperative/Container.hpp>
#include <proton/imperative/Connection.hpp>

#include <Constants.hpp>
#include <Broker.hpp>
#include <ProtonIncludes.hpp>

#include <iostream>

#include <gtest/gtest.h>

TEST(ContainerTest, constructorsWorkAsExpected)
{
   proton::Container cont;

   proton::Container copy(cont);
   proton::Container copyAssign;
   copyAssign = copy;

   proton::Container move(std::move(copy));
   proton::Container moveAssign;
   moveAssign = std::move(move);
}

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

TEST(ContainerTest, canCreateMultipleConnectionsFromContainer)
{
   Broker brk(url, destination);

   proton::Container cont;
   proton::Connection conn1 = cont.openConnection(url, proton::connection_options());
   proton::Connection conn2 = cont.openConnection(url, proton::connection_options());
   conn1.close();
   conn2.close();
   cont.close();
}

TEST(ContainerTest, ContainerCantCreateConnectionsAfterClose)
{
   proton::Container cont;
   cont.close();
   try {
      proton::Connection conn = cont.openConnection(url, proton::connection_options());
      FAIL() << TestShouldFailIfNoThrow;
   }
   catch (std::exception& e) {
      ASSERT_STREQ(ObjectClosedExceptionMsg.c_str(), e.what());
   }
}
