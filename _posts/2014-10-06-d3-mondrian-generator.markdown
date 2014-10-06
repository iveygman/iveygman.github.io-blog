---
title: D3.js-based Piet Mondrian Art Generator
layout: default
author:
  name: Ilya Veygman
  url: iveygman.github.io
---

To kick off this blog, I want to talk about something near and dear to me: getting out of having to do my high school art and english
class homework. In this instance, we're going to make some art.

[Piet Mondrian](http://en.wikipedia.org/wiki/Piet_Mondrian) was a Dutch painter who was a major contributor to a modern art movement
known as [De Stijl](http://en.wikipedia.org/wiki/De_Stijl). He's probably most famous for works like this:

![Composition II In Red, Blue and Yellow](http://upload.wikimedia.org/wikipedia/en/f/fe/Mondrian_Composition_II_in_Red%2C_Blue%2C_and_Yellow.jpg)

![Composition with Yellow, Blue, and Red](http://upload.wikimedia.org/wikipedia/en/7/72/Mondrian_CompRYB.jpg)

We're going to make our own little script that will generate similar artworks for both fun and possible copyright infringement.

Now, it's not to say that this hasn't been done before. In fact, there are [many](http://www.stratigery.com/automondrian.php) [such](http://fogleman.tumblr.com/post/11959143268/procedurally-generating-images-in-the-style-of-piet) [scripts](http://fullygraceful.tumblr.com/post/63119977601/i-wrote-a-recursive-random-mondrian-generator) out there of varying
quality and style. This was my own take on the matter, and I decided that, instead of using Python like one or two of the scripts above, I'd write the entire thing to be able to run client-side in your
browser using the D3.js library.

## Overview

For this, we will 