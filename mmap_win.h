#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>


typedef struct {
  void* addr;
  size_t len;
} ipc_mmap_handle;

static void* ipc_mmap_addr( ipc_mmap_handle* h ) {
  return h->addr;
}

static size_t ipc_mmap_size( ipc_mmap_handle* h ) {
  return h->len;
}


static void ipc_mmap_error( char* buf, size_t len, int code ) {
  if( len > 0 && 0 == FormatMessageA( FORMAT_MESSAGE_FROM_SYSTEM |
                                      FORMAT_MESSAGE_IGNORE_INSERTS,
                                      NULL,
                                      code,
                                      0,
                                      buf,
                                      len,
                                      NULL ) ) {
    strncpy( buf, "unknown error", len-1 );
    buf[ len-1 ] = '\0';
  }
}


static int ipc_mmap_open( ipc_mmap_handle* h, char const* name,
                          int mode ) {
  HANDLE hfile;
  HANDLE hmap;
  LARGE_INTEGER fsize;
  int cfflags = 0;
  int fmflags = 0;
  int mvflags = 0;
  /* figure out the open flags */
  if( mode & MEMFILE_W ) {
    cfflags = GENERIC_READ | GENERIC_WRITE;
    fmflags = PAGE_READWRITE;
    mvflags = FILE_MAP_ALL_ACCESS;
  } else if( mode & MEMFILE_R ) {
    cfflags = GENERIC_READ;
    fmflags = PAGE_READONLY;
    mvflags = FILE_MAP_READ;
  }
  /* open file */
  hfile = CreateFileA( name,
                       cfflags,
                       FILE_SHARE_READ |
                       FILE_SHARE_WRITE,
                       NULL,
                       OPEN_EXISTING,
                       FILE_ATTRIBUTE_NORMAL,
                       NULL );
  if( hfile == INVALID_HANDLE_VALUE )
    return IPC_ERR( GetLastError() );
  /* figure out the size of the file */
  if( !GetFileSizeEx( hfile, &fsize ) ) {
    int saved_errno = GetLastError();
    CloseHandle( hfile );
    return IPC_ERR( saved_errno );
  }
  if( fsize.QuadPart > ~((size_t)0) ) {
    CloseHandle( hfile );
    return IPC_ERR( ERROR_ARITHMETIC_OVERFLOW );
  }
  h->len = (size_t)fsize.QuadPart;
  /* create the anonymous file mapping */
  hmap = CreateFileMappingA( hfile,
                             NULL,
                             fmflags,
                             0,
                             0,
                             NULL );
  if( hmap == NULL ) {
    int saved_errno = GetLastError();
    CloseHandle( hfile );
    return IPC_ERR( saved_errno );
  }
  /* get an address for the file mapping */
  h->addr = MapViewOfFile( hmap,
                           mvflags,
                           0,
                           0,
                           0 );
  if( h->addr == NULL ) {
    int saved_errno = GetLastError();
    CloseHandle( hmap );
    CloseHandle( hfile );
    return IPC_ERR( saved_errno );
  }
  CloseHandle( hmap );
  CloseHandle( hfile );
  return 0;
}


static int ipc_mmap_close( ipc_mmap_handle* h ) {
  if( !UnmapViewOfFile( h->addr ) )
    return IPC_ERR( GetLastError() );
  return 0;
}


#define IPC_MMAP_HAVE_FLUSH
static int ipc_mmap_flush( ipc_mmap_handle* h, size_t pos ) {
  if( !FlushViewOfFile( h->addr, pos ) )
    return IPC_ERR( GetLastError() );
  return 0;
}

