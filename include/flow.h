/*
	(C) Copyright Thierry Seegers 2010. Distributed under the following license:

	Boost Software License - Version 1.0 - August 17th, 2003

	Permission is hereby granted, free of charge, to any person or organization
	obtaining a copy of the software and accompanying documentation covered by
	this license (the "Software") to use, reproduce, display, distribute,
	execute, and transmit the Software, and to prepare derivative works of the
	Software, and to permit third-parties to whom the Software is furnished to
	do so, all subject to the following:

	The copyright notices in the Software and this entire statement, including
	the above license grant, this restriction and the following disclaimer,
	must be included in all copies of the Software, in whole or in part, and
	all derivative works of the Software, unless such copies or derivative
	works are solely in the form of machine-executable object code generated by
	a source language processor.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
	SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
	FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
	ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
	DEALINGS IN THE SOFTWARE.
*/

#if !defined(FLOW_FLOW_H)
	 #define FLOW_FLOW_H

#include "graph.h"
#include "named.h"
#include "node.h"
#include "packet.h"
#include "pipe.h"
#include "timer.h"

#endif

/*!
\file flow.h

\brief Convenience header that includes all necessary headers.

\mainpage flow

\li \ref introduction
\li \ref considerations
\li \ref principles
\li \ref improvements
\li \ref samples
\li \ref examples
\li \ref history
\li \ref license

\section introduction Introduction

flow is a headers-only <a href="http://en.wikipedia.org/wiki/C%2B%2B0x">C++0x</a> 
framework which provides the building blocks for streaming data packets through a graph 
of data-transforming nodes. 
Note that this library has nothing to do with computer networking. 
In the context of this framework, a data packet is essentially a slice of a data stream.

A \ref flow::graph will typically be composed of \ref flow::producer "producer nodes", \ref flow::transformer "transformer nodes" and 
\ref flow::consumer "consumer nodes".
Nodes are connected to one another by \ref flow::pipe "pipes" attached to their input and output \ref flow::pin "pins".
As a library user, you are expected to write concrete node classes that perform the tasks you require.
The graph and base node classes already provide the necessary API to build and run a graph.

Here's an example of a simple graph. 
The two producers nodes could be capturing data from some hardware or be generating a steady stream of data on their own. 
The transformer node processes the data coming in from both producers. 
The transformer's output data finally goes to a consumer node. 

\image html ./introduction_graph_simple.png "Data flow for a simple graph"

Should we need to monitor the data coming in from <tt>producer 2</tt>, we can \ref flow::samples::generic::tee "tee" it to another consumer node.
This new consumer node could save all the data it receives to a file or log it in real-time without preserving it.
The \ref flow::samples::generic::tee "tee" transformer node is a \ref samples "sample concrete node" that duplicates incoming data to all its outputs.

\image html ./introduction_graph_tee.png "Data flow for a graph with a tee transformer node"

\section considerations Technical considerations

This implementation:
 - uses templates heavily.
 - requires RTTI.
 - depends on the following C++0x's features:
	- auto keyword
	- lambda expression
	- r-value reference
	- move constructor and move function
	- unique_ptr and shared_ptr
 - depends on thirdparty libraries, see \ref thirdparty.
 - has been tested with VS2010 and gcc v. 4.5.2.

It is not required to use the included Visual Studio solution since this library is composed of headers only.
The solution's only purpose is to run the example code.
The solution expects the following user macros to be set 
(best set through <a href="http://blog.gockelhut.com/2009/11/visual-studio-2010-property-sheets-and.html">property sheets</a>):
 - ThirdpartyIncludeDir: the directory where thirdparty libraries' headers are located.
 - ThirdpartyLibDir: the directory where thirdparty libraries' binaries are located.

\section thirdparty Use of thirdparty libraries

 - <a href="http://www.boost.org/">boost</a>: the file graph.h uses the boost library's threading facilities. 
	The files packet.h and timer.h use boost's date_time library.
 - <a href="http://www.codeproject.com/KB/threads/lwsync.aspx">lwsync</a>: node.h 
	uses lwsync::critical_resource<T> and lwsync::monitor<T> to perform  resource 
	synchronization. This headers-only library internally uses boost::thread by default. The 
	included lwsync has been modified to add a 
	<a href="http://www2.research.att.com/~bs/C++0xFAQ.html#rval">move constructor</a> to 
	lwsync::critical_resource<T>.

\section principles Design principles

\subsection use_unique_ptr Use of std::unique_ptr

When flowing through the graph, \ref flow::packet "data packets" are wrapped in std::unique_ptr. 
This helps memory managment tremendously and enforces the idea that, at any point in time, 
only a single entity -pipe or node- is responsible for a data packet.

\subsection thread_per_node A thread per node

flow is multi-threaded in that the \ref flow::graph "graph" assigns a thread of execution to each of its nodes.
The lifetime of these threads is already taken care by \ref flow::graph "graph".
As a library user, the only mutli-threaded code you would write is whatever would go beyond graph management.

The thread is started by \ref flow::graph "graph" by passing a reference to the node object to boost::thread. 
Since the application passes a shared_ptr to a node to the graph, 
it is possible for the application to modify the node's state while it is running.
It must do so in a thread-safe manner.

\subsection consumption_time Consumption time

Consumption time is the time at which a data packet can be set to be consumed by a consumer node.
When a data packet with an assigned consumption time arrives at a consumer node and the consumption time is:
 - in the future: the consumer node waits or sleeps until the current time and the consumption time match, then consumes the packet.
 - in the past: the packet is unused and discarded.

Node that consumption time is optional. 
Data packets with no consumption time are consumed as soon as they reach a consumer node.

\subsection named_things Named building blocks

The \ref flow::node "node" base class derives from \ref flow::named named.
That makes all node concrete classes required to be given a name too.
This feature serves two purposes:
 - nodes can be refered to by their names when building a graph, improving code readability greatly.
 - helps debugging, especially since all pins and pipes are also named and have names derived from what they are connected to.

\section improvements Future improvements

 - I'll think of something. I'm sure.

\section samples Samples concrete nodes

As convenience, a collection of concrete nodes is provided. 
They are found in the \ref flow::samples::generic and \ref flow::samples::math namespaces.

\section examples Examples

 - \subpage hello_world
 - \subpage multiplier

\section history History

 Years ago I was part of a team developing audio software for an 
 embedded platform. This team was part of a larger group with its other 
 teams focused on other multimedia aspects.

 The software platform we used had a generic data streaming layer. 
 Since all the software was written in C, so was that layer. 
 At the time, it was evident to me that this kind of library could be 
 elegantly written in C++. Alas, suggesting that the entire group use a 
 different tool chain was out of the question. Suggesting that I, a young'un, 
 rewrite this critical and widely used layer was also a lost cause.

 Fast forward to now and I'm itching to learn to use C++0x just like I learned 
 C++ and I'm telling myslef: "Well, why the heck not!" I wanted to do it. 
 I still do. I will!

\section license License

\verbatim
Boost Software License - Version 1.0 - August 17th, 2003

Permission is hereby granted, free of charge, to any person or organization
obtaining a copy of the software and accompanying documentation covered by
this license (the "Software") to use, reproduce, display, distribute,
execute, and transmit the Software, and to prepare derivative works of the
Software, and to permit third-parties to whom the Software is furnished to
do so, all subject to the following:

The copyright notices in the Software and this entire statement, including
the above license grant, this restriction and the following disclaimer,
must be included in all copies of the Software, in whole or in part, and
all derivative works of the Software, unless such copies or derivative
works are solely in the form of machine-executable object code generated by
a source language processor.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
\endverbatim

*/

