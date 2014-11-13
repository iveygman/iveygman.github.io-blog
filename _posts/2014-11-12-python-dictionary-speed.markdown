---
title: Instantiating Python Dictionaries
layout: default
author:
  name: Ilya Veygman
  url: iveygman.github.io
---

As pretty much anyone who has ever written a Python script knows, you can instantiate empty dictionaries in one of two ways: `my_dict = {}` or `my_dict = dict()`. If you're feeling particularly verbose, you can also do `my_dict = types.DictType.__new__(types.DictType, (), {})` although that's not really my cup of tea and I won't really get into it here.

So what's the difference between those first two methods? Note: all numbers given here are based on Python 2.7.3 running on a 2012 Macbook Pro running OS X Yosemite.

#### Speed

As it turns out, one of these is way faster than the other:

	>>> import timeit
	>>> timeit.timeit("{}", number=100000)
	0.0053899288177490234
	>>> timeit.timeit("dict()", number=100000)
	0.021266937255859375

`dict()` is about four times slower than `{}`. Weird, right? Especially considering that they both return empty dictionaries. So what gives? Doug Hellman goes into some of the reasons [here](http://doughellmann.com/2012/11/12/the-performance-impact-of-using-dict-instead-of-in-cpython-2-7-2.html). In essence, it has to do with how the two dictionaries are allocated under the hood.

#### Disassembly

`dis`, if you didn't know, is a disassembler library where you can look at CPython bytecode. Let's create two functions, `lit()`, which uses `{}`, and `func`, which uses `dict()`, then disassemble them.

	>>> def lit():  return {}
	... 
	>>> def func(): return dict()
	... 
	>>> dis.dis(lit)
	  1           0 BUILD_MAP                0
	              3 RETURN_VALUE        
	>>> dis.dis(func)
	  1           0 LOAD_GLOBAL              0 (dict)
	              3 CALL_FUNCTION            0
	              6 RETURN_VALUE     

Okay so from here, it's obvious that the two things aren't doing the same thing under the hood, and, in fact, `dict()` is doing more. Just knowing that there are extra instructions should be sufficient to understand why one is slower than the other, but just what the hell is `dict()` doing? You can read Doug Hellman's article above to get the full detail, but in summary, it turns out that calling `dict()` with any number of key-value pairs actually creates an intermediate dictionary whereas `{}` does not.
