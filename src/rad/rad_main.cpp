/*

  {_______            {_          {______
        {__          {_ __               {__
        {__         {_  {__               {__
     {__           {__   {__               {__
 {______          {__     {__              {__
       {__       {__       {__            {__   
         {_________         {______________		Expansion Unit
                
 RADExp - A framework for DMA interfacing with Commodore C64/C128 computers using a Raspberry Pi Zero 2 or 3A+/3B+
 Copyright (c) 2022 Carsten Dachsbacher <frenetic@dachsbacher.de>

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include <cstdio>
#include <circle/logger.h>
#include "kernel.h"

// required for C128
#define FORCE_RESET_VECTORS

#include "rad_main.h"
//#include "dirscan.h"
#include "c64screen.h"
//#include "linux/kernel.h"
#include "config.h"

static const char DRIVE[] = "SD:";
static const char FILENAME_CONFIG[] = "SD:RAD/rad.cfg";

// REU
//#include "rad_reu.h"
//#define REU_MAX_SIZE_KB	(16384)
//u8 mempool[ REU_MAX_SIZE_KB * 1024 + 8192 ] AAA = {0};
u8 prgLaunch[ 65536 + 2 ] AAA = {0};

// low-level communication code
u64 armCycleCounter;
#include "lowlevel_dma.h"

// GeoRAM
//#include "rad_georam.h"

#if 0
void warmCache()
{
	for ( int i = min( REU_SIZE_KB, 256 ) * 1024 - 64; i >= 0; i -= 64 )
		CACHE_PRELOADL2KEEP( &mempool[ i ] );

	CACHE_PRELOAD_DATA_CACHE( &reu, sizeof( REUSTATE ), CACHE_PRELOADL1KEEP )
	FORCE_READ_LINEAR32a( &reu, sizeof( REUSTATE ), sizeof( REUSTATE ) * 8 );

	CACHE_PRELOAD_INSTRUCTION_CACHE( (void*)reuUsingPolling, 1024 * 7 );
	FORCE_READ_LINEARa( (void*)reuUsingPolling, 1024 * 7, 65536 );
}

void warmCacheGeoRAM()
{
	for ( int k = 0; k < 1024; k += 64 )
		CACHE_PRELOADL2STRM( &mempool[ k ] );

	FORCE_READ_LINEARa( &mempool[ 0 ], 1024, 1024 * 64 );

	CACHE_PRELOAD_INSTRUCTION_CACHE( (void*)geoRAMUsingPolling, 1024 * 2 );
	FORCE_READ_LINEARa( (void*)geoRAMUsingPolling, 1024 * 2, 65536 );
}
#endif

static u16 resetVector = 0xFCE2;

// emulate GAME-cartridge to start C128 (also works on C64) with custom reset-vector => forces C128 in C64 mode
void startForcedResetVectors()
{
	register u32 g2, g3;

	const u8 romh[] = { 
		0x4c, 0x0a, 0xe5, 0x4c, 0x00, 0xe5, 0x52, 0x52,
		0x42, 0x59, 0x43, 0xfe, 0xe2, 0xfc, 0x48, 0xff };

	CACHE_PRELOAD_INSTRUCTION_CACHE( (void*)startForcedResetVectors, 1024 * 4 );
	CACHE_PRELOADL1STRM( romh );
	FORCE_READ_LINEARa( (void*)startForcedResetVectors, 1024 * 4, 65536 );
	FORCE_READ_LINEARa( (void*)romh, 16, 1024 );

	OUT_GPIO( DMA_OUT );
	OUT_GPIO( GAME_OUT );

	WAIT_FOR_CPU_HALFCYCLE
	WAIT_FOR_VIC_HALFCYCLE
	RESTART_CYCLE_COUNTER
	WAIT_UP_TO_CYCLE( 100 );
	SET_GPIO( bLATCH_A_OE | bIRQ_OUT | bOE_Dx | bRW_OUT | bDMA_OUT | bDIR_Dx );
	INP_GPIO_RW();
	INP_GPIO_IRQ();

	CLR_GPIO( bGAME_OUT );
	CLR_GPIO( bMPLEX_SEL );

	DELAY( 1 << 20 );
	SET_GPIO( bRESET_OUT );
	INP_GPIO( RESET_OUT );

	u32 nCycles = 0, nRead = 0;
	while ( 1 )
	{
		WAIT_FOR_CPU_HALFCYCLE
		RESTART_CYCLE_COUNTER
		WAIT_UP_TO_CYCLE( 50 );
		RESTART_CYCLE_COUNTER
		WAIT_UP_TO_CYCLE( WAIT_FOR_SIGNALS );
		g2 = read32( ARM_GPIO_GPLEV0 );

		SET_GPIO( bMPLEX_SEL );
		WAIT_UP_TO_CYCLE( WAIT_CYCLE_MULTIPLEXER );
		g3 = read32( ARM_GPIO_GPLEV0 );
		CLR_GPIO( bMPLEX_SEL );

		if ( nCycles ++ > 100000 )
		{
			OUT_GPIO( RESET_OUT );
			CLR_GPIO( bRESET_OUT );
			DELAY( 1 << 18 );
			SET_GPIO( bRESET_OUT );
			INP_GPIO( RESET_OUT );
			nRead = nCycles = 0;
		}

		if ( ROMH_ACCESS && CPU_READS_FROM_BUS )
		{
			u8 d = 0;
			if ( ADDRESS0to7 == 0xfc || ADDRESS0to7 == 0xfd ) nRead ++;
			if ( ADDRESS0to7 == 0xfc ) d = resetVector & 255;
			if ( ADDRESS0to7 == 0xfd ) d = resetVector >> 8;

			{
				register u32 DD = ( ( d ) & 255 ) << D0;
				write32( ARM_GPIO_GPCLR0, ( D_FLAG & ( ~DD ) ) | bOE_Dx | bDIR_Dx );
				write32( ARM_GPIO_GPSET0, DD );
				SET_BANK2_OUTPUT
				WAIT_UP_TO_CYCLE( WAIT_CYCLE_READ );
				SET_GPIO( bOE_Dx | bDIR_Dx );
			}

			if ( nRead >= 2 )
			{
				WAIT_FOR_VIC_HALFCYCLE
				SET_GPIO( bGAME_OUT );
				break;
			}
		}
		WAIT_FOR_VIC_HALFCYCLE
		RESET_CPU_CYCLE_COUNTER
	}
}


//#define REU_PROTOCOL
#ifdef REU_PROTOCOL
REUPROT reuProtocol[ 65536 ];
u32 nReuProtocol = 0;
#endif

#include "rad_hijack.h"

//volatile u8 bla = 0;
CLogger	*logger;

unsigned fiqRegOffset;
u32		 fiqRegMask;
u32 temperature;

#define WAIT_FOR_READY_PROMPT \
	int done;												\
	do {													\
		for ( u32 i = 0; i < 200000; i++ ) 					\
			emuWAIT_FOR_VIC_HALFCYCLE						\
		done = checkForReadyPrompt( !go64mode );			\
	} while ( !done );


REUSTATE reu;
void initREU( void )
{
	reu.WAIT_FOR_SIGNALS = WAIT_FOR_SIGNALS;
	reu.WAIT_CYCLE_MULTIPLEXER = WAIT_CYCLE_MULTIPLEXER;
	reu.WAIT_CYCLE_READ = WAIT_CYCLE_READ;
	reu.WAIT_CYCLE_WRITEDATA = WAIT_CYCLE_WRITEDATA;
	reu.WAIT_CYCLE_READ2 = WAIT_CYCLE_READ + 20;
	reu.WAIT_CYCLE_READ_VIC2 = WAIT_CYCLE_READ_VIC2;
	reu.WAIT_CYCLE_WRITEDATA_VIC2 = WAIT_CYCLE_WRITEDATA_VIC2;
	reu.WAIT_CYCLE_MULTIPLEXER_VIC2 = WAIT_CYCLE_MULTIPLEXER_VIC2;
	reu.WAIT_TRIGGER_DMA = WAIT_TRIGGER_DMA;
	reu.WAIT_RELEASE_DMA = WAIT_RELEASE_DMA;
	reu.TIMING_OFFSET_CBTD = TIMING_OFFSET_CBTD;
	reu.TIMING_DATA_HOLD = TIMING_DATA_HOLD;
	reu.TIMING_TRIGGER_DMA = TIMING_TRIGGER_DMA;
	reu.TIMING_ENABLE_ADDRLATCH = TIMING_ENABLE_ADDRLATCH;
	reu.TIMING_READ_BA_WRITING = TIMING_READ_BA_WRITING;
	reu.TIMING_ENABLE_RWOUT_ADDR_LATCH_WRITING = TIMING_ENABLE_RWOUT_ADDR_LATCH_WRITING;
	reu.TIMING_ENABLE_DATA_WRITING = TIMING_ENABLE_DATA_WRITING;
	reu.TIMING_BA_SIGNAL_AVAIL = TIMING_BA_SIGNAL_AVAIL;

	reu.CACHING_L1_WINDOW_KB = CACHING_L1_WINDOW_KB * 1024;
	reu.CACHING_L2_OFFSET_KB = CACHING_L2_OFFSET_KB * 1024;
	reu.CACHING_L2_PRELOADS_PER_CYCLE = CACHING_L2_PRELOADS_PER_CYCLE;
}

void CRAD::Run( void )
{
	m_EMMC.Initialize();
	gpioInit();
	setDefaultTimings( AUTO_TIMING_RPI3PLUS_C64C128 );
	readConfig( logger, DRIVE, FILENAME_CONFIG );

logger->Write("RAD_MAIN", LogNotice, "%s: before I/O stuff", __FUNCTION__);
	OUT_GPIO( RESET_OUT );
	CLR_GPIO( bRESET_OUT );
	DELAY( 1 << 25 );
	SET_GPIO( bRESET_OUT );
	INP_GPIO( RESET_OUT );

logger->Write("RAD_MAIN", LogNotice, "%s: before disableIRQs()", __FUNCTION__);
	DisableIRQs();
logger->Write("RAD_MAIN", LogNotice, "%s: after disableIRQs()", __FUNCTION__);

	register u32 g2;

	// this also initializes timing values
	//REU_SIZE_KB = 128;
	initREU();
logger->Write("RAD_MAIN", LogNotice, "before initHijack()");
	initHijack();
logger->Write("RAD_MAIN", LogNotice, "after initHijack()");

#ifdef REU_PROTOCOL
	nReuProtocol = 0;
#endif

	#ifdef FORCE_RESET_VECTORS
	resetVector = 0xfce2;
	#endif

	u32 prgSize = 0;
	u8 isC128PRG = 0;
	u8 go64mode = 0;
	int res = 0;

	DisableIRQs();

	checkIfMachineRunning();		
	DELAY( 1 << 27 );

	u8 justStarted = 1*0;

	while ( 1 )
	{
		prgSize = 0;
		isC128PRG = 0;
		go64mode = 0;
		res = 0;

		extern u32 radStartup, radStartupSize;
		if ( radStartup == 3 )
		{
			goto hijacking;
		}

	radIsWaiting:
		CLR_GPIO( bMPLEX_SEL );

		while ( 1 )
		{
			RESTART_CYCLE_COUNTER						
			WAIT_UP_TO_CYCLE( 1250 );	
			g2 = read32( ARM_GPIO_GPLEV0 );			
	
			if ( BUTTON_PRESSED ) 
				goto hijacking;
		}

logger->Write("RAD_MAIN", LogNotice, "hijacking...");
	hijacking:
		temperature = m_CPUThrottle.GetTemperature();

		SyncDataAndInstructionCache();
		CACHE_PRELOAD_INSTRUCTION_CACHE( (void*)hijackC64, 1024 * 10 );
		FORCE_READ_LINEARa( (void*)hijackC64, 1024 * 10, 65536 );

		extern u8 justBooted;
		justBooted = 1;

		///////////////////////////////////////////////////////////////////////
		//
		// goto menu
		//
		///////////////////////////////////////////////////////////////////////

		res = hijackC64( justStarted );			// after hijackC64 the CPU is still halted by DMA
		justStarted = 0;

		/*radLaunchPRG = 1;
		sprintf( radLaunchPRGFile, "SD:RAD_PRG/reu-checker v1.0.prg" );
		res = RUN_MEMEXP + 1;
		radLoadREUImage = 0;
		meSize0 = 4;*/

		WAIT_FOR_CPU_HALFCYCLE
		WAIT_FOR_VIC_HALFCYCLE
		RESTART_CYCLE_COUNTER
		SET_GPIO( bLATCH_A_OE | bIRQ_OUT | bGAME_OUT | bOE_Dx | bRW_OUT | bDMA_OUT );
		INP_GPIO( RW_OUT );
		INP_GPIO( IRQ_OUT );
		OUT_GPIO( RESET_OUT );
		CLR_GPIO( bRESET_OUT );

		// do we want to boot a C128 in C64-mode?
		go64mode = 0;

		// load PRG (if any) to figure out whether we want to force a C128 into C64-mode
		prgSize = 0;
		if ( radLaunchPRG )
		{
			readFile( logger, (char*)DRIVE, (char*)radLaunchPRGFile, prgLaunch, &prgSize );
			isC128PRG = *(u16*)prgLaunch == 0x1c01 ? 1 : 0;

			if ( isC128 )
			{
				if ( !isC128PRG && radLaunchPRG_NORUN_128 && !( *(u16 *)prgLaunch == 0x0801 ) )
					go64mode = 0; else
				{
					if ( ( !isC128PRG && radLaunchPRG ) || radLaunchGEORAM )
						go64mode = 1;
				}
			}

			// if we're not launching automatically: be nice and print load address
			if ( !isC128PRG && !( *(u16 *)prgLaunch == 0x0801 ) )
			{
				char *strAddr = strstr( &statusMsg[ 120 ], "$0000)" );
				char tmp[ 40 ];
				sprintf( tmp, "$%04X", *(u16 *)prgLaunch );
				for ( int i = 0; i < 5; i++ )
					strAddr[ i ] = tmp[ i ];
				memcpy( &statusMsg[ 80 ], &statusMsg[ 120 ], 40 );
			}
		}

		//
		// let the machine start booting (meanwhile we'll load or initialize the REU/GeoRAM image and emulation)
		//
		CLR_GPIO( bMPLEX_SEL );
	#ifdef FORCE_RESET_VECTORS
		if ( go64mode )
		{
			startForcedResetVectors();
		} else
	#endif
		{
			DELAY( 1 << 18 );
			SET_GPIO( bRESET_OUT );
			INP_GPIO( RESET_OUT );
		}

		if ( res == RUN_REBOOT )
		{
			reboot(); 
		} else
		///////////////////////////////////////////////////////////////////////
		//
		// no memory expansion (but possibly PRG start)
		//
		///////////////////////////////////////////////////////////////////////
		if ( res == RUN_MEMEXP + 3 ) // no memory expansion
		{
			// wait for "READY." to appear on screen
			WAIT_FOR_READY_PROMPT

			if ( radLaunchPRG )
				injectAndStartPRG( prgLaunch, prgSize, false ); else
			{
				#ifdef STATUS_MESSAGES
				setStatusMessage( &statusMsg[ 0 ], "RAD DISABLED" );
				setStatusMessage( &statusMsg[ 80 ], " " );
				injectMessage( false );
				#endif
			}

			goto radIsWaiting;
		}

		OUT_GPIO( GAME_OUT );
		OUT_GPIO( DMA_OUT );
		OUT_GPIO_IRQ();
		SET_GPIO( bLATCH_A_OE | bIRQ_OUT | bGAME_OUT | bOE_Dx | bRW_OUT | bDMA_OUT | bDIR_Dx );
		INP_GPIO_RW();
		INP_GPIO_IRQ();

		goto hijacking;
	}
}

__attribute__((optimize("align-functions=256"))) void CRAD::FIQHandler( void *pParam )
{
}

int rad_main( CKernel &main_kernel )
{
	CRAD kernel(main_kernel.get_isys(), main_kernel.get_timer(), main_kernel.get_logger());
	if ( kernel.Initialize() )
		kernel.Run();

	halt();
	return EXIT_HALT;
}

