//////////////////////////////////////////////////////////////////////
// Intentionally flawed system call library that implements          //
// (unfortunately, not) "safe" file I/O, "preventing" writing "MZ"   //
// at the beginning of a file.                                       //
//                                                                   //
// Written by Golden G. Richard III (@nolaforensix), 7/2017          //
//                                                                   //
// Props to Brian Hay for a similar exercise he used in a recent     //
// training.                                                        //
///////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fileio.h"

//
// GLOBALS
//

FSError fserror;

typedef struct _FileInternal
{
  FILE *fp;
  char mem[2]; // allows modeling writes to detect MZ
} FileInternal;

//
// private functions
//

static int seek_file(File file, SeekAnchor start, long offset)
{
  if (!file->fp || (start != BEGINNING_OF_FILE &&
                    start != CURRENT_POSITION && start != END_OF_FILE))
  {
    return 0;
  }
  else
  {
    if (!fseek(file->fp, offset, start == BEGINNING_OF_FILE ? SEEK_SET : (start == END_OF_FILE ? SEEK_END : SEEK_CUR)))
    {
      return 1;
    }
    else
    {
      return 0;
    }
  }
}

//
// public functions
//

// open or create a file with pathname 'name' and return a File
// handle.  The file is always opened with read/write access. If the
// open operation fails, the global 'fserror' is set to OPEN_FAILED,
// otherwise to NONE.
File open_file(char *name)
{
  File fh;
  fh = malloc(sizeof(FileInternal)); // allocate memory
  fserror = NONE;
  // try to open existing file
  fh->fp = fopen(name, "r+");
  if (!fh->fp)
  {
    // fail, fall back to creation
    fh->fp = fopen(name, "w+");
    if (!fh->fp)
    {
      fserror = OPEN_FAILED;
      return NULL;
    }
  }
  read_file_from(fh, fh->mem, 2, BEGINNING_OF_FILE, 0L); // read first 2 bytes
  return fh;
}

// close a 'file'.  If the close operation fails, the global 'fserror'
// is set to CLOSE_FAILED, otherwise to NONE.
void close_file(File file)
{
  if (file->fp && !fclose(file->fp))
  {
    free(file); // free memory
    fserror = NONE;
  }
  else
  {
    fserror = CLOSE_FAILED;
  }
}

// read at most 'num_bytes' bytes from 'file' into the buffer 'data',
// starting 'offset' bytes from the 'start' position.  The starting
// position is BEGINNING_OF_FILE, CURRENT_POSITION, or END_OF_FILE. If
// the read operation fails, the global 'fserror' is set to READ_FAILED,
// otherwise to NONE.
unsigned long read_file_from(File file, void *data, unsigned long num_bytes,
                             SeekAnchor start, long offset)
{
  unsigned long bytes_read = 0L;
  fserror = NONE;
  if (!file->fp || !seek_file(file, start, offset))
  {
    fserror = READ_FAILED;
  }
  else
  {
    bytes_read = fread(data, 1, num_bytes, file->fp);
    if (ferror(file->fp))
    {
      fserror = READ_FAILED;
    }
  }
  return bytes_read;
}

// write 'num_bytes' to 'file' from the buffer 'data', starting
// 'offset' bytes from the 'start' position.  The starting position is
// BEGINNING_OF_FILE, CURRENT_POSITION, or END_OF_FILE.  If an attempt
// is made to modify a file such that "MZ" appears in the first two
// bytes of the file, the write operation fails and ILLEGAL_MZ is
// stored in the global 'fserror'.  If the write fails for any other
// reason, the global 'fserror' is set to WRITE_FAILED, otherwise to
// NONE.
unsigned long write_file_at(File file, void *data, unsigned long num_bytes,
                            SeekAnchor start, long offset)
{
  unsigned long bytes_written = 0L;
  char *buf = (char *)data; // Gets char from void pointer
  fserror = NONE;
  if (!file->fp || !seek_file(file, start, offset))
  {
    fserror = WRITE_FAILED;
  }
  else if (file->mem[1] == 'Z' && buf[0] == 'M') // if Z in second byte, trying to write M 1st position
  {
    fserror = ILLEGAL_MZ;
  }
  else if (file->mem[0] == 'M' && buf[1] == 'Z') // checks if M in mem and trying to write Z in 2nd position
  {
    fserror = ILLEGAL_MZ;
  }
  else if ((file->mem[1] == 'Z' || buf[1] == 'Z') && !strncmp(data, "M", 1)) // Z in 2nd byte, trying to write M in first byte
  {
    fserror = ILLEGAL_MZ;
  }
  else if ((file->mem[0] == 'M' || buf[0] == 'M') && !strncmp(data, "Z", 1)) // M in first position, trying to write Z
  {
    fserror = ILLEGAL_MZ;
  }
  else
  {
    unsigned long filePointer = ftell(file->fp); // returns the current file position
    bytes_written = fwrite(data, 1, num_bytes, file->fp);
    if (bytes_written < num_bytes)
    {
      fserror = WRITE_FAILED;
    }
    // add char into mem
    else if (buf[0] == 'M' && filePointer == 0L)
    {
      file->mem[0] = 'M';
    }
    else if (buf[0] == 'Z' && filePointer == 1L)
    {
      file->mem[1] = 'Z';
    }
  }
  return bytes_written;
}

// print a string representation of the error indicated by the global
// 'fserror'.
void fs_print_error(void)
{
  printf("FS ERROR: ");
  switch (fserror)
  {
  case NONE:
    puts("NONE");
    break;
  case OPEN_FAILED:
    puts("OPEN_FAILED");
    break;
  case CLOSE_FAILED:
    puts("CLOSE_FAILED");
    break;
  case READ_FAILED:
    puts("READ_FAILED");
    break;
  case WRITE_FAILED:
    puts("WRITE_FAILED");
    break;
  case ILLEGAL_MZ:
    puts("ILLEGAL_MZ: SHAME ON YOU!");
    break;
  default:
    puts("** UNKNOWN ERROR **");
  }
}
