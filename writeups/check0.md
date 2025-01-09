Checkpoint 0 Writeup
====================

My name: Raymond Llata

My SUNet ID: rllata

I collaborated with: N/A

I would like to credit/thank these classmates for their help: N/A

This lab took me about [6] hours to do. I did not attend the lab session.

My secret code from section 2.1 was: 766565

I was surprised by or edified to learn that: An amazing Bytestream implementation could achieve 10.00 Gbit/s.

Describe ByteStream implementation. [Describe data structures and
approach taken. Describe alternative designs considered or tested.
Describe benefits and weaknesses of your design compared with
alternatives -- perhaps in terms of simplicity/complexity, risk of
bugs, asymptotic performance, empirical performance, required
implementation time and difficulty, and other factors. Include any
measurements if applicable.]

For my main data structure I only used a queue to store the bits, meaning information was being stored one char at a time. Aside from this, I permanently stored a string representing the current state of the queue to reference for quick peek times, storage complexity O(n). For pushing items I would add them to the queue, append a char to the string, and then mark up/down the respective int values O(1), for popping items I would pop them from queue, call substr to remove them from the peek string, and mark down the values O(n). All other functions simply called member variables to return an internal state, which runs in O(1). Asymptotically, Popping was definietly my weakest function, and I struggled to find a solution that would keep the peek values in internal memory, so that I could return them (See optional section). The implementation time took around 8 hours, but a large portion of that was spent fiddling with VScode. My code was being compiled with optimizations when debugging on VSCode (but not terminal GDB), and it ended up taking a while to figure out why.


- Optional: I had unexpected difficulty with: Setting up my VSCode debugger over SSH. I also had difficulty with the return values for peek. Since peek returns a string_view (not a string), it does not have access over it's internal memory; my original implementation returned a temporary string instead, which would turn to garbage as soon as the function ended. I ended up redoing my implementation of Bytestream because I did not realize peek was returning a string_view and not a string when I started.

My scores:
Test project /home/cs144/checkpoint0/minnow/build
      Start  1: compile with bug-checkers
 1/11 Test  #1: compile with bug-checkers ........   Passed    1.26 sec
      Start  2: t_webget
 2/11 Test  #2: t_webget .........................   Passed    0.70 sec
      Start  3: byte_stream_basics
 3/11 Test  #3: byte_stream_basics ...............   Passed    0.01 sec
      Start  4: byte_stream_capacity
 4/11 Test  #4: byte_stream_capacity .............   Passed    0.01 sec
      Start  5: byte_stream_one_write
 5/11 Test  #5: byte_stream_one_write ............   Passed    0.01 sec
      Start  6: byte_stream_two_writes
 6/11 Test  #6: byte_stream_two_writes ...........   Passed    0.01 sec
      Start  7: byte_stream_many_writes
 7/11 Test  #7: byte_stream_many_writes ..........   Passed    5.25 sec
      Start  8: byte_stream_stress_test
 8/11 Test  #8: byte_stream_stress_test ..........   Passed    0.67 sec
      Start 37: no_skip
 9/11 Test #37: no_skip ..........................   Passed    0.01 sec
      Start 38: compile with optimization
10/11 Test #38: compile with optimization ........   Passed    0.38 sec
      Start 39: byte_stream_speed_test
        ByteStream throughput (pop length 4096):  2.43 Gbit/s
        ByteStream throughput (pop length 128):   0.97 Gbit/s
        ByteStream throughput (pop length 32):    0.39 Gbit/s
11/11 Test #39: byte_stream_speed_test ...........   Passed    0.51 sec

100% tests passed, 0 tests failed out of 11

Total Test time (real) =   8.83 sec
Built target check0

- Optional: I think you could make this lab better by: [describe]

- Optional: I'm not sure about: What improvements I could obviously make to get to a higher gbips score.

- Optional: I contributed a new test case that catches a plausible bug
  not otherwise caught: [provide Pull Request URL]