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

Remaining Bugs:
[None, I hope.]

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this lab better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]

- Optional: I made an extra test I think will be helpful in catching bugs: [describe where to find]
