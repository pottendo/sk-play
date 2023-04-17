/*
  _________.__    .___      __   .__        __            _____                       
 /   _____/|__| __| _/____ |  | _|__| ____ |  | __       /     \   ____   ____  __ __ 
 \_____  \ |  |/ __ |/ __ \|  |/ /  |/ ___\|  |/ /      /  \ /  \_/ __ \ /    \|  |  \
 /        \|  / /_/ \  ___/|    <|  \  \___|    <      /    Y    \  ___/|   |  \  |  /
/_______  /|__\____ |\___  >__|_ \__|\___  >__|_ \     \____|__  /\___  >___|  /____/ 
        \/         \/    \/     \/       \/     \/             \/     \/     \/       
 
 kernel_menu20.h

 Sidekick64 - A framework for interfacing 8-Bit Commodore computers (C64/C128,C16/Plus4,VC20) and a Raspberry Pi Zero 2 or 3A+/3B+
            - Sidekick Menu: ugly glue code to expose some functionality in one menu with browser
 Copyright (c) 2019-2022 Carsten Dachsbacher <frenetic@dachsbacher.de>

 Logo created with http://patorjk.com/software/taag/
 
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

#ifndef _kernel_menu20_h
#define _kernel_menu20_h

// use the OLED connected to the latch
#define USE_OLED

// 0 = SSD1306 OLED 128x64
// 1 = ST7789 RGB TFT 240x240
extern int screenType;

//#define COMPILE_MENU_WITH_SOUND
//#define USE_VCHIQ_SOUND

#define USE_LATCH_OUTPUT

//#define USE_HDMI_VIDEO

#include <circle/startup.h>
#include <circle/bcm2835.h>
#include <circle/memio.h>
#include <circle/memory.h>
#include <circle/koptions.h>
#include <circle/devicenameservice.h>
#include <circle/screen.h>
#include <circle/interrupt.h>
#include <circle/timer.h>
#include <circle/logger.h>
#include <circle/sched/scheduler.h>
#include <circle/types.h>
#include <circle/gpioclock.h>
#include <circle/gpiopin.h>
#include <circle/gpiopinfiq.h>
#include <circle/gpiomanager.h>
//#include <circle/pwmsoundbasedevice.h>
//#include <circle/i2ssoundbasedevice.h>
#include <circle/util.h>

#include <circle/dmachannel.h>
#include <circle/spinlock.h>


#include <SDCard/emmc.h>
#include <fatfs/ff.h>

#include <stdio.h>

#ifdef USE_VCHIQ_SOUND
#include <vc4/vchiq/vchiqdevice.h>
#include <vc4/sound/vchiqsoundbasedevice.h>
#endif

#include "lowlevel_arm64.h"
#include "gpio_defs.h"
#include "latch.h"
#include "helpers.h"
#include "crt.h"
//#include "sound.h"

#include "oled.h"
#include "splash_sk20.h"
#include "tft_st7789.h"

#include "config.h"

extern CLogger *logger;

void startInjectCode();
void injectPOKE( u32 a, u8 d );

class CKernelMenu
{
public:
	CKernelMenu( CInterruptSystem *intsys, CTimer *timer, CLogger *l, CScreenDevice *sd)
		: m_CPUThrottle( CPUSpeedMaximum ),
		m_InputPin( PHI2, GPIOModeInput, intsys ),
		m_EMMC( intsys, timer, 0 )
	{
		logger = l;
		logger->Write("VIC20", LogNotice, "%s: - 1", __FUNCTION__);
		m_EMMC.Initialize();
		scr = sd;
	}

	~CKernelMenu( void )
	{
	}

	boolean Initialize( void );
	CScreenDevice *get_scrdevice(void) { return scr; }
	void Run( void );

private:
	static void FIQHandler( void *pParam );
	void activateCart( bool imm = true, bool noFadeOut = false );
	void resetVC20();

void WriteSoundData (unsigned nFrames);
void GetSoundData (void *pBuffer, unsigned nFrames);

public:
	// do not change this order
	CMemorySystem		m_Memory;
	CCPUThrottle		m_CPUThrottle;
#ifdef USE_HDMI_VIDEO
//	CScreenDevice		m_Screen;
#endif
	CGPIOPinFIQ		m_InputPin;
	CEMMCDevice			m_EMMC;
	CScreenDevice	*scr;
};

#endif
