
#include "hw_platform.h"
#include "plic.h"
#include "core_timer.h"

#define MTIMECMP_BASE_ADDR     0x44004000UL
#define MTIME_ADDR             0x4400BFF8UL

#define RTC_PRESCALER 100

void SysTick_Handler(void);

extern volatile uint32_t g_10ms_count;

/******************************************************************************
 * CoreTimer instance data.
 *****************************************************************************/
static timer_instance_t g_timer0;

/******************************************************************************
 * PLIC instance data.
 *****************************************************************************/
static plic_instance_t g_plic;

/******************************************************************************
 * Update the value of the mtimecmp to hold the value of mtime 10ms in the
 * future. This allows generating a timer interrupt with a 10ms tick. 
 *****************************************************************************/
static void add_10ms_to_mtimecmp(void)
{
    volatile uint64_t * mtime       = (uint64_t*) MTIME_ADDR;
    volatile uint64_t * mtimecmp    = (uint64_t*) MTIMECMP_BASE_ADDR;
    uint64_t now = *mtime;
    uint64_t then = now + ((SYS_CLK_FREQ / RTC_PRESCALER) / 100);
    *mtimecmp = then;
}

/******************************************************************************
 * Initialise interrupts.
 *****************************************************************************/
void interrupts_init(void)
{
    /*
     * Initialize the RISC-V platform level interrupt controller. 
     */
    PLIC_init(&g_plic, PLIC_BASE_ADDR, PLIC_NUM_SOURCES, PLIC_NUM_PRIORITIES);
    
    /*
     * Initialize the timer interrupt
     */
    TMR_init(&g_timer0,
             CORETIMER0_BASE_ADDR,
             TMR_CONTINUOUS_MODE,
             PRESCALER_DIV_1024, // (83MHZ / 1024) ~ 83kHz
             810); // (83kHz / 810) ~ 10ms
           
    // In this version of the PLIC, the priorities are fixed at 1.
    // Lower numbered devices have higher priorities.
    // But this code is given as an
    // example.
    PLIC_set_priority(&g_plic, TIMER0_IRQn, 1);  

    // Enable Timer 1 & 0 Interrupt
    PLIC_enable_interrupt(&g_plic, TIMER0_IRQn);  

    // Enable the Machine-External bit in MIE
//    set_csr(mie, MIP_MEIP);
    
    add_10ms_to_mtimecmp();

    // Enable the Machine-Timer bit in MIE
    set_csr(mie, MIP_MTIP);


    // Enable interrupts in general.
    set_csr(mstatus, MSTATUS_MIE);

    // Enable the timers...
    TMR_enable_int(&g_timer0);

    // Start the timer
    TMR_start(&g_timer0);
    
    /*
     * Start the timer.
     */
    TMR_start( &g_timer0 );
}

/******************************************************************************
 * Shutdown interrupts.
 *****************************************************************************/
void interrupts_deinit(void)
{
    /*b inter   
     * Stop the timer.
     */
    TMR_stop( &g_timer0 );
    
    /*
     *  Disable interrupts in general.
     */
    clear_csr(mstatus, MSTATUS_MIE);
    
    /*
     * Disable the Machine-External bit in MIE.
     */
    clear_csr(mie, MIP_MEIP);
    
    /*
     * Initialize the RISC-V platform level interrupt controller. 
     */
    PLIC_init(&g_plic, PLIC_BASE_ADDR, PLIC_NUM_SOURCES, PLIC_NUM_PRIORITIES);
}

/******************************************************************************
 * CoreTimer 0 Interrupt Handler.
 *****************************************************************************/
void Timer0_IRQHandler() {
    uint32_t stable;
    uint32_t gpout;
    
    g_10ms_count += 10;

     /*
      * For neatness, if we roll over, reset cleanly back to 0 so the count
      * always goes up in proper 10s.
      */
    if(g_10ms_count < 10)
        g_10ms_count = 0;
  
    TMR_clear_int(&g_timer0);
}

/******************************************************************************
 * RISC-V interrupt handler for external interrupts.
 *****************************************************************************/
/*Entry Point for PLIC Interrupt Handler*/
void handle_m_ext_interrupt(){
    plic_source int_num  = PLIC_claim_interrupt(&g_plic);
    switch(int_num) {
    case (0):
        break;
    case (External_30_IRQn): 
        Timer0_IRQHandler();
        break;
    case (External_31_IRQn): 
        break;
    default: 
        _exit(10 + (uintptr_t) int_num);
    }
    PLIC_complete_interrupt(&g_plic, int_num);
}

/******************************************************************************
 * RISC-V interrupt handler for machine timer interrupts.
 *****************************************************************************/
void handle_m_timer_interrupt(){

    clear_csr(mie, MIP_MTIP);

    add_10ms_to_mtimecmp();

    SysTick_Handler();

    // Re-enable the timer interrupt.
    set_csr(mie, MIP_MTIP);
}

