//
// kernel.h
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
#ifndef _kernel_h
#define _kernel_h

#include <circle/actled.h>
#include <circle/koptions.h>
#include <circle/devicenameservice.h>
#include <circle/screen.h>
#include <circle/serial.h>
#include <circle/exceptionhandler.h>
#include <circle/interrupt.h>
#include <circle/timer.h>
#include <circle/logger.h>
#include <circle/types.h>
#include <circle/sched/scheduler.h>

enum TShutdownMode
{
	ShutdownNone,
	ShutdownHalt,
	ShutdownReboot
};

class CKernel 
{
public:
	CKernel (void);
	boolean Initialize (void);
	TShutdownMode Run (void);
	int Cleanup (void) { return 0; }
	inline void set_pixel(unsigned int x, unsigned int y, TScreenColor c) { mScreen.SetPixel(x, y, c); }
private:
	CActLED			mActLED;
	CKernelOptions		mOptions;
	CDeviceNameService	mDeviceNameService;
	CScreenDevice		mScreen;
	CSerialDevice		mSerial;
	CExceptionHandler	mExceptionHandler;
	CInterruptSystem	mInterrupt;
	CTimer			mTimer;
	CLogger			mLogger;

	CScheduler		mScheduler;
	CSynchronizationEvent	mEvent;

};

#endif


