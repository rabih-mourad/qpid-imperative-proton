# qpid-imperative-proton

This is a POC. We are trying to have an imperative Amqp C++ client based on Qpid proton which is a reactive API.

Work in progress:

- managing error handling
- enhancing implementation

Work to be done:

- the close should return a future
- protect the next action, For example sender.send should ensure that sender,open returned future was fulfilled
- having a listener for the sender instead of only having a future
- implementing the synchronous receiver
- implementing the consumer with a listener


Should be discussed in our next meeting:
- The new API proposition : to merge the open and the creation of the objects

Technical stuff:
- compiled with visual studio 2013 on windows 7
- C++11 standard is used
- unit tests use gtest