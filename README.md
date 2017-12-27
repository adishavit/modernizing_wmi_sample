## `[WiP] ### Initial Commit. ###`

# C++ Modernization of Windows C Code

In December 2017, [Simon Brand](https://blog.tartanllama.xyz/optional-expected/), [Vittorio Romeo](https://vittorioromeo.info/index/blog/adts_over_exceptions.html) and [Jonathan Müller](http://foonathan.net/blog/2017/12/04/exceptions-vs-expected.html) all published widely circulated blog posts extolling the virtues of Algebraic Data Types (ADTs) vs. exceptions. 

This repo is an attempt to try out these techniques on real world code to understand the implications, pros and cons of the various approaches. The project also investigates the conveniece and utility of scope-guard-like generic scoped RAII resource management utilities.   

When this investigation is done, it will be discussed in yet another [blog post](http://videocortex.io/#blog). 

The code to be refactored is an [MSDN sample program](https://msdn.microsoft.com/en-us/library/aa384724(v=vs.85).aspx) from the Windows WMI Performance Monitoring documentation. It is a typical, non-trivial, sample, similar to many other Windows WIN32 C APIs. 


***This is a work in progress.***
