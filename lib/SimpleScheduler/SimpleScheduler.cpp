
#include "SimpleScheduler.h"

Task* first = 0; /* The first task in the linked list */
Task* currentTask = 0;

/* Contructor */
Task::Task(void (*func) (void), uint32_t inter, bool en) {
    interval = inter;
    function = func;
    enabled = en;
    nextExecution = millis() + interval;
    next = first;
    first = this;
}

void Task::runTasks() {
    /* If no task, start over with the list */
    if (currentTask == 0) {
        currentTask = first;
        return;
    }

    /* Run the task when enabled and time is reached */
    if (currentTask->enabled && (millis() > currentTask->nextExecution)) {
        currentTask->function();
        currentTask->nextExecution += currentTask->interval;
    }

    /* Jump to next task */
    currentTask = currentTask->next;
}

void Task::enable() {
    /* Only reset time if not enabled */
    if(!enabled) {
        enabled = true;
        nextExecution = millis();
    }
}

void Task::disable() {
    enabled = false;
}

void Task::setInterval(uint32_t inter) {
    /* Update execution time correctly */
    nextExecution += inter - interval;
    interval = inter;
}

uint32_t Task::getInterval() {
    return interval;
}

void Task::executeIn(uint32_t milliseconds) {
    nextExecution = millis() + milliseconds;
    enabled = true;
}

Task::~Task() {
    Serial.println("Deleting tasks is not implemented. Use 'disable()' instead");
}
