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
2. [ ] ... vs Entity Component System (ECS)
3. [ ] The cost of an arithmetic operation
4. [ ] The order of arithmetic operations (it matters)
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
When I run the AoS code on a Apple M3 Pro ARM-CPU I get a $\sim 5.90$ ms physics-update time.
The SoA code gives me a $\sim 3.65$ ms physics-update time.
We find the improvement through $$\frac{baseline}{new time} = \frac{5.90}{3.65} \approx 1.62,$$ so we have a $1.62$x improvement by switching the code structure from AoS to SoA.