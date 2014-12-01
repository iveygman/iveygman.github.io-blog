---
title: Bit Twiddling Tricks 101: The In-Place Swap
layout: default
author:
  name: Ilya Veygman
  url: iveygman.github.io
---

One of my favorite tricks in C and C++ is messing around with bit-twiddling. That is, screwing around with bitwise operators like AND, OR or XOR to achieve some neat effect. One of the more popular (and useful) tricks exploits a property of XOR to swap int values in-place. How does it work? Like this:

	int a = 1, b = 2;
	cout << a << ' ' << b << endl;
	a = a ^ b;	// step 1
	b = b ^ a;	// step 2
	a = a ^ b;	// step 3
	cout << a << ' ' << b << endl;

This prints

	1 2
	2 1

Why does this work? It's because of the properties of exclusive-or in Boolean algebra. Recall the truth-table for exclusive-or:

|   | 0 | 1 |
|---|---|---|
| 0 | 0 | 1 |
| 1 | 1 | 0 |

For any value, `A`, this means that `A XOR A == 0`. In the first stage of the swap, we set `a = a ^ b`. This means that in the second stage, `b ^ a` is effectively the same as `b ^ a ^ b`. Because exclusive-or is also associative, that is `A XOR B == B XOR A`, Therefore, `b ^ a` is equivalent to `b ^ b ^ a`, which, from associativity, is simply `a`. This is why we do step 1 -- it sets up `a` as a masked variable with `b`.

In step 3, we need to recover the original value of `b` similarly to get our new value of `a`. Since `b` now has the original value of `a` and `a` is `a ^ b`, this becomes equivalent to `a = a ^ b ^ b`, which, with `b`'s new value, is the same as `a ^ b ^ a`.

The whole point of this confusing-looking formula is to avoid temporary variables altogether. However, as an illustrative example, let's see how this looks with a temporary variable `x`:

	int a = 1, b = 2, x;
	cout << a << ' ' << b << endl;
	x = a ^ b;	// step 1
	b = b ^ x;	// step 2 (b now has original value of a)
	a = x ^ b;	// step 3 (a now has original value of b)
	cout << a << ' ' << b << endl;

#### Standard C++

Since C++11, you can now use `std::swap` as a fast, reliable way to swap the values of any two variables of the same type. This may be a better approach for many since the above XOR-method doesn't work for `float` or many other types of data.