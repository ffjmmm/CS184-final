<center>
#CS 184: Final Project Proposal
</center>
---
## Title, Summary and Team Members

+ **Title:** Cloth Simulation using OpenGL shader

+ **Summary:** Extend the CPU cloth simulation onto the GPU using OpenGL shaders. Implement cloth simulation with better accuracy as well as implement additional features such as accurate self-collision!

+ **Team Memeber:**
	+ Xinhao Song, 3034504188
	+ Shenao Zhang, 3034487184
	+ Jieming Fan, 3034504370

## Problem Description

We know how to implement cloth simulation in our last project. However, it is not enough if we want a more accurate result. So we will extend the CPU cloth simulation onto GPU using OpenGL shaders and implement it with better accuracy. 
Based on the implementation in project four, we plan to add more features to this simulation system: a ball interacting with the cloth, simulation wind via cloth movement. And we hope to also simulate more complicated scenarios like accurate self-collision to better represent the real world.


## Goals and Deliverables

In this project, we will create a cloth simulation system. This system can simulate accurate cloth like in the real world. We can also interact with the cloth, like controlling a ball to go through the cloth and adding wind towards the cloth. We can control the texture of the cloth, the size of the ball, the power when throwing the ball, the direction and power of the wind and so on.

+ What we plan to deliver:
	1. Extend the cloth simulation from CPU to GPU via OpenGL shader to get better performance. 
	2. Simulate the above interaction of wind, ball and the cloth and different results with different controls.

+ What we hope to deliver:
	1. Change the surface material of the ball to simulate different friction.
	2. Hope to simulate better self-collision with different algorithms.

We will measure the quality / performance of our system by comparing it with what happens in the real world and test if it satisfies the laws of physics. Besides that, we will also treat frame rate as a important physical performance

## Schedule

+ **First Week:** Finish Project 4, read the reference links to learn OpenGL and shader, And start to write code to extend the CPU cloth simulation onto the GPU based on the reference tutorials.

+ **Second Week:** Debug the code and complete the goal of the plan in week 2.

+ **Third Week:** Complete the goal of hope to deliver as much as possible. Start by changing the surface material of the ball to simulate different friction which is relatively easy. And consult relevant information to achieve better collision detection if time is enough.

+ **Last Week:** Check all the features of the project, Prepare presentation slide and videos.

## Resources

+ Introduce to shader: <https://www.khronos.org/opengl/wiki/Shader>
+ OpenGL tutorials: <https://github.com/JoeyDeVries/LearnOpenGL>
+ Mosegaards Cloth Simulation Coding Tutorial: <http://cg.alexandra.dk/2009/06/02/mosegaards-cloth-simulation-coding-tutorial>
+ Newtons law: <https://en.wikipedia.org/wiki/Newton%27s_laws_of_motion>
+ GUI and CPU cloth simulation based on Project 4: <https://cs184.eecs.berkeley.edu/sp19/article/34/assignment-4-cloth-simulation>

## Computer platform

MacOS 10.14 

Apple LLVM version 10.0.0 (clang-1000.11.45.5)

Graphics card：Intel HD Graphics 6000

Xcode 10.1
