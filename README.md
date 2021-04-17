# Some screens must save

## What is this?

It's my first project ever in graphics programming, and my first screensaver too.

Poggers.

> That's how they say, right?

### Technical details and requirements

The program is implemented in C (standard 17), and uses the Win32 API and Direct3D 11.

The tools I'm using are Clang as the compiler, and the Windows SDK for all libraries, headers, and other stuff (looking at you, ``nmake``).

Ideally, one can compile the project by using only the SDK, but i'm not too familiar with ``cl``. I guess it's still a completely viable option, if avoiding to install other stuff is desired, and the Microsoft compiler doesn't look scary; it still does a little bit for me.

## History

I learned to greatly enjoy microtonal music, and one day one of my favorite artists uploaded [this](https://www.youtube.com/watch?v=fCOFjBQ-oKw) video, which coincidentally became one my most loved tracks.

One day, a thought crossed my mind:

> Oh, how cool it would be if my computer showed that crazy geometry stuff when idle...

The day after that, I was already deep into MSDN's docs trying to whip up a window. The first commit is the result of roughly two weeks' occasional work.

Hopefully, this will get as close as possible to the video, without blowing up any GPU.

## Acknowledgements and pointers*

> _Yeah... couldn't resist._

So far, I'm the only one actually writing the stuff, but I stole so much from 🔓[DirectXTutorials](http://www.directxtutorial.com/) that I believe it would've taken me months to get to this point without its lessons. I highly recommend it if you wish to get your feet wet with DirectX in general; be advised that the examples found there are actually written in C++. Also, to be completely honest and transparent, I've only followed the non-paid lessons; I don't know what's in the paid lessons, but the beginner lessons do show promise.

Other than that, [BraynzarSoft](https://www.braynzarsoft.net/) is a nice complement if you're looking for a second opinion. They do have lessons of their own, and seeing two different tutorials back to back can be enlightening. Also, their whole corse is actually free, so you can look for guidance on more advanced topics.

And of course, MSDN and StackExchange are regular places to go look for specific stuff. Although, in my opinion, some times MSDN can be your best friend (obviously), and your worst enemy (_not so_ obviously): bring lots of patience.

Tech stuff aside, I'm deeply grateful to [Sevish](https://www.youtube.com/channel/UCmq2yMo-lTfC7BQLth_PqOw), the artist behind the inspirational video, and all the guys that are in his Discord corner: now I know that I'll never run out of interesting stuff to learn and new, good music to listen. Thank you, I really mean it.

Special shout-out to YAreyaREdAzE. You know why 😉.
