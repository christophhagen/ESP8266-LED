#ifndef __SIMPLE_SCHEDULER_H
#define __SIMPLE_SCHEDULER_H

#include <Arduino.h>

/* VERY simple task scheduling */
class Task {
    /* The first task in the linked list */
    static Task* first;

    /* The current task to execute */
    static Task* currentTask;

    /* Pointer to next task */
    Task* next;

    /* The interval and timeStamp of execution */
    uint32_t inter;

    /* The timeStamp of execution */
    uint32_t nextExecution;

    /* If the task should be executed */
    bool enabled;

    /* The function to be executed */
    void (*func) (void);

public:

    /* Constructor */
    Task(void (*function) (void), uint32_t interval, bool enable = true) {
        inter = interval;
        func = function;
        enabled = enable;
        nextExecution = millis() + interval;
        next = first;
        first = this;
    }

    /* Used to run the tasks */
    static void runTasks() {
        /* If no task, start over with the list */
        if (currentTask == 0) {
            currentTask = first;
            return;
        }

        /* Run the task when enabled and time is reached */
        if (currentTask->enabled && (millis() > currentTask->nextExecution)) {
            currentTask->func();
            currentTask->nextExecution += currentTask->inter;
        }

        currentTask = currentTask->next;
    }

    /* Enable or disable the task */
    void enable() {
        if(!enabled) {
            enabled = true;
            nextExecution = millis();
        }
    }

    /* Disable the task */
    void disable() {
        enabled = false;
    }

    /* Returns true, if the task is enabled */
    bool isEnabled() {
        return enabled;
    }

    /* Set execution interval */
    void setInterval(uint32_t interval) {
        nextExecution += interval - inter;
        interval = inter;
    }

    /* Get execution interval */
    uint32_t getInterval() {
        return inter;
    }

    /* Execute the task again after a certain time */
    void executeIn(uint32_t milliseconds) {
        nextExecution = millis() + milliseconds;
        enabled = true;
    }

    /* Deleting is not implemented */
    ~Task() {
        Serial.println("Delete unimplemented. Use 'disable()'");
    }
};

/* Initialize static members */
Task* Task::first = 0;
Task* Task::currentTask = 0;

#endif
