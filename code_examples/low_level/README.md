# Welcome!

This is a README for a now work in progress.
I ([@NilsECT](https://github.com/NilsECT)) will be adding some low level optimisation examples along with some tips, tricks and useful links (not the video game character).

##### A note on the language used:
The text may appear less serious than the information presented.
Many of the things presented in this subdirectory is useful in a general sense and in some cases critical for adequate performance.
In most cases, for me at least, low-level optimisation takes the form of a mini-game driven by the question "_Can I make it go faster?_" or "_I want more things happening! (?)_" where it becomes a fun pastime rather than work.
As such, I have chosen to apply a language that shows just that:
>Low-level optimisation is fun.

## What will come

This directory will contain an initial program which will model some simple space physics and an optimised version of the.

Based on the initial program, I will push examples of
1. [x] Array of structures (AoS) vs Structure of Arrays (SoA)
2. [x] ... vs Entity Component System (ECS)
3. [x] The cost of an arithmetic operation
4. [x] The order of arithmetic operations - Instruction Level Parallelism
5. [ ] Superscalar CPU, i.e. built-in parallelisation between float and integer operations
6. [ ] Fixed point arithmetics

## The project

I believe the best way to learn how to optimise (or simply program) at low-level is to have a project.
Therefore, I have set out to write a complete program which **you** can play around with and in which **I** can show you how to apply different optimisation techniques.
When I write "complete program" I mean complete with regard to the checklist above; you are free to take the code and expand or reduce it in whichever way you fancy, and please feel free to do so!

For the project I will simulate a 2D box with holes in which a number of small particles will be injected.
The holes will have varying sizes and mass while the particles will have a fixed size and mass.
As an initial case the holes will not be moving while the particles will be affected by force of gravity.
If I have the time I will expand the rules of movement for each class and maybe add some more classes.
If I find more time and some courage I will add a user interface (UI) and textures to make it look nice.

The simulation will be displayed on a screen using the SFML package for simple graphics rendering.
To measure the performance I will measure the time it takes to update each physics-step and gather it in a moving average for a relatively stable point of comparison.
The goal is then to either increase the number of particles on screen without increasing the update rate or to reduce the update rate while using the same number of particles.
Here, one physics-step is calculating one timestep ($dt$) ahead; there may be a maximum number of physics updates between each frame update (the rendering step).
This way we can keep improving the update rate without having it affect our visual experience: if these steps were not separate, the particles would move faster and faster for each improvement to the code.

First the full program will be written without optimisation completely object oriented, i.e. using array of structures (AoS).
As a First I will transform the code to be a structure of arrays (SoA) where necessary.
Based on the SoA code, I will implement optimisations through `inline` functions via their respective header files.
Each implemented optimisation can will be found in the subsections for the different optimisation techniques (see **What will come**), where they will be presented along with the necessary background information to understand _why_ and/or _how_ it works.


## AoS vs SoA

This is placeholder text, I will go into detail later but for now:

I wrote the AoS (fully object oriented) code late winter 2025.
The code was written with many suboptimal implementations and steps, some of these were on purpose yet most -probably- are simply sourced in some wisdom of not writing optimal code on the first try.
I left the code aside to complete my thesis and go on vacation, through which I completely forgot how I wrote the code.
Now, starting in late October 2025, I have picked the code up again as if I found it on the internet and had never anything to do with it before now: I did this in part because that is most likely your situation now, and because I struggled a little with picking it up again.

The first step was to transform the code from AoS to SoA.
It took me $1.5$ hours to get reacquainted with the code (it's a pretty small repository to be fair) and 5 hours to transform the code into SoA and adapt the CMakeLists.txt file and repository structure such that I can choose which version to run when I compile.
When I run the AoS code on an Apple M3 Pro ARM-CPU for $100$ particles I get a $\sim 5.90$ ms physics-update time.
The SoA code gives me a $\sim 4.00$ ms physics-update time.
We find the improvement through $$\frac{baseline}{new time} = \frac{5.90}{4.00} \approx 1.475,$$ so we have a $1.475$x improvement by switching the code structure from AoS to SoA.

## ECS

This is placeholder text, I will go into detail later but for now:

ECS is quite a different way to write your code, but it ends up being fully SoA and incredibly general.
There are some hiccups to keep it general in face of new entities (say ships or dark matter) which make for some double calculations in loops.
For instance I chose to update the position of all entities with velocities with respect to all entities which have mass and position.
That allows for a general updating of gravitational forces at the cost of looping extra over all entities (suboptimal looping).
The strength is now that the changes I need to apply to the rest of my code when adding new entities (things that interact with the environment or other forces) are minimal, almost none: which is very nice if I want to make this simulation grow in complexity.

It took me 6 hours to write the ECS code, the main difficulty was to get used to the new coding paradigm (again).
Running the ECS code on an Apple M3 Pro ARM-CPU for $100$ particles I get a $\sim 8.00$ ms physics-update time.
In this case, compared to AoS code we have no improvement other than the code is more easily maintainable... but does it scale better than OOP ?

## The cost of an arithmetic operation

This is placeholder text, I will go into detail later but for now:

In [`include/optimisation_soa/policies.h`](https://github.com/anderkve/FYS3150/blob/master/code_examples/low_level/include/optimisation_soa/policies.h) you can find two `inline` functions which are named `policies::baseline` and `policies::simplified`.
The first of those is the code you can find in the SoA directories, i.e. before we try to optimise.
The latter, `policies::simplified`, contains the same logic but where I've reduced the number of arithmetic operations, namely:
1. there is one square root instead of two,
2. mass is absent as we return an acceleration,
3. there is now 1 division instead of two,
4. and we precompute the reciprocal.

To efficiently change between the two policies you can compile with a cmake flag `-DCOST_FUNC=policy::simplified`, where what comes after the equal sign is the function you want to use.
You are free to define a function for each change, and I invite you to do so!

In my case, when running on an Apple M3 Pro ARM-CPU I get an improvement of $1.6$x.
With the improvement of converting the code to SoA from the OOP AoS which was $\sim1.5$x, we get a total improvement of approximately $1.6 \times 1.5 =2.2$x.

To add some flesh to this example you can also explore the [`examples.cpp`](https://github.com/anderkve/FYS3150/blob/master/code_examples/low_level/examples.cpp) file.
In it you will find an example to measure the cost of a square root operation in terms of nanoseconds.
This example is minimal, but the improvement is non-negligible: I get a 1.1x improvement from removing a `std::sqrt()`.

## The order of independent arithmetic operations - Instruction Level Parallelism (ILP)

This is placeholder text, I will go into detail later but for now:

I have implemented an ILP example in [`include/optimisation_soa/policies.h`](https://github.com/anderkve/FYS3150/blob/master/code_examples/low_level/include/optimisation_soa/policies.h) with leading name `braid2` followed by the arithmetic policy to use.
In essence, we want to execute multiple independent instructions (such as arithmetic operations) at the same time.
We achieve this by separating independent computations such that while the CPU is working on executing the first instruction it can start on the second as it is independent on the fist:
$$a = b + c$$
$$d = e + f$$
$$g = a + d.$$
In the above case $a$ and $d$ can be computed independently of each other and so the CPU can compute them 'in parallel'.
It's not REALLY in parallel, but the total time from start to finish may be diminished.
There are some quirks that may come in the way of having improvements from ILP.
Take my implementation for instance: I get a $<1$x improvement from taking two items at a time rather than one.
This can be due to myriad things, though I suspect it is because I fetch the same data multiple places unnecessarily (see where I access arrays).
The time it takes to fetch the data is comparable to the computation speed-'gain' AND I suspect I should braid over more than two items at a time.

To check I implemented a `braid2` and `braid4` in [`examples.cpp`](https://github.com/anderkve/FYS3150/blob/master/code_examples/low_level/examples.cpp) to check.
For `braid4` I also cache the array fetching since I am quite aware of it slowing down my code alongside my speed-up, hence they go hand-in-hand.
I only get an improvement from the baseline with the `braid4` case, and indeed: an improvement comparable to removing a square root!
Lesson: first remove the square root, then maybe restructure your loop if you need to.
