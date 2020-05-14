#include "syscall.h"

int
main()
{
    SpaceId newProc;
    OpenFileId input = ConsoleInput;
    OpenFileId output = ConsoleOutput;
    char prompt[6], ch, buffer[60];
    int i;

    prompt[0] = 'i';
    prompt[1] = 'n';
	prompt[2] = 'p';
prompt[3] = 'u';
prompt[4] = 't';
prompt[5] = ':';

    while( 1 )
    {
	Write(prompt, 6, output);

	i = 0;
	
	do {
	
	    Read(&buffer[i], 1, input); 

	} while( buffer[i++] != '\n' );

	buffer[--i] = '\0';

	if( i > 0 ) {
		newProc = Exec(buffer);
		//Join(newProc);
	}
    }
}

