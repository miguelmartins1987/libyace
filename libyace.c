/*
Copyright (c) 2018 Miguel Martins

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <unistd.h>

#include <sys/types.h>
#include <sys/wait.h>

#include <paths.h>

#ifndef OCI_ORACLE
#include <oci.h>
#endif

#define STANDARD_INPUT 0
#define STANDARD_OUTPUT 1
#define STANDARD_ERROR 2

#define PIPE_READ 0
#define PIPE_WRITE 1

#define MAXIMUM_NUMBER_OF_CHARACTERS 32767

int pipes[3][2];

int ExecuteCommand(OCIExtProcContext* ctx, char* command, short command_indicator, int command_length, char* command_output, short* command_output_indicator, int* command_output_length, char* command_error, short* command_error_indicator, int* command_error_length, short* return_indicator)
{
  *command_output_indicator = OCI_IND_NULL;
  *command_output_length = 0;

  *command_error_indicator = OCI_IND_NULL;
  *command_error_length = 0;

  *return_indicator = OCI_IND_NOTNULL;

  if (command_indicator == OCI_IND_NULL)
  {
    OCIExtProcRaiseExcpWithMsg(ctx, 20000, (oratext*)"No command specified!", 0);
    return EXIT_FAILURE;
  }

  if (pipe(pipes[STANDARD_INPUT]) < 0)
  {
    OCIExtProcRaiseExcpWithMsg(ctx, 20000, (oratext*)"Unable to pipe standard input!", 0);
    return EXIT_FAILURE;
  }

  if (pipe(pipes[STANDARD_OUTPUT]) < 0)
  {
    close(pipes[STANDARD_INPUT][PIPE_READ]);
    close(pipes[STANDARD_INPUT][PIPE_WRITE]);
    OCIExtProcRaiseExcpWithMsg(ctx, 20000, (oratext*)"Unable to pipe standard output!", 0);
    return EXIT_FAILURE;
  }

  if (pipe(pipes[STANDARD_ERROR]) < 0)
  {
    close(pipes[STANDARD_INPUT][PIPE_READ]);
    close(pipes[STANDARD_INPUT][PIPE_WRITE]);
    close(pipes[STANDARD_OUTPUT][PIPE_READ]);
    close(pipes[STANDARD_OUTPUT][PIPE_WRITE]);
    OCIExtProcRaiseExcpWithMsg(ctx, 20000, (oratext*)"Unable to pipe standard error!", 0);
    return EXIT_FAILURE;
  }

  pid_t pid = fork();
  if (pid < 0)
  {
    close(pipes[STANDARD_INPUT][PIPE_READ]);
    close(pipes[STANDARD_INPUT][PIPE_WRITE]);
    close(pipes[STANDARD_OUTPUT][PIPE_READ]);
    close(pipes[STANDARD_OUTPUT][PIPE_WRITE]);
    close(pipes[STANDARD_ERROR][PIPE_READ]);
    close(pipes[STANDARD_ERROR][PIPE_WRITE]);
    OCIExtProcRaiseExcpWithMsg(ctx, 20000, (oratext*)"Unable to fork!", 0);
    return EXIT_FAILURE;
  }
  else if (pid == 0)
  {
    // Child 
    if (dup2(pipes[STANDARD_OUTPUT][PIPE_WRITE], STANDARD_OUTPUT) < 0)
    {
      OCIExtProcRaiseExcpWithMsg(ctx, 20000, (oratext*)"Child unable to redirect standard output!", 0);
      return EXIT_FAILURE;
    }   
    if (dup2(pipes[STANDARD_ERROR][PIPE_WRITE], STANDARD_ERROR) < 0)
    {
      OCIExtProcRaiseExcpWithMsg(ctx, 20000, (oratext*)"Child unable to redirect standard error!", 0);
      return EXIT_FAILURE;
    }

    close(pipes[STANDARD_INPUT][PIPE_READ]);
    close(pipes[STANDARD_OUTPUT][PIPE_READ]);
    close(pipes[STANDARD_ERROR][PIPE_READ]);

    if (execlp(_PATH_BSHELL, "sh", "-c", command, (char*)NULL) < 0)
    {
      OCIExtProcRaiseExcpWithMsg(ctx, 20000, (oratext*)"Child unable to execute command!", 0);
      return EXIT_FAILURE;
    }
  }
  else
  {
    // Parent
    int status = 0;
    if (waitpid(pid, &status, 0) < 0)
    {
      OCIExtProcRaiseExcpWithMsg(ctx, 20000, (oratext*)"Parent unable to wait for child!", 0);
      return EXIT_FAILURE;
    }

    close(pipes[STANDARD_INPUT][PIPE_WRITE]);
    close(pipes[STANDARD_OUTPUT][PIPE_WRITE]);
    close(pipes[STANDARD_ERROR][PIPE_WRITE]);

    // Read standard output
    ssize_t number_of_bytes_read = read(pipes[STANDARD_OUTPUT][PIPE_READ], command_output, MAXIMUM_NUMBER_OF_CHARACTERS);
    if (number_of_bytes_read < 0)
    {
      OCIExtProcRaiseExcpWithMsg(ctx, 20000, (oratext*)"Unable to read child's standard output!", 0);
      return EXIT_FAILURE;
    }
    *command_output_length += number_of_bytes_read;           
    if (*command_output_length > 0)
    {
      *command_output_indicator = OCI_IND_NOTNULL;
    }

    // Read standard error
    number_of_bytes_read = read(pipes[STANDARD_ERROR][PIPE_READ], command_error, MAXIMUM_NUMBER_OF_CHARACTERS);
    if (number_of_bytes_read < 0)
    {
      OCIExtProcRaiseExcpWithMsg(ctx, 20000, (oratext*)"Unable to read child's standard error!", 0);
      return EXIT_FAILURE;
    }
    *command_error_length += number_of_bytes_read;
    if (*command_error_length > 0)
    {
      *command_error_indicator = OCI_IND_NOTNULL;
    }

    if (!WIFEXITED(status))
    {
      OCIExtProcRaiseExcpWithMsg(ctx, 20000, (oratext*)"Child didn't exit properly!", 0);
      return EXIT_FAILURE;
    }
    else
    {
      return WEXITSTATUS(status);
    }
  }
  return EXIT_SUCCESS;
}

