/* ftglue.c: Glue code for compiling the OpenType code from
 *           FreeType 1 using only the public API of FreeType 2
 *
 * By David Turner, The FreeType Project (www.freetype.org)
 *
 * This code is explicitely put in the public domain
 *
 * See ftglue.h for more information.
 */

#include "ftglue.h"

#if 0
#include <stdio.h>
#define  LOG(x)  _hb_ftglue_log x

static void
_hb_ftglue_log( const char*   format, ... )
{
  va_list  ap;
 
  va_start( ap, format );
  vfprintf( stderr, format, ap );
  va_end( ap );
}

#else
#define  LOG(x)  do {} while (0)
#endif


FT_Pointer _hb_ftglue_alloc(FT_ULong   size,
                            FT_Error  *perror )
{
  FT_Error    error = 0;
  FT_Pointer  block = NULL;

  if ( size > 0 )
  {
    block = malloc( size );
    if ( !block )
      error = FT_Err_Out_Of_Memory;
    else
      memset( (char*)block, 0, (size_t)size );
  }

  *perror = error;
  return block;
}


FT_Pointer _hb_ftglue_realloc(FT_Pointer  block,
                              FT_ULong    old_size,
                              FT_ULong    new_size,
                              FT_Error   *perror )
{
    FT_Pointer  block2 = NULL;
    FT_Error    error  = 0;

    block2 = realloc( block, new_size );
    if ( block2 == NULL )
        error = FT_Err_Out_Of_Memory;
    else if ( new_size > old_size )
        memset( (char*)block2 + old_size, 0, (size_t)(new_size - old_size) );

    if ( !error )
        block = block2;
    
    *perror = error;
    return block;
}


void _hb_ftglue_free( FT_Pointer  block )
{
  if ( block )
    free( block );
}

