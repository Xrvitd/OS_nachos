#include "copyright.h"
#include "system.h"
#include "syscall.h"


extern void StartProcess(int);

// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//	The result of the system call, if any, must be put back into r2.
void AdvancePC() {
    machine->WriteRegister(PCReg, machine->ReadRegister(PCReg) + 4);
    machine->WriteRegister(NextPCReg, machine->ReadRegister(NextPCReg) + 4);
}

void ExceptionHandler(ExceptionType which) {

    int type = machine->ReadRegister(2);  // 系统调用号
   // printf("-------\nwhich: %d, type: %d\n\n", which, type);
    if (which == SyscallException) {
        switch (type) {
            case SC_Halt: {
                interrupt->Halt();
                break;
            }
            case SC_Exec: {
                printf("Exec : \n");
                char filename[128];
                int addr = machine->ReadRegister(4);
                int i = 0;
                do { machine->ReadMem(addr + i, 1, (int *) &filename[i]); } while (filename[i++] != '\0');
                OpenFile *executable = fileSystem->Open(filename);
                if (executable == NULL) {
                    printf("Unable to open file: %s", filename);
                    return;
                }
                AddrSpace *space = new AddrSpace(executable);
                delete executable;

                Thread *thread = new Thread(filename);

                printf("---space->getSpaceID():%d\n", space->getSpaceID());
                int pidd = space->getSpaceID();
                thread->Fork(StartProcess, pidd);
                thread->space = space;

                printf("---sc exec:%s\n", filename);
                machine->WriteRegister(2, space->getSpaceID());
                AdvancePC();
                break;
            }
            case SC_Exit: {
                int ExitStatus = machine->ReadRegister(4);
                delete currentThread->space;
                printf("--exit code:%d\n", ExitStatus);
                currentThread->Finish();
                printf("--currentThread->Finish(); ok\n");
                AdvancePC();
                break;
            }
            case SC_Yield:{
                currentThread->Yield();
                AdvancePC();
                break;

            }

            case SC_Create:{

                int base = machine->ReadRegister(4);
                int value;
                int count =0 ;
                char* FileName = new char[128];

                do {
                    machine->ReadMem(base+count,1,&value);
                    FileName[count] = *(char*)&value;
                    count++;

                }while (*(char*)&value != '\0' && count<128);

                if(!fileSystem->Create(FileName,0))
                    printf("Create file &s failed!\n",FileName);
                AdvancePC();
                break;


            }
            case SC_Open:{
                int base = machine->ReadRegister(4);
                int value;
                int count =0 ;
                char* FileName = new char[128];

                do {
                    machine->ReadMem(base+count,1,&value);
                    FileName[count] = *(char*)&value;
                    count++;

                }while (*(char*)&value != '\0' && count<128);

                int fileid;
                OpenFile* openfile= fileSystem->Open(FileName);
                if(openfile==NULL)
                {
                    printf("Open file &s failed!\n",FileName);
                    fileid = -1;
                }
                else{
                    fileid = currentThread->space->getFileDescriptor(openfile);
                    if(fileid<0)
                    {
                        printf("Open too many!\n");
                    }
                }
                machine->WriteRegister(2,fileid);AdvancePC();

            }
            case SC_Write:{
        int base =machine->ReadRegister(4);//buffer
        int size=machine->ReadRegister(5);//bytes written to file
        int fileld=machine->ReadRegister(6);//fd
        int value;
        int count=0;
    //printf("base=%d,size=%d,fileld=%d \n",base,size,fileld);
        OpenFile* openfile =new OpenFile(fileld);
        ASSERT(openfile !=NULL);
        char* buffer=new char[128];
        do{
            machine->ReadMem(base+count,1,&value);
            buffer[count]=*(char*)&value;
            count++;
              }while((*(char*)&value!='\0')&&(count<size));
            buffer[size]='\0';
            openfile=currentThread->space->getFiled(fileld);
            //printf("openfile =%d\n",openfile);
            if(openfile==NULL)
            {
                printf("Failed to Open file \"%d\".\n",fileld);
                AdvancePC();
                break;
            }
            if(fileld==1||fileld==2)
            {
                openfile->WriteStdout(buffer,size);
                delete []buffer;
                AdvancePC();
                break;}
            int WritePosition=openfile->Length();
            openfile->Seek(WritePosition);
            int writtenBytes;
            writtenBytes = openfile->Write(buffer,size);
            if(writtenBytes==0)
            {
                printf("Write failed\n");
            }
            else{
                if(fileld!=1&&fileld!=2)
                {
                    printf("Write success\n");
                }
            }
            delete []buffer;
            AdvancePC();
            break;
            }

            case SC_Read:
            {
                   int base =machine->ReadRegister(4);//buffer
        int size=machine->ReadRegister(5);//bytes written to file
        int fileld=machine->ReadRegister(6);//fd
    OpenFile* openfile=currentThread->space->getFiled(fileld);
                char buffer[size];
                int readnum=0;
                if(fileld==0)
                readnum=openfile->ReadStdin(buffer,size);
                else
                readnum = openfile->Read(buffer,size);

                for(int i=0;i<readnum;i++)
                {
                    machine->WriteMem(base,1,buffer[i]);
                }
                buffer[readnum] = '\0';

                for(int i=0;i<readnum;i++)
                {
                    if(buffer[i]>=0&&buffer[i]<=9)
                    {
                        buffer[i] = buffer[i] + 0x30;
                    }
                }

                char *buf=buffer;
                if(readnum>0)
                {
                    if(fileld!=0)
                    {
                        printf("read success!\n");
                    }
                }else{
                    printf("read failed!\n");
                }
                machine->WriteRegister(2,readnum);
                AdvancePC();
                break;
            }

            case SC_Close:
            {
                int fileld =  machine->ReadRegister(4);
                OpenFile* openfile = currentThread->space->getFiled(fileld);
                if(openfile!=NULL)
                {
                    //openfile->WriteBack();
                    delete openfile;
                    currentThread->space->releaseFileDescriptor(fileld);

                    printf("close  success!\n");



                }else{
                    printf("read failed!\n");
                }
 AdvancePC();
                break;

            }

        //    case SC_Join: {
        //        int SpaceId = machine->ReadRegister(4);
        //        currentThread->Join(SpaceId);
        //        machine->WriteRegister(2, currentThread->waitProcessExitCode);
        //        AdvancePC();
        //        break;
        //    }
            default: {
                printf("Unexpected syscall %d %d\n", which, type);
                ASSERT(FALSE);
            }
        }
    } else {
        printf("Unexpected user mode exception %d %d\n", which, type);
        ASSERT(FALSE);
    }
}
