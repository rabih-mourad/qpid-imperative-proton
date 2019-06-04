# qpid-imperative-proton

This is a POC. We are trying to have an imperative Amqp C++ client based on Qpid proton which is a reactive API.


Work to be done:

- manage deliveries with auto ack
- managing error handling
- the close should return a future
- the sender should wait for a credit
- having a listener for the sender instead of only having a future
- implementing the synchronous receiver
- implementing the consumer with a listener
- adding a logger


Should be discussed in our next meeting:
The strategy was to make everything async even for the creation of the connection, session, receivers... but because of thread safety issues we can't:
- The return<connection> of container.connect is not thread safe, how should we proceed with the async API?
  For now i made it synchronous


Technical stuff:
- compiled with visual studio 2017
- C++11 standard is used
- unit tests use gtest commit 8fbf9d16a63
- qpid-proton 0.27.1
