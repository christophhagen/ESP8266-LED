#ifndef __SIMPLE_SCHEDULER_H
#define __SIMPLE_SCHEDULER_H

#include <Arduino.h>

/* VERY simple task scheduling */
class Task {
    /* Pointer to next task */
    Task* next;

    /* The interval and timeStamp of execution */
    uint32_t interval;

    /* The timeStamp of execution */
    uint32_t nextExecution;

    /* If the task should be executed */
    bool enabled;

    /* The function to be executed */
    void (*function) (void);

public:

    /* Contructor */
    Task(void (*func) (void), uint32_t inter, bool enabled = true);

    /* Used to run the tasks */
    static void runTasks();

    /* Enable or disable the task */
    void enable();
    void disable();

    /* Set and get execution interval */
    void setInterval(uint32_t inter);
    uint32_t getInterval();

    /* Execute the task again after a certain time */
    void executeIn(uint32_t milliseconds);

    /* Deleting is not implemented */
    ~Task();

};

#endif
