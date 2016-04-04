---
title: Sunday Hack&#58; Democratic Primary Simulator
layout: default
author:
  name: Ilya Veygman
  url: iveygman.github.io
---

I've been out for a while doing... things, but I figured I would try to start posting random crap here. The 2016 Presidential Primaries are currently ongoing and everyone is trying to figure out who will be the big winner.

I, for one, have been browsing the prediction markets on [PredictIt.com](www.PredictIt.com) as well periodically reading FiveThirtyEight.com, particularly [this post](projects.fivethirtyeight.com/election-2016/delegate-targets/democrats/), which is a nice visualization of what's currently going on. If you browse Reddit, you're also aware that the site is very much in favor of Bernie Sanders (/r/The_Donald aside), to the point that it gets a bit spammy. Well, one post that caught my eye was under /r/bestof about [how exactly the delegate math may work out for Sanders](https://np.reddit.com/r/enoughsandersspam/comments/4avw76/the_numbers_are_looking_really_bad_for_sanders/). This seems like a good opportunity to practice my Javascript. Let's get into it.

But first, you may find the actual page here: [http://iveygman.github.io/demprimarysimulator/](http://iveygman.github.io/demprimarysimulator/)

### Goal

Our goal here is to make a tool where we can visualize how the two candidates may fare under certain scenarios. We don't want to bother simulating primaries that have already finished, instead focusing on future races that haven't yet been resolved. Individual races may be simulated individually or we can look at a few preset scenarios.

### Our Tech Stack

I'm always looking for an excuse to shoehorn in [JQuery](https://www.JQuery.com), but it didn't really fit here so it was left out. Since we're dealing with a bunch of data that we want to dynamically manipulate, [AngularJS](https://angularjs.org) is a useful tool. Finally, we'll be making a simple bar graph to visualize it all, so we'll also pull in [D3.js](https://d3js.org/). The entire application will be front-end to keep it quick and light (well for our servers anyway, heheheh)

### Step 1: Make the Table

See [app.js](https://github.com/iveygman/demprimarysimulator/blob/master/app.js) to follow along.

Create a data controller `DataCtrl` that will control our DOM and load our data in a nice JSON format. It's not enough to simply load the data, we also want to track some aggregates, so after we load the Democratic primary data into our scope, we also want to assign each unfinished race some additional flags and members:

	for (i in $scope.democrats) {
		var notSet = $scope.democrats[i].Clinton.actual == null;
		$scope.democrats[i].notYetSet = notSet;
		$scope.democrats[i].totalDelegates = 0;
		for (j in $scope.democratnames) {
			var name = $scope.democratnames[j];
			$scope.democrats[i][name].modeledDelegates = $scope.democrats[i][name].expected;
			$scope.democrats[i].totalDelegates += $scope.democrats[i][name].expected;
		}
	}

Let's break this down.

We say that any entry where the actual delegate count is null hasn't yet been set (we can check either Clinton or Sanders for this). To any object, we set a flag that corresponds to whether or not this field is null (`$scope.democrats[i].notYetSet`). We then want to know how many total delegates there are per race as well as to set a model of expected delegates per candidate for each race.

We let `modeledDelegates` be the same as `expected` for each candidate in each race, per the FiveThirtyEight modeled data. This is the "expected" result assuming Nate Silver's models are 100% accurate.

On the [HTML side](https://github.com/iveygman/demprimarysimulator/blob/master/delegates.html), we make a nice table showing the date and location of each primary, as well as the total number of delegates at stake and the number of expected delegates for each candidate. If the race has already finished, then we display the actual number of delegates won by each candidate. Otherwise, we display the adjustable `modeledDelegates` for the candidates.

This adjusting can be done using a slider to the right of the delegate count cells. Here's what the code looks like to make this slider:

	<td class="slider-cell">
		<span ng-if"!primary.notYetSet">&nbsp;</span>
		<span ng-if="primary.notYetSet" title="More votes for Clinton" class="range-label">C</span>
		<input 	class="slider"
				ng-disabled="!primary.notYetSet"
				ng-model="primary.Sanders.modeledDelegates"
				ng-change="sliderDidChange(primary.state, 'democrats', primary.Sanders.modeledDelegates)"
				type="range" 
				id="slider-{{ primary.state }}" 
				min=0 
				max="{{ primary.Clinton.expected + primary.Sanders.expected }}" 
				value="{{ primary.Sanders.modeledDelegates }}">
		<span ng-if="primary.notYetSet" title="More votes for Sanders" class="range-label">S</span>
	</td>

And in Javascript:

	$scope.superDelegatesDidChange = function(sandersDelegates) {
		$scope.democratsuperdelegates.Sanders = parseInt(sandersDelegates, 10); // to ensure we have a number
		$scope.democratsuperdelegates.Clinton = $scope.democratsuperdelegates.total - $scope.democratsuperdelegates.Sanders;
		$scope.updateModel();
	}

Let's explain this too. We want to disable the adjusting if the individual primary has finished, so we use `ng-if` and `ng-disabled` with the flag `primary.notYetSet`, where `primary` is our iterator. The adjustment is modeled by moving one candidate's -- here Sanders' -- delegate count up or down. Since only two candidates are present, we can trivially compute Clinton's delegate count from this input as well. What does `updateModel()` do? 

At the bottom, we see the totals for the number of delegates won so far both with and without the modeled data. This is what we -- I admit pretty inefficiently -- do in `updateModel()`. What do you want from me? I did this in a few hours. This function does two things:

1. Sum up the total number of actual, expected and modeled delegates (see `sumAllDemocraticDelegates` for a messy implementation).
2. Reset the data we will use in our plot (more on that later).

You can adjust delegate counts for unfinished races to see how this works out.

You can also download the code and null out already-finished races if you want to try more what-if scenarios.

### Step 2: Make the Plot

Here's where D3 comes in. I'm pretty shit with this library, so I ended up finding an example [here](http://bl.ocks.org/biovisualize/5372077) and modifying it accordingly. I won't get into the details here, since I just hacked it together through trial and error, but I'll touch on two modifications I made to the original code.

First, there is this bit toward the end of the D3 block:

	if (drawCount < 2) {
		drawCount += 1;
		svg.append("text").attr("text-anchor", "middle")  // this makes it easy to center the text as the transform is applied to the anchor
	    .attr("transform", "translate("+ margin.left/2 +","+ chartH/2+")rotate(-90)") // text is drawn off the screen top left, move down and out and rotate
	    .text("100's of delegates");
	}

Why do we care how many times we draw? Well it turns out that if you don't do this, the library will keep overwriting its axis label and make it kind of ugly. This refresh happens any time we modify the model data, so it can happen a lot. It's very hacky, yes, and there's likely a better way of doing it, yes, but, again, I wanted something quick and dirty.

Second, we modify the style of the bars to change color depending on their value. If the value rises above the total number of delegates needed to win, then we change the color to gold. Otherwise, it stays blue. See

	   }).style('fill', function(d, i) { 
	   	if (d > delegatesToWin.democrats) {
	   		return '#CCC02B';
	   	} else {
	   		return '#0000DD';
	   	}
	   });

### Step 3: Make Some Scenarios

We make a few callbacks and some pretty CSS spans that will manipulate the model data in certain ways to simulate scenarios. For example, one such scenario is Bernie Sanders winning every race not yet completed by 10% MORE than expected. For this we create a callback in our scope:

	$scope.sandersPctBetter = function(winScale) {
		$scope.reset();
		for (i in $scope.democrats) {
			if ($scope.democrats[i].notYetSet) {
				$scope.democrats[i].Sanders.modeledDelegates = Math.round($scope.democrats[i].Sanders.expected * winScale);
				$scope.democrats[i].Clinton.modeledDelegates =  $scope.democrats[i].Sanders.expected + $scope.democrats[i].Clinton.expected - $scope.democrats[i].Sanders.modeledDelegates;
			}
		}
		$scope.updateModel();
	}
	
As an explanation, we first reset to the "default" state (more on that in a bit), then for any unfinished race, we set `modeledDelegates` to be some multiplier of `expected` for Sanders. We then subtract that from the total number of delegates available for this race to get Clinton's resultant delegate count. By letting `winScale` be set by the caller, we can simulate several scenarios like 20% and 30% more than expected wins across all such states. `updateModel` is called at the end to ensure that we get correct totals and that the graph is appropriately updated.

Obviously we want to be able to reset to the original model states after messing around, so a "Default" is provided. This simply resets `modeledDelegates` to be equal to `expected` for each candidate in each unfinished race.

	$scope.updateModel = function() {
		if (!$scope.superDelegatesEnabled) {
			$scope.democratsuperdelegates.Sanders = $scope.democratsuperdelegates.Clinton = 0;
		}
		$scope.sumAllDemocraticDelegates();
		$scope.data = [$scope.totalDelegates.Clinton, $scope.totalDelegates.Sanders];
	}

There's another callback for a big win in states with delegate count above some threshold (e.g. Sanders gets 75% of the vote in all "big" states or all "small" states), however I won't go into that here.

Okay so what about super delegates?

### Step 4: The X Factor

The part that Nate Silver doesn't model on that page is super delegates since, in theory, they could vote as they please. To account for this, we add a checkbox above the plot that lets us turn the super delegate simulation on or off. If we turn this on, both candidates start with 0 delegates until the slider is disturbed (yeah, I know, lazy).

Since this is unmodeled, we always reset it by default when switching scenarios -- though you can simply turn it back on and set it however you like afterward.


### Conclusion and Looking Forward

This was actually pretty fun and different from the daemon or CI automation development I do at work. At some point, I'd like to generalize this for any primary race (e.g. when there are no superdelegates or when there are multiple competitors such as the state of the GOP primary at the time I'm writing this). I'd also like to clean up the code a bit so it's not quite so... hacky.

Finally, I'm hoping I can motivate myself to post here more often but... yeah we'll see.