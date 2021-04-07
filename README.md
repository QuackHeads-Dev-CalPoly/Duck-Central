# Duck Central
Duck Central is the repository for code before the Great CDP port of 2021. This code is written using the Raspberry Pico C/C++ SDK


## Raspberry Pico
The Raspberry Pi foundation released the Pico in early 2021 as their first **microcontroller**. Note: this is **NOT** a "normal" Raspberry Pi. The Pico does **NOT** run an operating system, it is a microcontroller.

Arguably the best feature of the Pico is its documentation. The docs are extremely well written, and allow you to get set up quickly and easily.

#### Getting started on the Pico
See this doc for getting your IDE and pico set up (on Mac, Windows, *nix) and blinking: https://datasheets.raspberrypi.org/pico/getting-started-with-pico.pdf

#### Using the Pico SDK
See this doc for anything you want to know about the Pico C/C++ SDK: https://datasheets.raspberrypi.org/pico/raspberry-pi-pico-c-sdk.pdf

#### Pico datasheet
You probably won't need this, but here is the datasheet for the Pico: https://datasheets.raspberrypi.org/pico/pico-datasheet.pdf

#### RP2040 datasheet
You also probably won't need this, but here is the datasheet for the RP2040, which is the actual microcontroller on the Pico board: https://datasheets.raspberrypi.org/rp2040/rp2040-datasheet.pdf

#### Pinout![Raspberry Pico Pinout](https://www.raspberrypi.org/documentation/rp2040/getting-started/static/64b50c4316a7aefef66290dcdecda8be/Pico-R3-SDK11-Pinout.svg)



# CDP
Duck Central is tightly coupled to the repository owned by Project OWL: https://github.com/Call-for-Code/ClusterDuck-Protocol

This is a separate project designed to work with [custom hardware](https://github.com/QuackHeads-Dev-CalPoly/Quacker-Design) that is entirely written in C and C++, with no dependencies on Arduino.



# How to contribute

### Introduction
This repository uses CMake as its build system. CMake is the build system supported by Raspberry Pico, and works wonderfully when used correctly. Documentation for CMake can be found here: https://cmake.org/cmake/help/v3.20/

### Directory structure
#### `src`
The `src` directory is where all source code will live. Each subdirectory here is a CMake "library" and within each of those is a `test` directory. This test directory is for unit testing the library, and should never use libraries other than the library under test. The only exception to this rule is standard or pico libraries.

#### `include`
The `include` directory is where all generic include files will live. This is specifically for widely applicable `#define`s that are used in many, if not all, libraries and test files.

#### `tst`
The `tst` directory is where all *system* test code will live. This is not the place for unit tests. Unit tests belong in `src/<library>/test`. System tests pull in multiple libraries such as LoRa and GPS. These are designed for demos or tests that work together to demonstrate interoperability between libraries.


### How to add a new library
 
 1. Add a subdirectory for your library in `src/`.
 1. Write an `add_subdirectory` statement to the `CMakeLists.txt` file in `src/`.
 1. Add a `CMakeLists.txt` file in your library directory. This should be similar to other library CMakeLists.txt files.
 1. Add a `test` directory in your library directory. This is for your unit tests.
 1. Add a `CMakeLists.txt` to that test directory for the test executable(s).


### How to add a new system test or system demo

 1. Write your test/demo in `tst/`. This should be only **ONE** file. If you absolutely need to write multiple files for your test, you should make a subdirectory in `tst/`
 1. Edit the `tst/CMakeLists.txt` file and add your new demo in a way that is similar to existing tests.


### How to push changes
The main branch is locked down so that all changes must come through a pull request. This is to ensure that Code Reviews are performed on all changes. To create a new pull request, follow these steps:

 1. Write your code on a new local branch.
 1. Make your changes.
 1. Add and commit your changes. There should be one commit per pull request, unless you have  a good reason to do otherwise.
 1. `git fetch`
 1. `git rebase`
 1. If an error just popped up about tracking an upstream branch, you will need to set your current local branch to track from `origin/main`.
 1. `git push origin HEAD:<some remote branch>`. This can be an existing remote branch (that is yours!) or a new remote branch. If it is new, give the new remote branch a sensical name. Some people use their name, i.e. `lard4-dev`, or the issue name, or something similar.
 1. The git CLI will output a link to create a new pull request if it is a new branch. Copy/paste this into your browser to create the pull request. If the branch exists, open up GitHub's website and create the new pull request.
 1. Write a message in the pull request.
 1. Add reviewers (assignees) on the toolbar to the right.
 1. Create the pull request.
 1. Get reviewed.
 1. Once approved, **Squash and merge**. This will create one commit and merge it into `main` cleanly.