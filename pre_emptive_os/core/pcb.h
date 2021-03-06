/******************************************************************************
 *
 * Copyright:
 *    (C) 2000 - 2005 Embedded Artists AB
 *
 * Description:
 *
 * ESIC:
 *    pre_emptive_os
 *
 * Version:
 *    1.4.0
 *
 * Generate date:
 *    2005-03-15 at 20:27:15
 *
 * NOTE:
 *    DO NOT EDIT THIS FILE. IT IS AUTO GENERATED.
 *    CHANGES TO THIS FILE WILL BE LOST IF THE FILE IS RE-GENERATED
 *
 * Signature:
 *   7072655F656D70746976655F6F73,312E342E302E30,020235
 *   ,35,10104021013134373435363030,07323838,3732,01013
 *   830,0101013138303030303030,3135,33,3135,0232323530
 *   ,01020130,0231343734353539,3238313831,020101100302
 *   103030310010133,0163130,3230,3330,3430,3530,3630,3
 *   730,3830,3930,313030,313130,313230,313330,313430,3
 *   13530,313630,,35,35,35,35,35,35,35,35,35,35,35,35,
 *   35,35,35,35,,,,1001001100011000000000]484152445741
 *   5245,4C5043323130365F32,545538,756E7369676E6564206
 *   3686172,414C49474E4D454E54,34,54424F4F4C,756E73696
 *   76E65642063686172,54553332,756E7369676E656420696E7
 *   4,544D505F46494C4553,2A2E656C663B2A2E6C73743B2A2E6
 *   D61703B2A2E6F3B2A2E6F626A3B2A2E64,454E4449414E,4C4
 *   954544C45,54533332,7369676E656420696E74,545338,736
 *   9676E65642063686172,54553136,756E7369676E656420736
 *   86F7274,54533136,7369676E65642073686F7274,44455343
 *   52495054494F4E,,44454255475F4C4556454C,30,434F4445
 *   5F524F4F54,,47454E5F52554C4553,,4C494E455F5445524D
 *   ,43524C46,4252414345,,43524541544F52,416E646572732
 *   0526F7376616C6C,4352454154494F4E5F44415445,3230303
 *   52D30332D31352032303A31373A3432,524F4F54,433A2F446
 *   F63756D656E747320616E642053657474696E67732F416E646
 *   5727320526F7376616C6C2F4D696E6120646F6B756D656E742
 *   F456D62656464656420417274697374732F50726F647563747
 *   32F4C50433231303620525332333220517569636B537461727
 *   420426F6172642F72746F732F]505245464958,,4445425547
 *   5F4C4556454C,30,555345525F434F4D4D454E54,]64656661
 *   756C74,
 *
 * Checksum:
 *    96480
 *
 *****************************************************************************/

#ifndef _PCB__h
#define _PCB__h
/******************************************************************************
 * Includes
 *****************************************************************************/

#include "../api/general.h"
/******************************************************************************
 * Defines, macros, and typedefs
 *****************************************************************************/

/* processor states (flag) */
#define PROC_ENDED      0x01
#define PROC_ACTIVE     0x02
#define PROC_SLEEP      0x04
#define PROC_EVENT_WAIT 0x08
#define PROC_SUSPENDED  0x10
#define PROC_SIG_WAIT   0x20

/* process control block */
typedef struct _tOSPCB__ {
    tU8 *pStk;
    /* pointer to top of stack */
    struct _tOSPCB__ *pNextPrioQueueReady;
    /* next pointer in a prioritized ready queue */
    struct _tOSPCB__ *pPrevPrioQueueReady;
    /* prev pointer in a prioritized ready queue */
    struct _tOSPCB__ *pNextPrioQueueEvent;
    /* next pointer in a prioritized event queue */
    struct _tOSPCB__ *pPrevPrioQueueEvent;
    /* prev pointer in a prioritized event queue */
    struct _tOSPCB__ *pNextTimeQueue;
    /* next pointer in time queue */
    tU8 pid;
    /* process id */
    tU8 prio;
    /* process priority */
    tU8 flag;
    /* flag (PROC_SEM_WAIT, PROC_SLEEP,...) */
    tU32 sleep;
    /* number of ticks to sleep (relative
                    other processes in time list) */

    tU8 *pStkOrg;
    tU16 stackSize;

} tOSPCB;

#endif

