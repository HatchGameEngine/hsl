#ifndef UTILITIES_CLOCK_H
#define UTILITIES_CLOCK_H

#include <Includes/Standard.h>

class Clock {
public:
	static void Init();
	static void Start();
	static double GetTicks();
	static double End();
	static void Delay(double milliseconds);
};

#endif /* UTILITIES_CLOCK_H */
