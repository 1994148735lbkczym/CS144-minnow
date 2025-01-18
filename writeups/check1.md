Checkpoint 1 Writeup
====================

My name: Raymond Llata

My SUNet ID: rllata

I collaborated with: kyles7

I would like to thank/reward these classmates for their help: [list sunetids here]

This lab took me about [6] hours to do. I [did] attend the lab session.

I was surprised by or edified to learn that: [The Reassembler is releveant to the actual internet and TCP abstraction as discussed in 1/17 lecture]

Report from the hands-on component of the lab checkpoint: [include
information from 2.1(4), and report on your experience in 2.2]

Average Time Taken: 11.2 ms
1676 packets transmitted, 1676 received, 0% packet loss, time 339811ms
rtt min/avg/max/mdev = 6.076/11.200/126.409/9.767 ms
0% packet loss?
Did not see some DUP messages
Yes, saw Source/DST/TTL and a few others.
The TTL is different and requests say reply and vice versa.

In order to recieve a datagram from my partner I needed to use the code snippet from lecture but replace Address{ "1" } with our
target IP address. We would not recieve the datagram otherwise. Sending UDP datagrams allowed us to view the message locally, for some
raw datagrams (protocol 5) we would not be able to view the contents. We later resolved this using the -v (verbose) flag on the tcpdump
function.


Describe Reassembler structure and design. [Describe data structures and
approach taken. Describe alternative designs considered or tested.
Describe benefits and weaknesses of your design compared with
alternatives -- perhaps in terms of simplicity/complexity, risk of
bugs, asymptotic performance, empirical performance, required
implementation time and difficulty, and other factors. Include any
measurements if applicable.]

I used a unordered_map to store information that could not be enqueued. This was because adding to the 
map would be O(1) (average) and accessing would also be O(1). I opted to store each char individually, 
which would make it easier to access when I was requeueing them (I would be able to search for an index
without getting creative in finding an index within a string). 

An alternative approach I considered was 
storing whole strings together (using a unordered_map<int, string> instead of unordered_map<int, char>), 
and I could just reattempt to add all strings when I needed to using some sort of complex indexing math. I initially thought
about this approach to save storage space (some of the test cases store alot of data), but I soon realized that storing the chars
individually vs storing a string does not actually save that much overhead. I guess it would have reduced the number of collisions
in my unordered_map, but worst case get()/set() time would be O(logn), which isn't too bad. I decided the simplicity was more worth the 
potentially slower runtime. Generally, it is my understanding that I would not have to deal with collisions in my map much unless I was storing
thousands of thousands of keys.

An alternative DS I considered was some sort of priority queue, because I could only add the next number in the queue, 
so some sort of sorted DS would be good for this objective. I decided against it for several reasons: PQ's have slower 
push + pop due to the bubble up algorithm O(logn) expected time. Furthermore, the convience of sorted order was unecessary since I more specifically 
could only add the index last_number+1, not the most recent index in general. Using an unordered_map I am able to check for the existence of last_number+1 with an 
unordered_map with the same expected runtime as peak for a PQ, so the map seemed like the all around better choice here - in terms of runtime, and simplicity. I do 
believe a PQ could have been simpler when implementing some parts of the reassembler (such as restoring the Bytestream from my saved state). Instead of saving indexes
I could simply peek and pop on a loop to readd. One challenge it would pose is it would not do well with duplicates, so I would need to find a quick way of 
checking if a item already exists. An unordered_map excels at this O(1) runtime.

17/18 Test #39: byte_stream_speed_test ...........   Passed    0.53 sec
      Start 40: reassembler_speed_test
        Reassembler throughput (no overlap):   2.23 Gbit/s
        Reassembler throughput (10x overlap):  0.35 Gbit/s
18/18 Test #40: reassembler_speed_test ...........   Passed    0.99 sec


Implementation Challenges:
[Mac UTM default compiler made it so I couldnt pass reassembler_win for a while. Ed post #251 helped alot, thanks Keith!!
I struggled also in understanding when the Reassembler could close the connection. I felt that the assignment handout was vague in this (as was the header file),
and the way I went about this was really just running the test cases and studying them to derive what correct behavior would be.]

Remaining Bugs:
[None (hopefully)]

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this lab better by: [describe]

- Optional: I'm not sure about: [describe]
