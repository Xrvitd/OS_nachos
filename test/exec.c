#include "syscall.h"
int
main()
{
	SpaceId pid;
	pid = Exec("halt.noff");
	Exit(1);
	//Halt();
}
