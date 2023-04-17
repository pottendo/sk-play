//
// kernel.cpp
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
#include "kernel.h"

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <circle/timer.h>
#include <iostream>
#include "rad/rad_main.h"

extern int mandel_driver(void);

CKernel::CKernel(void) :
	mScreen (mOptions.GetWidth (), mOptions.GetHeight ()),
	mTimer (&mInterrupt),
	mLogger (mOptions.GetLogLevel (), &mTimer), mScheduler()
{
}

boolean CKernel::Initialize (void) 
{
	boolean bOK = TRUE;

	if (bOK) bOK = mScreen.Initialize ();
	if (bOK) bOK = mSerial.Initialize (115200);
	if (bOK)
	{
		CDevice *pTarget = mDeviceNameService.GetDevice (mOptions.GetLogDevice (), FALSE);
		if (pTarget == 0)
			pTarget = &mScreen;
		bOK = mLogger.Initialize (pTarget);
	}
	if (bOK) bOK = mInterrupt.Initialize ();
	if (bOK) bOK = mTimer.Initialize ();
	return bOK;
}

extern CKernel Kernel;
extern int mandel_iterate(int);

TShutdownMode CKernel::Run (void)
{
	mLogger.Write ("pottendo-kern", LogNotice, "Mandelbrot Demo");

	(void) mandel_iterate(1000*1000);
#ifdef RAD
	extern int rad_main(CKernel &kernel); 
	(void) rad_main(Kernel);
#endif
#ifdef SKVIC20
	extern int skvic20_main(CKernel &kernel);
	(void) skvic20_main(Kernel);
#endif
	mLogger.Write ("pottendo-kern", LogNotice, "Demo finished");
	return ShutdownHalt;
}

void RPiConsole_put_pixel(uint32_t x, uint32_t y, uint16_t c)
{
	Kernel.set_pixel(x, y, c);
}