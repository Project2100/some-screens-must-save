# Some screens must save

## What is this?

It's my first project ever in graphics programming, and my first screensaver too. Whichever is hardest to learn, I'll let you be the judge.

### Technical details and requirements

The program is implemented in C (standard 17) and HLSL, and uses the Win32 and Direct3D 11 APIs.

Altough different tools have been used to build this project in previous revisions (Clang, basically), it is currently buildable from the Developer Consoles provided with Visual Studio by using ``nmake``. The Windows SDK version used as of today (2021-05-08) is 10.0.19041.0, some previous versions have proved to be fine too.

> _Finally got over ``cl``, yes._

## History

I learned to greatly enjoy microtonal music, and one day one of my favorite artists uploaded [this](https://www.youtube.com/watch?v=fCOFjBQ-oKw) video, which coincidentally became one my most loved tracks.

One day, a thought crossed my mind:

> _Oh, how cool it would be if my computer showed that crazy geometry stuff when idle..._

The day after that, I was already deep into MSDN's docs trying to whip up a window. The first commit is the result of roughly two weeks' occasional work.

Hopefully, this will get as close as possible to the video, without blowing up any GPU.

## Lessons learned

I want to dedicate a special section here to _those_ takeaways that may not be clear at all when developing such programs, and nothing seems to concretely help you even with your best efforts.

### _Screensavers don't like files<sup>1</sup>_

To some, this may sound obvious at first. Observe that this is a program that is required _by design_ to execute without a console. Barring debuggers, a very easy choice for getting a trace of what is happening at runtime is to setup a logfile.

A screensaver that uses logfiles will work perfectly on preview, and even up to first install; however, it will fail when it is actually invoked after the idle interval, _and no logs will be created whatsoever_. What may surprise, is that this behaviour can be reproduced by simply importing ``stdio.h`` in the source, without using a single file-related function at all.

The only hint of what happens on crash is in the Windows Event Viewer, which reports a ``STATUS_STACK_BUFFER_OVERRUN`` error; I've learned from the arguably most famous Windows guru that [this error can be infuriatingly misleading](https://devblogs.microsoft.com/oldnewthing/20190108-00/?p=100655).

However, the sneaky catch lies in the consequences for DirectX 11: a popular strategy in building the graphics pipeline involves compiling the shaders' files at runtime by using ``D3DCompileFromFile``. As you might have guessed by now, this is a file operation. _Same problem here_.

The takeaway here is: when you want to build an ``.scr`` file that _actually works_, check carefully for anything that may involve a file, and possibly change it to avoid using the file itself.

Do however keep a logging infrastructure for a separate build scenario, if you feel comfortable with it. I know I did.

<sub><sup>1</sup>: To add to confusion, the [MSDN page dedicated to screensavers](https://docs.microsoft.com/en-us/windows/win32/lwef/screen-saver-library) talks frequently about interacting with an ``.ini`` configuration file. I admit that I still haven't investigated much about that.</sub>

## Acknowledgements and pointers*

> _Yeah... couldn't resist._

So far, I'm the only one actually writing the stuff, but I stole so much from 🔓[DirectXTutorials](http://www.directxtutorial.com/) that I believe it would've taken me months to get to this point without its lessons. I highly recommend it if you wish to get your feet wet with DirectX in general; be advised that the examples found there are actually written in C++. Also, to be completely honest and transparent, I've only followed the non-paid lessons; I don't know what's in the paid lessons, but the beginner lessons do show promise.

Other than that, [BraynzarSoft](https://www.braynzarsoft.net/) is a nice complement if you're looking for a second opinion. They do have lessons of their own, and seeing two different tutorials back to back can be enlightening. Also, their whole corse is actually free, so you can look for guidance on more advanced topics.

And of course, MSDN and StackExchange are regular places to go look for specific stuff. Although, in my opinion, some times MSDN can be your best friend (obviously), and your worst enemy (_not so_ obviously): bring lots of patience.

Many thanks go to [McLytar](https://github.com/mclytar) for testing it consistenlty on his system; it helps a lot to have a different testing environment... with multiple screens, or just better GPUs.

Tech stuff aside, I'm deeply grateful to [Sevish](https://www.youtube.com/channel/UCmq2yMo-lTfC7BQLth_PqOw), the artist behind the inspirational video, and all the guys that are in his Discord corner: now I know that I'll never run out of interesting stuff to learn and new, good music to listen. Thank you, I really mean it.

Special shout-out to YAreyaREdAzE. You know why 😉.
