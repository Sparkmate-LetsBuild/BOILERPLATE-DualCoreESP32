# DualCore Task Management on the ESP32

_⚡ a Sparkmate Let's Build boiler-plate ⚡_

## Why use Dual Core / Task Management

If you've worked with Arduinos/ESP32 before you'll be familiar with the conventional code flow:

<image src="./readme_assets/Setup_Loop_flow.jpg" width="500" /><br>

This is great for when you have a predictable, repeatable procedure that runs forever after an initial setup stage, e.g. you set up your sensors, then periodically poll your sensors and push the results to the cloud over 4G.

But some projects require multiple "tasks" to run in parallel after an initial setup. In a task-based system, each task is assigned to do a specific job, and they can all run at the same time, independently of each other. This means that different tasks can start, run, and finish without waiting for other tasks to be completed first.

<image src="./readme_assets/Setup_Tasks_flow.jpg" width="500" /><br>

In some cases, you will not only need multiple tasks to run at the same time, but to specificy that a task runs on a specific processing core. You might do this so as to dedicate an entire core to a critical and intense task, or to limit access to components/communications-lines to a specific core (to ensure two cores aren't trying to access the same component at the same time).

<image src="./readme_assets/Setup_TasksOnCores_flow.jpg" width="500" /><br>

## Tasks explanation (multi-threading behaviour)

On the ESP32, tasks are part of the [FreeRTOS framework](https://www.freertos.org/index.html), whereby we run multiple tasks on the ESP32 at the same time, and the framework evaluates what can/should be run at any given time.

We could use a combination of scheduler-handled tasks and verbosely ordered tasks.

**For verbosely ordered tasks** such as `do_A` then `do_B`, we shouldn't be attempting to do B before A is finished: it not only doesn't make sense to, and it can lead to nasty results, and so we combine these two subtasks under one main task which we instruct to run continuously on a dedicated core.

As such, the safest way to ensure that a sub-task B always happens after a sub-task A is to verbosely tell A to run then B to run in code flow, and not trust a scheduler to handle this.

For example, in `do_A` then `do_B` scenario (where B always follows A):

```cpp
void Tasks::mainActualTask(void *param)
{
    for (;;)
    // because we want this task to keep running again and again
    {
        doA();
        doB();
        vTaskDelay(1000); // Allow the processor to have other tasks do stuff now, and wait for 1 second.
    }
}
```

**For scheduler-handled tasks** such as `doA` but in parallel (or otherwise) `doB`, we want task A to run whenever task B is not busy and visa versa. We could set these to be the same priority as each other, or make one task a higher priority than the other.

```cpp
void Tasks::taskA(void *param)
{
    for (;;)
    // because we want this task to keep running again and again
    {
        doA();
        vTaskDelay(1000); // Task B and other tasks will run
    }
}

void Tasks::taskB(void *param)
{
    for (;;)
    // because we want this task to keep running again and again
    {
        doB();
        vTaskDelay(1000); // Task A and other tasks will run
    }
}
```

## Code structure

`main.cpp` calls the task handler declared in `main_task_handler.h` where we also assign priorities to various tasks.

These tasks are found in the `tasks/` directory and are self-contained tasks that the task-scheduler can call upon whenever there is time to complete the tasks.

The tasks will call upon specific bricks - pieces of firmware specific to specific hardware/low-level functions - found in `bricks/`.
In some cases, it makes sense to initialize part of a brick so it can be used more globally, before defining the rest of the brick. This is only really for the SIMCOM and Firmware/VMU meta bricks as they are so tightly bound, and you can see these initializations in `inits/`.

Config files can be found in `configs`, generally accessible at any level.

This logic means that any level can only call upon it's level or below. A task can call a brick, but a brick can not call a task:

```
src/main.cpp                    ← include/configs/*
↑
include/main_task_handler.h     ← ''
↑
include/tasks/*                 ← ''
↑
include/inits/*                 ← '', (used for select bricks only)
↑
include/bricks/*                ← ''
↑
lib/*                           ← ''
```

# Super fast Accelerometer logging _(example of repo)_

In this repo I have an example that has two intense and critical tasks running in parallel after an initial set up period.
We're going to attempt to read data from an accelerometer at 1kHz and write it to an SD card. The trouble is, making a single core ask the accelerometer for data, waiting for a response, parsing the data, then dumping it (quickly and safely) to an SD card takes a very long time, and with just one core we max out at around 100 Hz only.

So, instead, I can dedicate one core exclusively to talking to and parsing the accelerometer data, and then the ESP32's second core exclusively to dumping that data to the SD card (safely).

Another cool (and critical) benefit of this is that if the SD card starts to slow down momentarily, which often happens when writing long strings or when we need to close and then open the file again to avoid corruption, this slowing down will not affect the accelerometer read frequency because it's handled on a completely separate core.

These two tasks can be found in

```cpp
PeripheralsTasks::c0_bufferWriteToSD()
```

and

```cpp
SensorTasks::c1_writeAccelToBuffer
```

Both are pinned to different cores. And both share a LoopbackStreamBuffer to transfer the data from one side to the other, the:

```cpp
FileHandler::SESSION_Loopback
```

You can explore the code in [main.cpp](./src/main.cpp) to understand how these tasks are assigned and how they work further.

Don't forget to check out a much more informative and involved tutorial at [the FreeRTOS website](https://www.freertos.org/implementation/a00004.html).

# The Sparkmate Let's Build open source policy

This boilerplate is open source, reusable, and hackable by design. **We want you to build** with it. We don't ask for credit, attribution, funding, or anything like that.
We just want you to make something cool, and we hope we've helped 😉.

Please add any issues or MRs as you see fit.

Get in touch with [letsbuild@sparkmate.co](mailto:letsbuild@sparkmate.co) for any questions or comments.