/*!

\page hello_world Hello, world!

In this example, we set up a graph with three \ref flow::samples::generic::generator "generators". 
Each of these generators operate on the same \ref flow::monotonous_timer "timer". 
Every time the timer fires, the generators produce a packet of data.
The data they produce is dictated by the functors given to them at construction time.
In this case, the functors are just functions that return strings.

The produced packets are then fed to an \ref flow::samples::math::adder "adder" transformer node. 
This adder uses operator+= internally. Thus, for strings, it concatenates its input.

Finally, the adder's output is connected to an \ref flow::samples::generic::ostreamer "ostreamer". 
This node simply streams the data packets it receives to a std::ostream of our choice, std::cout in this case.

\include hello_world.cpp
*/

/*!

\page multiplier Multiplication expression

In this example, we set up a graph with two \ref flow::samples::generic::generator "generators". 
Each of these generators operate on the same \ref flow::monotonous_timer "timer". 
Every time the timer fires, the generators produce a packet of data.
The data they produce is dictated by the functors given to them at construction time.
In this case, the functors are a reference to a random number generator.

The produced packets are then fed to a transformer defined locally.
This transformer takes its inputs (in terms of T), multiplies them, then outputs the multiplication expression including the product as a string.
For example, given the inputs of 3 and 4, it outputs the string "3 * 4 = 12".

Finally, the transformer's output is connected to an \ref flow::samples::generic::ostreamer "ostreamer". 
This node simply streams the data packets it receives to a std::ostream of our choice, std::cout in this case.

\include multiplier.cpp

Here's the output of a run of about 30 seconds:

\code
6 * 2 = 12
5 * 8 = 40
1 * 7 = 7
3 * 8 = 24
10 * 5 = 50
1 * 0 = 0
4 * 8 = 32
2 * 9 = 18
\endcode
*/
