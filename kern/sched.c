#include <inc/assert.h>
#include <inc/x86.h>
#include <kern/env.h>
#include <kern/monitor.h>


struct Taskstate cpu_ts;
_Noreturn void sched_halt(void);

/* Choose a user environment to run and run it */
_Noreturn void
sched_yield(void) {
    /* Implement simple round-robin scheduling.
     *
     * Search through 'envs' for an ENV_RUNNABLE environment in
     * circular fashion starting just after the env was
     * last running.  Switch to the first such environment found.
     *
     * If no envs are runnable, but the environment previously
     * running is still ENV_RUNNING, it's okay to
     * choose that environment.
     *
     * If there are no runnable environments,
     * simply drop through to the code
     * below to halt the cpu */

    // LAB 3: Your code here:
    //    env_run(&envs[0]);

    int current_position = 0;
    if (curenv) {
        // из env.h:
        // * The environment index ENVX(eid) equals the environment's offset in the
        // * 'envs[]' array.  The uniqueifier distinguishes environments that were
        // * created at different times, but share the same environment index.
        current_position = ENVX(curenv->env_id) + 1;
    }

    // * Search through 'envs' for an ENV_RUNNABLE environment in
    // * circular fashion starting just after the env was
    // * last running.  Switch to the first such environment found.
    for (int i = 0; i < NENV; i++) {
        int real_position = (current_position + i) % NENV;
        if (envs[real_position].env_status == ENV_RUNNABLE) {
            env_run(&envs[real_position]);
        }
    }
    // * If no envs are runnable, but the environment previously
    // * running is still ENV_RUNNING, it's okay to
    // * choose that environment.
    if (curenv && curenv->env_status == ENV_RUNNING) {
        env_run(curenv);
    }

    cprintf("Halt\n");

    /* No runnable environments,
     * so just halt the cpu */
    sched_halt();
}

/* Halt this CPU when there is nothing to do. Wait until the
 * timer interrupt wakes it up. This function never returns */
_Noreturn void
sched_halt(void) {

    /* For debugging and testing purposes, if there are no runnable
     * environments in the system, then drop into the kernel monitor */
    int i;
    for (i = 0; i < NENV; i++)
        if (envs[i].env_status == ENV_RUNNABLE ||
            envs[i].env_status == ENV_RUNNING) break;
    if (i == NENV) {
        cprintf("No runnable environments in the system!\n");
        for (;;) monitor(NULL);
    }

    /* Mark that no environment is running on CPU */
    curenv = NULL;

    /* Reset stack pointer, enable interrupts and then halt */
    asm volatile(
            "movq $0, %%rbp\n"
            "movq %0, %%rsp\n"
            "pushq $0\n"
            "pushq $0\n"
            "sti\n"
            "hlt\n" ::"a"(cpu_ts.ts_rsp0));

    /* Unreachable */
    for (;;)
        ;
}
