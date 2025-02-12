Checkpoint 3 Writeup
====================

My name: Raymond Llata

My SUNet ID: rllata

I collaborated with: [janlrudo]

I would like to thank/reward these classmates for their help: [list sunetids here]

This checkpoint took me about [10-13] hours to do. I [did not] attend the lab session.

Program Structure and Design of the TCPSender [Describe data
structures and approach taken. Describe alternative designs considered
or tested.  Describe benefits and weaknesses of your design compared
with alternatives -- perhaps in terms of simplicity/complexity, risk
of bugs, asymptotic performance, empirical performance, required
implementation time and difficulty, and other factors. Include any
measurements if applicable.]: 

I used a standard cpp map class to store outstanding bytes for the TCPSender. The other state variables I used for the assignment were constant space. The main benefit for this data structure was simplicity for my code. Insert/Get/Erase operations all take runtime O(logn), which I deemed fairly efficient. I picked this data structure when laying out the framework for processing messages. I wanted a quick way to loop through outstanding_segments, and remove the smallest elements first. Given that a standard map is implicitly sorted, this worked well for me. However, this was not the fast solution that I considered.

The alternate approach I had in mind was using two data structures; a Queue and a Unordered_Map. A stand alone unordered_map would not work primarily due to my looping through the keys -- when looping in an unordered_map the keys would iterate in a seemingly random order. So, the solution to this that I considered was including a queue of segment keys; This way instead of looping through the unordered_map, I could iterate through the queue, and recieve elements in order that way. This method would have faster average case runtime. Unordered_maps have O(1) average case Insert/Get/Erase. However, they also have worst case O(n) Insert/Get/Erase, which would have been substantially worse then my standard cpp map implementation. Furthermore, maintaining two data structures would have added alot more complexity to my code, especially in the recieve and push functions where I make alot of changes to my global state. Given the simplicity of my initial approach, and the fact that the complex approach did not necessairly provide better results, I decided to implement my TCPSender using a standard map.

Edit: I have now realized I could have just used a single Queue since I was leveraging the sorted order of a map. This approach would have added quicker runtimes and greater simplicity, and does not have much downside, as far as I can tell.

My class functions were fairly straightfoward. For Tick() I simply coded the algorithm straight from the Assignment Spec. For Recieve() I added a helper function to update outstanding_segments_, and other global state variables. The logic gates in Recieve() were more or less taken straight from the assignment spec. Push() was my most complicated function. My approach was to create a helper, push_packet(), that when given the proper information, would be able to push every type of packet required. The logic in Push() mainly looks at the input_, and global state variables, and makes a decision as to what kind of packet to push. It will pass this information to push_packet(), which will create a Message and transmit it. I created many intermediate state variables for clarity and a better debugging experience.

Report from the hands-on component: [

I was able to send bytes to and forth my lab partner, as well as pass the one-megabyte challenge. Our SHA256 Checksum's matched, and we found similar success vice versa. The largest amount of bytes we had the patience to wait for was 100 Megabytes (or 100000000 Bytes). For this, I was the client, and janlrudo was the server. We did not attempt any higher because of time constraints. I have included the output from ls -l and sha-256 below.

My SHA256 Checksum:
2cb457f7c3aa3e1e031b0012673de0d6fa78631f1b9d9a86c81d2f74bd8960e4
Their SHA256 Checksum:
2cb457f7c3aa3e1e031b0012673de0d6fa78631f1b9d9a86c81d2f74bd8960e4
My ls -l output:
-rw-rw-r-- 1 cs144 cs144 100000000 Feb 12 02:49 /tmp/big-received.txt
Their ls -l output:
-rw-rw-r-- 1 owlyfeather6 owlyfeather6 100000000 Feb 12 02:44 /tmp/biggest.txt

One thing I found interesting was the time sending larger files took. I was under the impression that the time it took to send larger files would increase linearly. (As it seemed to me that link speeds were always constant, data rate was always constant, and queue sizes / paths did not change). However, in practice it seemed that the larger files we tried to send, the longer the time would take. For example, the one Megabyte challenge took around a second, but 10 Megabytes took us around 35 seconds (as opposed to 10 seconds like I thought), and 100 Megabytes took us over five minutes (I did not keep track of exactly how long). My hypothesis for why this is is that as for larger file sizes the queues potentially fill up faster and we end up losing more bytes, but even then, as per the data rate graphs we have seen in class I do not understand why this results in a non-linear time increase.
]

Implementation Challenges:
[I struggled with understanding the behavior of the TCP Sender. I think that alot of the functionality was supposed to be intuitively understood without being listed on the Assignment Spec, so it was difficult for me to deduce how to implement it. 

For example, behavior with a RST Flag was not really discussed in the handout. Intuitively, I gleaned that it would be a good idea to communicate a RST Flag from Sender/Reciever ASAP, but it was unclear if it was supposed to be in a packet by itself. Another example was the SYN Flag. In class, we covered that a SYN Flag can be sent by itself or with a larger packet, but on the Assignment Handout it only explicitly mentions how to handle a SYN Flag-only packet, with no payload. This led me to believe the TCP Sender I was implementing was always initializing it's link with a reciever first. And so I proceeded like this through the first few test cases, debugging as I needed, until I found a test case were the Reciever was reaching out first, which my code did not handle. I think, for me, these were the greatest challenges. The Retransmit Timer logic was simple enough, and most of the remaining difficulty for the assignment was figuring out how to implement the functionality I want with as little state-keeping as possible.

I also struggled with the hands-on component, as I forgot to run commands to link my computer to the webserver / make ipv4 properly.]

Remaining Bugs:
[N/A]

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this lab better by: [I think it would be good to direct us to possible relevant commands that are preventing us from succesfully running ipv4. (ie. ensure we are running make properly / reminding us to reconnect to the webserver)]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]

- Optional: I made an extra test I think will be helpful in catching bugs: [describe where to find]
