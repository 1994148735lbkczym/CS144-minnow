Checkpoint 5 Writeup
====================

My name: [Raymond Llata]

My SUNet ID: [rllata]

I collaborated with: [list sunetids here]

I would like to thank/reward these classmates for their help: [list sunetids here]

This checkpoint took me about [5] hours to do. I [did not] attend the lab session.

Program Structure and Design of the NetworkInterface [Describe data
structures and approach taken. Describe alternative designs considered
or tested.  Describe benefits and weaknesses of your design compared
with alternatives -- perhaps in terms of simplicity/complexity, risk
of bugs, asymptotic performance, empirical performance, required
implementation time and difficulty, and other factors. Include any
measurements if applicable.]:
[To implement the Network Interface I used a combination of a Map & a Queue. I had a master queue to store the order of key insertions into my Map. I used the queue to determine whether or not a address-Ethernet pair had expired, and if it had, I would remove it from the dictionary. Otherwise, I used a dictionary to map "IP DST Addresses" to "Ethernet Addresses". I did so using a struct to store releveant information (ie. if this was a pending ARP request, if the ARP had been send, and the messages queued waiting for an ARP Reply). An alternative approach I considered was only using a Map. Doing this, when I wanted to remove old elements I would need to loop through the entire map, and check each key individually. This would imply O(nlogn) work where "n" is the number of entries in the Map. My approach would use O(mlogn) where "m" is the number of entries to be removed and "n" is the number of entries in the map. Since the implementation changes to use two data structures was not that high, I just decided to use a queue as well, to minimize the extra work my code is doing.

One thing I did to increase the simplicity of my code was using a struct. Initially, I had even more data structures since for every key/addr it was either a known IP or a pending ARP message, and there was alot of state to keep track of. THis resulted in multiple Datastructures, which was initially a bit confusing. In order reduce the complexity I bundled all the parallel data values into a struct, and added some additional housekeeping variables like "isArp" to it to make things simpler.]

Implementation Challenges:
[I think for me the biggest challenge was keeping track of the state-keeping. I think one conceptual misunderstanding I had that caused alot of this was not updating the knownIP timer when the IP was used again. In alot of other cache systems I have seen, (LRU Cache), the deletion timer is generally restarted whenever a value was used, and I thought the same metric would apply to this system. If an Addresses' Ethernet IP is being requested multiple times, maybe it should not be deleted. I built alot of my implementation around this initially, only to find out this was not the case, would caused some refactoring challenges.

Another instance of challenging statekeeping would be when deciding when to add to my IP Queue. Initially, my intent was to add every time something was transmitted, but this raised errors when an Ethernet IP had been transmitted to many times, then deleted and re-recieved, but some of the trasmits persisted in my queue, thus prompting my code to delete the new ARP entry when it should not be deleted. To solve this I added the "last_ARP_sent" var to my struct, as a final check before deletion to decide if this was really recent enough.]

Remaining Bugs:
[]

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this lab better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]
