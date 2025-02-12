Checkpoint 3 Writeup
====================

My name: Raymond Llata

My SUNet ID: rllata

I collaborated with: [list sunetids here]

I would like to thank/reward these classmates for their help: [list sunetids here]

This checkpoint took me about [n] hours to do. I [did not] attend the lab session.

Program Structure and Design of the TCPSender [Describe data
structures and approach taken. Describe alternative designs considered
or tested.  Describe benefits and weaknesses of your design compared
with alternatives -- perhaps in terms of simplicity/complexity, risk
of bugs, asymptotic performance, empirical performance, required
implementation time and difficulty, and other factors. Include any
measurements if applicable.]: 

I used a standard cpp map class to store outstanding bytes for the TCPSender. The other state variables I used for the assignment were constant space. The main benefit for this data structure was simplicity for my code. Insert/Get/Erase operations all take runtime O(logn), which I deemed fairly efficient. I picked this data structure when laying out the framework for processing messages. I wanted a quick way to loop through outstanding_segments, and remove the smallest elements first. Given that a standard map is implicitly sorted, this worked well for me. However, this was not the fast solution that I considered.

The alternate approach I had in mind was using two data structures; a Queue and a Unordered_Map. A stand alone unordered_map would not work primarily due to my looping through the keys -- when looping in an unordered_map the keys would iterate in a seemingly random order. So, the solution to this that I considered was including a queue of segment keys; This way instead of looping through the unordered_map, I could iterate through the queue, and recieve elements in order that way. This method would have faster average case runtime. Unordered_maps have O(1) average case Insert/Get/Erase. However, they also have worst case O(n) Insert/Get/Erase, which would have been substantially worse then my standard cpp map implementation. Furthermore, maintaining two data structures would have added alot more complexity to my code, especially in the recieve and push functions where I make alot of changes to my global state. Given the simplicity of my initial approach, and the fact that the complex approach did not necessairly provide better results, I decided to implement my TCPSender using a standard map.

My class functions were fairly straightfoward. For Tick() I simply coded the algorithm straight from the Assignment Spec. For Recieve() I added a helper function to update outstanding_segments_, and other global state variables. The logic gates in Recieve() were more or less taken straight from the assignment spec. Push() was my most complicated function. My approach was to create a helper, push_packet(), that when given the proper information, would be able to push every type of packet required. The logic in Push() mainly looks at the input_, and global state variables, and makes a decision as to what kind of packet to push. It will pass this information to push_packet(), which will create a Message and transmit it. I created many intermediate state variables for clarity and a better debugging experience.

Report from the hands-on component: [


]

Implementation Challenges:
[]

Remaining Bugs:
[]

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this lab better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]

- Optional: I made an extra test I think will be helpful in catching bugs: [describe where to find]
