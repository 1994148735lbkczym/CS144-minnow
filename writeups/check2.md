Checkpoint 2 Writeup
====================

My name: Raymond Llata

My SUNet ID: rllata

I collaborated with: [list sunetids here]

I would like to thank/reward these classmates for their help: [list sunetids here]

This lab took me about [5] hours to do. I [did not] attend the lab session.

Describe Wrap32 and TCPReceiver structure and design. [Describe data
structures and approach taken. Describe alternative designs considered
or tested.  Describe benefits and weaknesses of your design compared
with alternatives -- perhaps in terms of simplicity/complexity, risk
of bugs, asymptotic performance, empirical performance, required
implementation time and difficulty, and other factors. Include any
measurements if applicable.]

Wrap32:

For wrap, I simply implement the formula ISN + zero_point % 2^32. This took one line, and runs in 0(1). I do not think there is a better approach.

I begin by converting the checkpoint uint64_t into a Wrap32, so that I may take advantage of the principle that the offset between checkpoint and actual the actual index is preserved. Then, I calculated the minimum distance from the curr index to checkpoint index and store it in the diff var. Next, I add and subtract diff from the index, to derive two potential locations that our index could be at, and check convert each of those locations to Wrap32's. Next, I compare the new Wrap32's to the initial Wrap to see if they are the same, and return the corresponding element. 

One design I considered and initially went with was calculating the absolute value of the difference. This passed many of the test cases, but would fail when I was subtracting a larger number from a smaller number (ie 0 - 1). Due to the overflow, I would get an inaccurate value that would result in picking the wrong Absolute Index for unwrap. 

In terms of simplicity, I do believe it would be hard to come up with a much simpler approach; My function is quite succinct, and makes sense intuitively once you understand offset is preserved. In terms of complexity, it runs in constant time and uses contant space. I do believe there are some marginal improvements that could be made (ie. create less variables), but nothing that would have a substantial impact on my code. This took me <2 hours, and most of the time was spent understanding the assignment handout.

TCPReceiver:

In terms of time/storage complexity, my TCPReciever also runs in constant time, I do not think there is a substantially better solution for this class then the one I have coded. My overall approach for receive() was to process each part of the message individually, and update the corresponding member variables. One problem I ran into was the behavior when the RST flag has been raised; I thought initially I could just return and not add to the Bytestream, but the test cases seemed like the correct behavior was to raise an error in the bytestream itself, which I ended up doing.

An alternate approach I considered was storing the Window Size in a local variable. I implemented this for a while, before I noticed that the window size was the same as the Reassmebler capacity, just with an upper limit. So, I removed all the logic I had updating my window_ variable, and used the min() standard function to return window_size in send.

I felt like this class was fairly straightforward, and there was not much need for a complex data structure that would heavily influence performance. This took me ~3 hours, much of which was parsing through test cases and stepping through the debugger to figure out what proper behavior was.

Implementation Challenges:
[I found some of the edge cases to not be well discussed in the Lab Handout/Lecture so I had to spend alot of time reading through test cases to derive the proper functionality.]

Test project /home/cs144/checkpoint0/minnow/build
      Start  1: compile with bug-checkers
 1/30 Test  #1: compile with bug-checkers ........   Passed    2.06 sec
      Start  3: byte_stream_basics
 2/30 Test  #3: byte_stream_basics ...............   Passed    0.01 sec
      Start  4: byte_stream_capacity
 3/30 Test  #4: byte_stream_capacity .............   Passed    0.01 sec
      Start  5: byte_stream_one_write
 4/30 Test  #5: byte_stream_one_write ............   Passed    0.01 sec
      Start  6: byte_stream_two_writes
 5/30 Test  #6: byte_stream_two_writes ...........   Passed    0.01 sec
      Start  7: byte_stream_many_writes
 6/30 Test  #7: byte_stream_many_writes ..........   Passed    0.06 sec
      Start  8: byte_stream_stress_test
 7/30 Test  #8: byte_stream_stress_test ..........   Passed    0.02 sec
      Start  9: reassembler_single
 8/30 Test  #9: reassembler_single ...............   Passed    0.01 sec
      Start 10: reassembler_cap
 9/30 Test #10: reassembler_cap ..................   Passed    0.01 sec
      Start 11: reassembler_seq
10/30 Test #11: reassembler_seq ..................   Passed    0.01 sec
      Start 12: reassembler_dup
11/30 Test #12: reassembler_dup ..................   Passed    0.03 sec
      Start 13: reassembler_holes
12/30 Test #13: reassembler_holes ................   Passed    0.01 sec
      Start 14: reassembler_overlapping
13/30 Test #14: reassembler_overlapping ..........   Passed    0.01 sec
      Start 15: reassembler_win
14/30 Test #15: reassembler_win ..................   Passed    6.63 sec
      Start 16: wrapping_integers_cmp
15/30 Test #16: wrapping_integers_cmp ............   Passed    0.01 sec
      Start 17: wrapping_integers_wrap
16/30 Test #17: wrapping_integers_wrap ...........   Passed    0.00 sec
      Start 18: wrapping_integers_unwrap
17/30 Test #18: wrapping_integers_unwrap .........   Passed    0.01 sec
      Start 19: wrapping_integers_roundtrip
18/30 Test #19: wrapping_integers_roundtrip ......   Passed    0.28 sec
      Start 20: wrapping_integers_extra
19/30 Test #20: wrapping_integers_extra ..........   Passed    0.22 sec
      Start 21: recv_connect
20/30 Test #21: recv_connect .....................   Passed    0.01 sec
      Start 22: recv_transmit
21/30 Test #22: recv_transmit ....................   Passed    0.25 sec
      Start 23: recv_window
22/30 Test #23: recv_window ......................   Passed    0.01 sec
      Start 24: recv_reorder
23/30 Test #24: recv_reorder .....................   Passed    0.01 sec
      Start 25: recv_reorder_more
24/30 Test #25: recv_reorder_more ................   Passed   12.83 sec
      Start 26: recv_close
25/30 Test #26: recv_close .......................   Passed    0.01 sec
      Start 27: recv_special
26/30 Test #27: recv_special .....................   Passed    0.04 sec
      Start 37: no_skip
27/30 Test #37: no_skip ..........................   Passed    0.00 sec
      Start 38: compile with optimization
28/30 Test #38: compile with optimization ........   Passed    2.41 sec
      Start 39: byte_stream_speed_test
        ByteStream throughput (pop length 4096):  2.29 Gbit/s
        ByteStream throughput (pop length 128):   0.97 Gbit/s
        ByteStream throughput (pop length 32):    0.37 Gbit/s
29/30 Test #39: byte_stream_speed_test ...........   Passed    0.52 sec
      Start 40: reassembler_speed_test
        Reassembler throughput (no overlap):   2.22 Gbit/s
        Reassembler throughput (10x overlap):  0.28 Gbit/s
30/30 Test #40: reassembler_speed_test ...........   Passed    1.20 sec

Remaining Bugs:
[None, I hope.]

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this lab better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]

- Optional: I made an extra test I think will be helpful in catching bugs: [describe where to find]
