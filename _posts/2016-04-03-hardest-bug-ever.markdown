---
title: The Strangest Bug I Ever Debugged
layout: default
author:
  name: Ilya Veygman
  url: iveygman.github.io
---

Let's talk about the most unusual bug I've ever had to debug. This will take us back all the way to about 5 years ago in 2011. 

### Background

At the time, I was working for a major semiconductor manufacturer, Maxim Integrated, making software and algorithms related to optical sensors. This particular project was a new technology (which spawned several patents) that required me to write some demo, test and evaluation software.

Now I wasn't familiar with Arduino at this time, so I used an internal platform called [MAXQ2000](https://www.maximintegrated.com/en/products/digital/microcontrollers/MAXQ2000.html). This was a pretty old system, even by our analog company standards, but it was all I had so I went with it. 

One particularly annoying thing about this chip was that it had no native [I2C](https://en.wikipedia.org/wiki/I%C2%B2C) support, meaning you had to implement it using bit-banging on some of the general purpose I/O (GPIO) ports. Lucky for me, we had an internal library that did just this, so I didn't need to burn a bunch of time poring over the standard and developing this from scratch.

I imported the header, linked the resultant binary and everything was fine. 

For a time.

### The Weird Shit Starts

I realized something odd was happening when I finished doing some changes, unplugged my dev board, went to lunch, then returned to find that I was suddenly unable to communicate with the sensor. Well, not exactly.

See, I could turn it on, but whenever I tried to *read* data from it or change settings, it would always return garbage or not respond. Specifically, it would always return the same values from the data channels no matter what I did. I initially chalked this up to a bad sensor, so I swapped mine out for a new one -- to no avail. Then I assumed the PCB had an issue -- but so did all the other ones.

I thought that, maybe, I somehow introduced a bug in my code, so I reverted to increasingly older variants of the firmware only to find the same issue happening again and again.

Here's where it got really weird: this wouldnt happen on all devices, at least not at first. Some of our modules running *identical code* but built earlier never had any issues. Naturally, in the time it took me to ultimately fix this bug, those became prized posessions amongs our field guys and testers.

### A Wild Workaround Appears

As it turned out, you could remove this issue flawlessly by executing the following steps:

1. Flash the firmware to the device
2. Restart the device but do NOT unplug for more than 0.5 seconds (roughly the time constant of our power circuit)
3. Repeat step 1 every time you move the device

Naturally this wasn't a great solution and we couldn't ship that to customers, but at least we could demo and test for the time being.

Weird, right? Well the annoying part about this workaround is it works even if you don't want it to. What I mean is that this workaround also prevented me from ever using a serial debugger or in-circuit emulator to step through the code and find out what's been happening. Why? Because every time you plug in the debugger and tell it to start, it flashes the firmware anew and restarts, essentially doing the workaround every time. Great, so that's out.

It got even weirder, though: this issue began to appear on previously "good" modules, completely inexplicably. What was going on? Apparently I'd discovered something which is both a [Heisenbug](https://en.wikipedia.org/wiki/Heisenbug) and a Schrodinbug.  

### X Weeks Later

I'm too ashamed to admit how long it took me to find a permanent solution to this bug, but I ultimately found one. Since I couldn't use a debugger, I ended up having to dump massive amounts of print statements onto our serial output to try and figure out what was happening. 

Nothing in my code yielded any insight, so I had to look elsewhere, but where? Well, the only other non-standard library I was importing was our I2C utility. And therein I found the issue, sort of. The header of my main.c looked something like this:

	#include <stdlib.h> // plus some other libraries
	#include "i2c.h"    // our in-house i2c implementation

	extern uint8_t I2C_BUFFER[128];
	extern uint8_t I2C_SLAVE_ADDR; // 7-bit slave address
	
	extern uint8_t i2c_write(...);
	...

Essentially, there was a single buffer you'd either write to or read from to write/read the slave device, and this buffer lived in `i2c.h`:

	// some stdlib includes
	
	uint8_t I2C_BUFFER[128];
	uint8_t I2C_SLAVE_ADDR;
	
	// function definitions here
	
Yeah, yeah, I know, globals bad but I didn't write this code so don't shoot me. So what happened? Well, after putting a few serial outputs in the right places, I found that while I was *sending* the expected bits to the slave device, I was also *receiving* the correct response. This meant that the library was returning the wrong data in `I2C_BUFFER`.

This was a puzzling discovery to say the least, especially knowing that it behaved correctly right after flashing. I fixed it by making the following changes. `main.c`:

	#include <stdlib.h> // plus some other libraries
	#include "i2c.h"    // our in-house i2c implementation

	uint8_t I2C_BUFFER[128]; // this now lives in main
	uint8_t I2C_SLAVE_ADDR; // 7-bit slave address now lives in main
	
	extern uint8_t i2c_write(...);
	...

`i2c.h`:

	// some stdlib includes
	
	extern uint8_t I2C_BUFFER[128];
	extern uint8_t I2C_SLAVE_ADDR;
	
	// function definitions here
	
### Wait, What?

Okay, so I admit I'm not 100% sure that this was the root cause, but I strongly suspect it to this day. See, microcontollers have both flash memory and RAM when executing code, and you can typically only write to one of those during normal execution -- the RAM. Flash is where you keep the firmware whilst your heap would be in the RAM.

So my working hypothesis to this day is that those two globals in `i2c.h` somehow got put into the flash memory address space, which would have possibly remained writable whilst still plugged into the flashing device/software but would have become read-only after losing power. In retrospect, since I theoretically know the range of addresses corresponding to both flash and RAM, I could have confirmed this, but the theory fits pretty well. Somehow, moving the globals to `main.c` caused them to live in RAM and not flash.

### Lessons Learned

**Lesson the First:** don't assume that just because you have a library that it's 1) well-tested, 2) robust or 3) bug-free. This is something only a relatively green engineer would assume and I no longer make these mistakes (so I think anyway).

I tried to report the bug to the original author of the software but it seemed they had left the company.

**Lesson the Second:** abstractions are only good to a certain extent. Yes, the compiler takes care of a lot for you, but at the end of the day, a computer program is a microcontroller executing instructions in a sequence from memory. That memory has physical hardware behind it and may not always behave as expected. It's easy to take that for granted and it can really come back to bite you in the ass.