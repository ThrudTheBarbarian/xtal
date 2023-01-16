#ifndef NOTIFICATIONS_H
#define NOTIFICATIONS_H


/*************************************************************************\
|* Notification constant: A binary XEX or similar file has been loaded
\*************************************************************************/
#define NTFY_BINARY_LOADED "N:binary loaded"


/*************************************************************************\
|* Notification constant: The simulator is ready to go, pointers included
\*************************************************************************/
#define NTFY_SIM_AVAILABLE "N:simulator available"

/*************************************************************************\
|* Notification constant: Buttons were pressed
\*************************************************************************/
#define NTFY_BTN_PLAY_BACK		"N:button:play back"
#define NTFY_BTN_STEP_BACK		"N:button:step back"
#define NTFY_BTN_STOP			"N:button:stop"
#define NTFY_BTN_STEP_FORWARD	"N:button:step forward"
#define NTFY_BTN_PLAY_FORWARD	"N:button:play forward"

/*************************************************************************\
|* Notification constant: Worker entered a new state
\*************************************************************************/
#define NTFY_WRK_PLAY_FORWARD	"N:wrk:play forward"

/*************************************************************************\
|* Notification constant: Trace selection changed
\*************************************************************************/
#define NTFY_TRACE_SEL_CHG		"N:trace:sel:chg"

/*************************************************************************\
|* Notification constant: Assembly selection changed
\*************************************************************************/
#define NTFY_ASM_SEL_CHG		"N:asm:sel:chg"

/*************************************************************************\
|* Notification constant: Assembly complete
\*************************************************************************/
#define NTFY_ASM_DONE			"N:asm:done"

/*************************************************************************\
|* Notification constant: Prepare to simulate
\*************************************************************************/
#define NTFY_SIM_START			"N:sim:start"

/*************************************************************************\
|* Notification constant: Simulation complete
\*************************************************************************/
#define NTFY_SIM_DONE			"N:sim:done"

#endif // NOTIFICATIONS_H
