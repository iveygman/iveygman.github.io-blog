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

![Composition II In Red, Blue and Yellow](http://upload.wikimedia.org/wikipedia/en/f/fe/Mondrian_Composition_II_in_Red%2C_Blue%2C_and_Yellow.jpg =250x "Composition II In Red, Blue and Yellow") ![Composition with Yellow, Blue, and Red](http://upload.wikimedia.org/wikipedia/en/7/72/Mondrian_CompRYB.jpg =250x "Composition with Yellow, Blue, and Red")

We're going to make our own little script that will generate similar artworks for both fun and possible copyright infringement.

Now, it's not to say that this hasn't been done before. In fact, there are [many](http://www.stratigery.com/automondrian.php) [such](http://fogleman.tumblr.com/post/11959143268/procedurally-generating-images-in-the-style-of-piet) [scripts](http://fullygraceful.tumblr.com/post/63119977601/i-wrote-a-recursive-random-mondrian-generator) out there of varying
quality and style. This was my own take on the matter, and I decided that, instead of using Python like one or two of the scripts above, I'd write the entire thing to be able to run client-side in your
browser using the D3.js library.

### Overview: Basic Algorithm Idea

For this, we will be implementing something similar to *Composition II in Red, Yellow and Blue*. To do this, we'll take the following steps:
1. Draw some lines that will either stop at the edge of the canvas or at another line
2. Find the rectangles formed by these lines
3. Fill in some of these rectangles with red, yellow or blue

Numbers 1 and 3 are actually fairly straightforward with a bit of math knowledge and Googling. Number 2 is probably the toughest part.
Let's go through each algorithm section and explain how we accomplish this.

#### Step 1: Draw Some Lines

Each line can be thought of as a partition of the canvas, since it divides up either a section or a subsection into smaller chunks. So first things first, we'll
choose how many times to partition. Let's pick some random number between 4 and 16, why not. 

Actually, I lied. First you need to create a place for D3 to draw stuff. We'll call this our canvas, even though we're not strictly using the HTML5 \<canvas\> element. To do this, we'll just put an SVG (D3's choice of formats) into a \<div\> element on our page whose CSS class is mondrian, like so:

    var container = d3.select(".mondrian").append("svg").attr("width", maxWidth).attr("height", maxHeight);

Obviously you'd want to give it some maximum height and width so it fits in the space you want. Next, we need to draw a border. You 
can decide later if you want to leave it in or not, but for now we'll draw four black lines around the outside of our canvas. We will also
keep track of each line we draw and each point at which the lines intersect, like so:

    var intersectionPoints = [{x:0,y:0}, {x:0,y:maxHeight},{x:maxWidth,y:0}, {x:maxWidth,y:maxHeight}];

Drawing a line in d3 can be done like so:

    container.append("line").attr("x1", point1.x).attr("y1", point1.y).attr("x2", point2.x).attr("y2", point2.y)
                                .attr("stroke-width", width).attr("stroke", color);

As you can imagine, this is pretty verbose, so we'll be using a helper function called makeLine to handle this for us. makeLine accepts
the SVG's container, a start point, an end point, a line width and a line stroke. Here we'll actually draw the border lines in white because
we don't want to see them.

    var upperLeft = {x: 0, y: 0};
    var lowerLeft = {x: 0, y: maxHeight};
    var upperRight = {x: maxWidth, y: 0};
    var lowerRight = {x: maxWidth, y: maxHeight};
    container = makeLine(container, upperLeft, upperRight, 4, "white");
    container = makeLine(container, upperLeft, lowerLeft, 4, "white");
    container = makeLine(container, lowerLeft, lowerRight, 4, "white");
    container = makeLine(container, upperRight, lowerRight, 4, "white");

After this we'll store these four lines, as defined by their start and end points, designated as line.p1 or line.p2, in an array:

    drawnLines.push({p1: upperLeft, p2: upperRight});
    drawnLines.push({p1: upperLeft, p2: lowerLeft});
    drawnLines.push({p1: lowerLeft, p2: lowerRight});
    drawnLines.push({p1: upperRight, p2: lowerRight});

Okay, now down to the fun part: let's make N partitions. To do this, you'll need to do the following:

	for 1 to numPartitions:
		randomly choose to draw a vertical line or horizontal line, L, with endpoints spanning the height or width of the canvas, respectively
		check if L is closer than some threshold to any line in drawnLines
		if yes, choose a new L as above, else continue
		for each line M in drawnLines:
			if L and M intersect at point K:
				randomly replace either L.p1 or L.p2 with K
		draw the newly-shortened L
		store L in drawnLines
		store L.p1 and L.p2 in intersectionPoints

For the sake of brevity, I'm omitting the actual implementation of this, but you can see the full implementation within mondrian.js in this project's repository. The reason why you need to check if L is too close to other lines is because if you don't, you can either get really tiny rectangles or lines on top of one another. In my implementation, I mandate that there are at least minRectSide pixels between each parallel line.

#### Step 2: Find the Resultant Rectangles

Obviously, we want to fill in some of the rectangles formed by the intersection points of this newly-drawn grid of lines. We could, in theory, find *any* rectangle formed by any four intersection points, but that wouldn't be fun or challenging. Also, it would no longer look like *Composition II in Red, Yellow and Blue*. How do we find this? There are probably lots of algorithms out there, but here's how I did it:

	// let each rectangle be defined by its upper-left and lower-right points, P1 and P2
	// the upper-right and lower-left points shall be P3 and P4, respectively, as well
	rectangles = []
	for each P1 in intersectionPoints:
		// let P1 be the upper-left point of the rectangle
		if P1 is already the upper-left point in any rectangle in rectangles, discard this P1 and continue to next iteration

		let candidatesP4 be a list of possible points that may be the P4 corresponding to this P1
		for each P4 in intersectionPoints:
			if P4 has the same x and a larger y than P1, store P4 in candidatesP4
		if candidatesP4 is empty, go to next P1

		let candidatesP3 be a list of possible points that may be the P4 corresponding to this P1
		for each P3 in intersectionPoints:
			if P3 has the same y and a larger x than P1, store P3 in candidatesP3
		if candidatesP3 is empty, go to next P1
		otherwise, find the entry in P3 that is closest to P1, let this be P3

		// now find a point P2 such that:
		//		1. P2.x == P3.x
		//		2. P2.y >  P3.y
		//		3. There exists a point P4 such thatP4.x < P2.x and P4.y == P2.y and P1.x == P4.x
		let candidatesP2 be a list of possible points that may be the P4 corresponding to this P1
		for each P2 in intersectionPoints:
			if P2.x == P3.x and P2.y > P3.y:
				for each P4 in intersectionPoints:
					if P4.x < P2.x and P4.y == P2.y and P4.x == P1.x:
						then such a P4 exists
				if such a P4 exists, add P2 to candidatesP2

		if candidatesP2 is empty, go to next P1
		otherwise, P2 is the first element in candidatesP2
		define a new rectangle, R, with upper-left = P1 and lower-right = P2
		add R to rectangles

#### Step 3: Color The Rectangles

This is the last step. Here we need to randomly pick some of the rectangles we found in the previous step to color and fill them in. The real trick here, honestly, is to figure out how many of the rectangles we want to color in. For my implementation, I randomly choose about 25% of the rectangles to be colored in, but also mandate that at least 1 but no more than 33% of the rectangles are colored. To implement this, I set a series of flags corresponding to each rectangle. If the flag is 0, I don't color. Otherwise, I color. Here's how it's done in pseudocode:

	repeat until between 1 and 1/3 of total are to be colored:
		for each rectangle in rectangles:
			tag rectangle as to be colored with 25% probability
	for each rectangle in rectangles:
		if rectangle is to be colored:
			randomly select a color from [red, yellow, blue]
			color the rectangle
	redraw all lines except borders

### D3.js

I picked this partially because I wanted to learn more about D3. Since it's mostly a library designed for drawing, it only appears in three places in this project. 

The first is when D3 appends an SVG to the .mondrian div on the webpage and initializes it with some width and height. This is pretty straightforward:

    var container = d3.select(".mondrian").append("svg").attr("width", maxWidth).attr("height", maxHeight);

We store this object representing the SVG in `var container`. The other two places where it shows up are in the methods used for drawing lines and filled rectangles, respectively:

	function makeLine(lineContainer, point1, point2, width, color) {
    	lineContainer.append("line").attr("x1", point1.x).attr("y1", point1.y).attr("x2", point2.x).attr("y2", point2.y).attr("stroke-width", width).attr("stroke", color);
    	return lineContainer;
	}

	/**
	    Draws a rectangle of the desired size and fill shape
	        rect contains an upper left and lower right x,y point defining its borders
	 */
	function drawRectangle(container, rect, color) {
	    var xStart = rect.p1.x;
	    var yStart = rect.p1.y;
	    var width = Math.abs(rect.p2.x - xStart);
	    var height = Math.abs(rect.p2.y - yStart);
	    container.append("rect").attr("x", xStart).attr("y", yStart).attr("width", width).attr("height", height).attr("fill", color);
	}

`makeline` is a very simple method -- here we simply append a line object to the container (`.append('line')`), then set its start and end points. We also give it a color and a stroke width, in pixels. An example call to this in this project might be `makeLine(container, {x: 0, y: 0}, {x: 0, y: maxHeight}, 4, "black")`, which draws a line along the left-most border of the SVG that is 4 pixels wide and black.

`drawRectangle` involves a bit of math but is otherwise pretty simple. We define the origin of the rectangle, which in d3.js is the upper-left corner, as `p1` of the `rect` object passed to this function. From there, we compute the height and width, respectively, by computing the distances in x and y, respectively, between `rect.p1` and `rect.p2`. From there, we `.append('rect')` to the container, giving it the start x and y coordinates, the height, the width and a fill color. In our case, the fill color is chosen pseudorandomly from a list of three possible colors.

I have one last utility function used solely for debugging:

	/**
	    Draws text
	 */
	function addText(container, pos, _text) {
	    return container.append("text").attr("dx", pos.x).attr("dy", pos.y+10).text(_text);
	}

This adds some text to the container at the specified position.