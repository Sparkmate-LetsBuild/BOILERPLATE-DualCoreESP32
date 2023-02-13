# DualCore Task Management on the ESP32
_⚡ a Sparkmate Let's Build boiler-plate ⚡_


# Why use Dual Core / Task Management

If you've worked with Arduinos/ESP32 before you'll be familiar with the conventional code flow:

<image src="./readme_assets/Setup_Loop_flow.jpg" width="500" /><br>

This is great for when you have a predictable, repeatable procedure that runs forever after an initial setup stage, e.g. you set up your sensors, then periodically poll your sensors and stream the results to the cloud over 4G.

But some projects require multiple "tasks" to run in parallel after an initial setup. In a task-based system, each task is assigned to do a specific job, and they can all run at the same time, independently of each other. This means that different tasks can start, run, and finish without waiting for other tasks to be completed first.

<image src="./readme_assets/Setup_Tasks_flow.jpg" width="500" /><br>

In some cases, you will not only need multiple tasks to run at the same time, but to specificy that a task runs on a specific processing core. You might do this so as to dedicate an entire core to a critical and intense task, or to limit access to components/communications-lines to a specific core (to ensure two cores aren't trying to access the same component at the same time).

<image src="./readme_assets/Setup_TasksOnCores_flow.jpg" width="500" /><br>

# Tasks explanation (multi-threading behaviour)

On the ESP32, tasks are part of the [FreeRTOS framework](https://www.freertos.org/index.html), whereby we run multiple tasks on the ESP32 at the same time, and the framework evaluates what can/should be run at any given time.

We use a combination of scheduler-handled tasks and verbosely ordered tasks.

**For verbosely ordered tasks** (such as `c0_sniffAndFillBuffer`) we shouldn't be filling the Upload Buffer with data before we've sniffed data (it not only doesn't make sense to, and it can lead to nasty results), and so we combine these two subtasks under one main task which we instruct to run continuously on a dedicated core.

As such, the safest way to ensure that a sub-task B always happens after a sub-task A is to verbosely tell A to run then B to run in code flow, and not trust a scheduler to handle this.

For example,
In [main_task_handler.h](./include/main_task_handler.h):

```cpp
void Tasks::c0_sniffAndFillBuffer(void *param)
{
    for (;;)
    {
        DataSniffingTasks::sniff(20); // You must sniff
        if (DATA_UPLOAD_ENABLED)
        {
            DataSniffingTasks::setUploadHistoryChanges(&UploadBufferStream); // Then update and previous upload changes
            DataSniffingTasks::fillUploadBuffer(&UploadBufferStream); // Then fill the Upload Buffer
        }
    }
}
```

These sub-tasks can be found in the files in [tasks](./include/tasks/) folder. In the example above, all of the sub-tasks can be found in [data_sniffing.h](./include/tasks/data_sniffing.h) where we then leverage specific code from bricks, for example (simplified code):

```cpp
    void sniff(int time_limit = 3000)
    {
        unsigned long int time_now = millis();
        FileHandler::openFileForAppend(SESSION_FILE_NAME.c_str());
        while (millis() < time_now + (time_limit))
        {
            NMEA2000.ParseMessages();
            Accelerometer.checkAccel();
        }
        FileHandler::closeFileForAppend(SESSION_FILE_NAME.c_str());
    }
```

**For scheduler-handled tasks** (such as `c1_updateRTCFromNetwork`), we want this task to run whenever the Uploader is not busy, and CORE 1 has some availability. We set this to the same priority as `c1_uploadFromUploadBuffer`, and allow the scheduler to determine when it makes sense to run it. We try to update the RTC at the beginning of operations, and if we were not successful we "sleep the task" for 20s and wait to try again later. Once this task has successfully completed we delete it, i.e. we only update the RTC once per session.


# Code structure

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

**Why no src (.cpp) files?**

[You don't need both anymore!](https://stackoverflow.com/questions/1305947/why-does-c-need-a-separate-header-file#:~:text=The%20answer%20is%20that%20C%2B%2B,everything%20in%20the%20header%20files.) _(if you're careful, and you use `pragma once`)_

# Using other Sparkmate Let's Build Bricks

In order to keep it very clear which function belongs to which brick we've adopted Namespaces. For example `TimeHandler::setRTCFromEpoch` found in [bricks/time_handler.h](./include/bricks/time_handler.h), which is responsible for setting the time on the Real Time Clock from an epoch time. We have done this entirely for the sake of human readibility, not compilation/performance, etc.

You will find that all bricks will use a PascalCased namespace reflective of the filename. The exception is [optimized_subsampler\_\_c.h](./include/bricks/optimized_subsampler__c.h) which is a class, not a namespace, as we have multiple subsamplers for different session uploads.

Namespaces act like objects (from a human readibility perspective) and so are essentially treated as non-replicable (i.e. we only have one `FileHandler` brick, only one `UploadsCacher` brick, etc).

In the case of the Optimized Subsampler, we create two subsamplers at runtime. One for the working SESSION file, and one for any old sessions we're uploading. This keeps us uploading only the most relevant data.

# The Sparkmate Let's Build open source policy

This boilerplate is open source, reusable, and hackable by design. **We want you to build** with it. We don't ask for credit, attribution, funding, or anything like that.
We just want you to make something cool, and we hope we've helped 😉.

Please add any issues or MRs as you see fit.

Get in touch with [letsbuild@sparkmate.co](mailto:letsbuild@sparkmate.co) for any questions or comments.