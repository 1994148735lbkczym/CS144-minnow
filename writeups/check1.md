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


Describe Reassembler structure and design. [Describe data structures and
approach taken. Describe alternative designs considered or tested.
Describe benefits and weaknesses of your design compared with
alternatives -- perhaps in terms of simplicity/complexity, risk of
bugs, asymptotic performance, empirical performance, required
implementation time and difficulty, and other factors. Include any
measurements if applicable.]

I used a unordered_map to store information that could not be enqueued. This was because adding to the 
map would be O(1) (average) and accessing would also be O(1). I opted to store each char individually, which would make it easier to access when I was requeueing them. An alternative approach I considered was storing whole strings together, and I could just reattempt to add all strings when I needed to, but I figured storing strings vs individual chars was only saving a small amount of overhead (if any) and wasn't even worth it.

An alternative DS I considered was some sort of priority queue, because I could only add the next number in the queue, so some sort of sorted DS would be good for this objective. I decided against it for several reasons. PQ's have slower push + pop due to the bubble up algorithm O(logn). The convience of sorted order was unecessary since I more specifically could only add last_number+1, not just the most recent one. I am able to check for the existence of last_number+1 with an unordered_map with the same expected runtime as peak for a PQ, so the map seemed like the all around better choice here.


Implementation Challenges:
[Mac UTM default compiler made it so I couldnt pass reassembler_win for a while. Ed post #251 helped alot, thanks Keith!!]

Remaining Bugs:
[None (hopefully)]

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this lab better by: [describe]

- Optional: I'm not sure about: [describe]
