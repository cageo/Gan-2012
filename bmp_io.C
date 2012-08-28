# include <cstdlib>
# include <iostream>
# include <iomanip>
# include <fstream>

using namespace std;

# include "bmp_io.H"

//**********************************
// This set of utilities are modified from source codes by John Burkardt.
// ********************************

static bool bmp_byte_swap = true;

bool bmp_byte_swap_get ( )
{
  return bmp_byte_swap;
}

void bmp_byte_swap_set ( bool value )
{
  bmp_byte_swap = value;

  return;
}

bool bmp_24_data_read ( ifstream &file_in, unsigned long int width, long int height, 
  unsigned char *rarray, unsigned char *garray, unsigned char *barray )
{
  char c;
  bool error;
  int i;
  unsigned char *indexb;
  unsigned char *indexg;
  unsigned char *indexr;
  int j;
  int numbyte;
  int padding;
//
//  Set the padding.
//
  padding = ( 4 - ( ( 3 * width ) % 4 ) ) % 4;

  indexr = rarray;
  indexg = garray;
  indexb = barray;
  numbyte = 0;

  for ( j = 0; j < abs ( height ); j++ ) 
  {
    for ( i = 0; i < width; i++ )
    {

      file_in.read ( &c, 1 );

      error = file_in.eof();

      if ( error )
      {
        cout << "\n";
        cout << "BMP_24_DATA_READ: Fatal error!\n";
        cout << "  Failed reading B for pixel (" << i << "," << j << ").\n";
        return error;
      }

      *indexb = ( unsigned char ) c;
      numbyte = numbyte + 1;
      indexb = indexb + 1;

      file_in.read ( &c, 1 );

      error = file_in.eof();

      if ( error ) 
      {
        cout << "\n";
        cout << "BMP_24_DATA_READ: Fatal error!\n";
        cout << "  Failed reading G for pixel (" << i << "," << j << ").\n";
        return error;
      }

      *indexg = ( unsigned char ) c;
      numbyte = numbyte + 1;
      indexg = indexg + 1;

      file_in.read ( &c, 1 );

      error = file_in.eof();

      if ( error ) 
      {
        cout << "\n";
        cout << "BMP_24_DATA_READ: Fatal error!\n";
        cout << "  Failed reading R for pixel (" << i << "," << j << ").\n";
        return error;
      }

      *indexr = ( unsigned char ) c;
      numbyte = numbyte + 1;
      indexr = indexr + 1;
    }
//
//  If necessary, read a few padding characters.
//
    for ( i = 0; i < padding; i++ )
    {

      file_in.read ( &c, 1 );

      error = file_in.eof();

      if ( error )
      {
        cout << "\n";
        cout << "BMP_24_DATA_READ - Warning!\n";
        cout << "  Failed while reading padding character " << i << "\n";
        cout << "  of total " << padding << " characters\n";
        cout << "  at the end of line " << j << "\n";
        cout << "\n";
        cout << "  This is a minor error.\n";
        return false;
      }
    }
  }

  return false;
}

//****************************************************************************
void bmp_24_data_write ( ofstream &file_out, unsigned long int width, 
  long int height, unsigned char *rarray, unsigned char *garray, 
  unsigned char *barray )
{
  int i;
  unsigned char *indexb;
  unsigned char *indexg;
  unsigned char *indexr;
  int j;
  int padding;
//
//  Set the padding.
//
  padding = ( 4 - ( ( 3 * width ) % 4 ) ) % 4;

  indexr = rarray;
  indexg = garray;
  indexb = barray;

  for ( j = 0; j < abs ( height ); j++ )
  {
    for ( i = 0; i < width; i++ )
    {
      file_out << *indexb;
      file_out << *indexg;
      file_out << *indexr;

      indexb = indexb + 1;
      indexg = indexg + 1;
      indexr = indexr + 1;
    }

    for ( i = 0; i < padding; i++ )
    {
      file_out << 0;
    }
  }

  return;
}

//****************************************************************************

bool bmp_header1_read ( ifstream &file_in, unsigned short int *filetype, 
  unsigned long int *filesize, unsigned short int *reserved1, 
  unsigned short int *reserved2, unsigned long int *bitmapoffset )
{
  bool error;
  char i1;
  char i2;  
//
//  Read FILETYPE.
//
  error = u_short_int_read ( filetype, file_in );

  if ( error )
  {
    return error;
  }
//
//  If you are doing swapping, you have to reunswap the filetype, I think, JVB 15 December 2004.
//
  if ( bmp_byte_swap )
  {
    i1 = ( char ) ( *filetype / 256 );
    i2 = ( char ) ( *filetype % 256 );
    *filetype = i2 * 256 + i1;
  }
//
//  Read FILESIZE.
//
  error = u_long_int_read ( filesize, file_in );
  if ( error ) 
  {
    return error;
  }
//
//  Read RESERVED1.
//
  error = u_short_int_read ( reserved1, file_in );
  if ( error )
  {
    return error;
  }
//
//  Read RESERVED2.
//
  error = u_short_int_read ( reserved2, file_in );
  if ( error )
  {
    return error;
  }
//
//  Read BITMAPOFFSET.
//
  error = u_long_int_read ( bitmapoffset, file_in );
  if ( error )
  {
    return error;
  }

  error = false;
  return error;
}

//****************************************************************************

void bmp_header1_write ( ofstream &file_out, unsigned short int filetype,
  unsigned long int filesize, unsigned short int reserved1, 
  unsigned short int reserved2, unsigned long int bitmapoffset )
{
  u_short_int_write ( filetype, file_out );
  u_long_int_write ( filesize, file_out );
  u_short_int_write ( reserved1, file_out );
  u_short_int_write ( reserved2, file_out );
  u_long_int_write ( bitmapoffset, file_out );

  return;
}

//****************************************************************************

bool bmp_header2_read ( ifstream &file_in, unsigned long int *size,
  unsigned long int *width, long int *height, 
  unsigned short int *planes, unsigned short int *bitsperpixel,
  unsigned long int *compression, unsigned long int *sizeofbitmap,
  unsigned long int *horzresolution, unsigned long int *vertresolution,
  unsigned long int *colorsused, unsigned long int *colorsimportant )
{
  unsigned char c1;
  unsigned char c2;
  bool error;
//
//  Read SIZE, the size of the header in bytes.
//
  error = u_long_int_read ( size, file_in );
  if ( error )
  {
    return error;
  }
//
//  Read WIDTH, the image width in pixels.
//
  error = u_long_int_read ( width, file_in );
  if ( error )
  {
    return error;
  }
//
//  Read HEIGHT, the image height in pixels.
//
  error = long_int_read ( height, file_in );
  if ( error )
  {
    return error;
  }
//
//  Read PLANES, the number of color planes.
//
  error = u_short_int_read ( planes, file_in ); 
  if ( error )
  {
    return error;
  }
//
//  Read BITSPERPIXEL.
//
  error = u_short_int_read ( bitsperpixel, file_in );
  if ( error )
  {
    return error;
  }
//
//  Read COMPRESSION.
//
  error = u_long_int_read ( compression, file_in );
  if ( error )
  {
    return error;
  }
//
//  Read SIZEOFBITMAP.
//
  error = u_long_int_read ( sizeofbitmap, file_in );
  if ( error )
  {
    return error;
  }
//
//  Read HORZRESOLUTION.
//
  error = u_long_int_read ( horzresolution, file_in );
  if ( error )
  {
    return error;
  }
//
//  Read VERTRESOLUTION.
//
  error = u_long_int_read ( vertresolution, file_in );
  if ( error )
  {
    return error;
  }
//
//  Read COLORSUSED.
//
  error = u_long_int_read ( colorsused, file_in );
  if ( error )
  {
    return error;
  }
//
//  Read COLORSIMPORTANT.
//
  error = u_long_int_read ( colorsimportant, file_in );
  if ( error )
  {
    return error;
  }

  error = false;
  return error;
}
//****************************************************************************

void bmp_header2_write ( ofstream &file_out, unsigned long int size,
  unsigned long int width, long int height, 
  unsigned short int planes, unsigned short int bitsperpixel,
  unsigned long int compression, unsigned long int sizeofbitmap,
  unsigned long int horzresolution, unsigned long int vertresolution,
  unsigned long int colorsused, unsigned long int colorsimportant )
{
  u_long_int_write ( size, file_out );
  u_long_int_write ( width, file_out );
  long_int_write ( height, file_out );
  u_short_int_write ( planes, file_out ); 
  u_short_int_write ( bitsperpixel, file_out );
  u_long_int_write ( compression, file_out );
  u_long_int_write ( sizeofbitmap, file_out );
  u_long_int_write ( horzresolution, file_out );
  u_long_int_write ( vertresolution, file_out );
  u_long_int_write ( colorsused, file_out );
  u_long_int_write ( colorsimportant, file_out );

  return;
}

//****************************************************************************

bool bmp_palette_read ( ifstream &file_in, unsigned long int colorsused,
  unsigned char *rparray, unsigned char *gparray, unsigned char *bparray, 
  unsigned char *aparray )
{
  char c;
  bool error;
  int i;
  unsigned char *indexa;
  unsigned char *indexb;
  unsigned char *indexg;
  unsigned char *indexr;

  indexr = rparray;
  indexg = gparray;
  indexb = bparray;
  indexa = aparray;

  for ( i = 0; i < colorsused; i++ )
  {

    file_in.read ( &c, 1 );

    error = file_in.eof();

    if ( error )
    {
      cout << "\n";
      cout << "BMP_PALETTE_READ: Fatal error!\n";
      cout << "  Failed reading B for palette color " << i << ".\n";
      return error;
    }

    *indexb = ( unsigned char ) c;
    indexb = indexb + 1;

    file_in.read ( &c, 1 );

    error = file_in.eof();

    if ( error )
    {
      cout << "\n";
      cout << "BMP_PALETTE_READ: Fatal error!\n";
      cout << "  Failed reading G for palette color " << i << ".\n";
      return error;
    }

    *indexg = ( unsigned char ) c;
    indexg = indexg + 1;

    file_in.read ( &c, 1 );

    error = file_in.eof();

    if ( error )
    {
      cout << "\n";
      cout << "BMP_PALETTE_READ: Fatal error!\n";
      cout << "  Failed reading R for palette color " << i << ".\n";
      return error;
    }

    *indexr = ( unsigned char ) c;
    indexr = indexr + 1;

    file_in.read ( &c, 1 );

    error = file_in.eof();

    if ( error )
    {
      cout << "\n";
      cout << "BMP_PALETTE_READ: Fatal error!\n";
      cout << "  Failed reading A for palette color " << i << ".\n";
      return error;
    }

    *indexa = ( unsigned char ) c;
    indexa = indexa + 1;
  }

  error = false;
  return error;
}
//****************************************************************************

void bmp_palette_write ( ofstream &file_out, unsigned long int colorsused, 
  unsigned char *rparray, unsigned char *gparray, unsigned char *bparray,
  unsigned char *aparray )
{
  int i;
  unsigned char *indexa;
  unsigned char *indexb;
  unsigned char *indexg;
  unsigned char *indexr;

  indexr = rparray;
  indexg = gparray;
  indexb = bparray;
  indexa = aparray;

  for ( i = 0; i < colorsused; i++ )
  {
    file_out << *indexb;
    file_out << *indexg;
    file_out << *indexr;
    file_out << *indexa;

    indexb = indexb + 1;
    indexg = indexg + 1;
    indexr = indexr + 1;
    indexa = indexa + 1;
  }

  return;
}


bool bmp_read ( char *file_in_name, unsigned long int *width, long int *height, 
  unsigned char **rarray, unsigned char **garray, unsigned char **barray )
{
  unsigned char *aparray;
  unsigned long int bitmapoffset;
  unsigned short int bitsperpixel;
  unsigned char *bparray;
  unsigned long int colorsimportant;
  unsigned long int colorsused;
  unsigned long int compression;
  bool error;
  ifstream file_in;
  unsigned long int filesize;
  unsigned short int filetype;
  unsigned char *gparray;
  unsigned long int horzresolution;
  unsigned short int magic;
  int numbytes;
  unsigned short int planes;
  unsigned short int reserved1;
  unsigned short int reserved2;
  unsigned char *rparray;
  unsigned long int size;
  unsigned long int sizeofbitmap;
  unsigned long int vertresolution;
//
//  Open the input file.
//
  file_in.open ( file_in_name, ios::in | ios::binary );

  if ( !file_in ) 
  {
    error = true;
    cout << "\n";
    cout << "BMP_READ - Fatal error!\n";
    cout << "  Could not open the input file.\n";
    return error;
  }
//
//  Read header 1.
//
  error = bmp_header1_read ( file_in, &filetype, &filesize, &reserved1, 
    &reserved2, &bitmapoffset );

  if ( error ) 
  {
    cout << "\n";
    cout << "BMP_READ: Fatal error!\n";
    cout << "  BMP_HEADER1_READ failed.\n";
    return error;
  }
//
//  Make sure the filetype is 'BM'.
//
  magic = 'B' * 256 + 'M';

  if ( filetype != magic )
  {
    cout << "\n";
    cout << "BMP_READ: Fatal error!\n";
    cout << "  The file's internal magic number is not \"BM\".\n";
    cout << "  with the numeric value " << magic << "\n";
    cout << "\n";
    cout << "  Instead, it is \"" 
         << ( char ) ( filetype / 256 ) 
         << ( char ) ( filetype % 256 )
         << "\".\n";
    cout << "  with the numeric value " << filetype << "\n";
    cout << "\n";
    cout << "  (Perhaps you need to reverse the byte swapping option!)\n";
    return 1;
  }
//
//  Read header 2.
//
  error = bmp_header2_read ( file_in, &size, width, height, &planes,
    &bitsperpixel, &compression, &sizeofbitmap, &horzresolution,
    &vertresolution, &colorsused, &colorsimportant );

  if ( error ) 
  {
    cout << "\n";
    cout << "BMP_READ: Fatal error!\n";
    cout << "  BMP_HEADER2_READ failed.\n";
    return error;
  }
//
//  Read the palette.
//
  if ( colorsused > 0 )
  {
    rparray = new unsigned char[colorsused];
    gparray = new unsigned char[colorsused];
    bparray = new unsigned char[colorsused];
    aparray = new unsigned char[colorsused];

    error = bmp_palette_read ( file_in, colorsused, rparray, gparray,
      bparray, aparray );

    if ( error ) 
    {
      cout << "\n";
      cout << "BMP_READ: Fatal error!\n";
      cout << "  BMP_PALETTE_READ failed.\n";
      return error;
    }
    delete [] rparray;
    delete [] gparray;
    delete [] bparray;
    delete [] aparray;
  }
//
//  Allocate storage.
//
  numbytes = ( *width ) * ( abs ( *height ) ) * sizeof ( unsigned char );

  *rarray = new unsigned char[numbytes];
  *garray = new unsigned char[numbytes];
  *barray = new unsigned char[numbytes];
//
//  Read the data.
//
  if ( bitsperpixel == 24 )
  {
    error = bmp_24_data_read ( file_in, *width, *height, *rarray, *garray, 
      *barray );

    if ( error ) 
    {
      cout << "\n";
      cout << "BMP_READ: Fatal error!\n";
      cout << "  BMP_24_DATA_READ failed.\n";
      return error;
    }
  }
  else
  {
    cout << "\n";
    cout << "BMP_READ: Fatal error!\n";
    cout << "  Unrecognized value of BITSPERPIXEL = " << bitsperpixel << "\n";
    return 1;
  }
//
//  Close the file.
//
  file_in.close ( );

  error = false;
  return error;
}

bool bmp_24_write ( char *file_out_name, unsigned long int width, long int height, 
  unsigned char *rarray, unsigned char *garray, unsigned char *barray )
{
  unsigned char *aparray = NULL;
  unsigned long int bitmapoffset;
  unsigned short int bitsperpixel;
  unsigned char *bparray = NULL;
  unsigned long int colorsimportant;
  unsigned long int colorsused;
  unsigned long int compression;
  bool error;
  ofstream file_out;
  unsigned long int filesize;
  unsigned short int filetype;
  unsigned char *gparray = NULL;
  unsigned long int horzresolution;
  int padding;
  unsigned short int planes;
  unsigned short int reserved1 = 0;
  unsigned short int reserved2 = 0;
  unsigned char *rparray = NULL;
  unsigned long int size = 40;
  unsigned long int sizeofbitmap;
  unsigned long int vertresolution;
//
//  Open the output file.
//
  file_out.open ( file_out_name, ios::out | ios::binary );

  error = !file_out;

  if ( error )
  {
    cout << "\n";
    cout << "BMP_24_WRITE - Fatal error!\n";
    cout << "  Could not open the output file.\n";
    return error;
  }
//
//  Write header 1.
//
  if ( bmp_byte_swap )
  {
    filetype = 'M' * 256 + 'B';
  }
  else
  {
    filetype = 'B' * 256 + 'M';
  }
//
//  Determine the padding needed when WIDTH is not a multiple of 4.
//
  padding = ( 4 - ( ( 3 * width ) % 4 ) ) % 4;

  filesize = 54 + ( ( 3 * width ) + padding ) * abs ( height );
  bitmapoffset = 54;

  bmp_header1_write ( file_out, filetype, filesize, reserved1, 
    reserved2, bitmapoffset );
//
//  Write header 2.
//
  planes = 1;
  bitsperpixel = 24;
  compression = 0;
  sizeofbitmap = 0;
  horzresolution = 0;
  vertresolution = 0;
  colorsused = 0;
  colorsimportant = 0;

  bmp_header2_write ( file_out, size, width, height, planes, bitsperpixel, 
    compression, sizeofbitmap, horzresolution, vertresolution,
    colorsused, colorsimportant );
//
//  Write the palette.
//
  bmp_palette_write ( file_out, colorsused, rparray, gparray, bparray, 
    aparray );
//
//  Write the data.
//
  bmp_24_data_write ( file_out, width, height, rarray, garray, barray );
//
//  Close the file.
//
  file_out.close ( );

  error = false;
  return error;
}

//****************************************************************************

bool long_int_read ( long int *long_int_val, ifstream &file_in )
{
  bool error;
  unsigned short int u_short_int_val_hi;
  unsigned short int u_short_int_val_lo;

  if ( bmp_byte_swap )
  {
    error = u_short_int_read ( &u_short_int_val_lo, file_in );
    if ( error )
    {
      return error;
    }
    error = u_short_int_read ( &u_short_int_val_hi, file_in );
    if ( error )
    {
      return error;
    }
  }
  else
  {
    error = u_short_int_read ( &u_short_int_val_hi, file_in );
    if ( error )
    {
      return error;
    }
    error = u_short_int_read ( &u_short_int_val_lo, file_in );
    if ( error )
    {
      return error;
    }
  }

  *long_int_val = ( long int ) 
    ( u_short_int_val_hi << 16 ) | u_short_int_val_lo;

  return false;
}

//****************************************************************************
void long_int_write ( long int long_int_val, ofstream &file_out )
{
  long int temp;
  unsigned short int u_short_int_val_hi;
  unsigned short int u_short_int_val_lo;

  temp = long_int_val / 65536;
  if ( temp < 0 )
  {
    temp = temp + 65536;
  }
  u_short_int_val_hi = ( unsigned short ) temp;

  temp = long_int_val % 65536;
  if ( temp < 0 )
  {
    temp = temp + 65536;
  }
  u_short_int_val_lo = ( unsigned short ) temp;

  if ( bmp_byte_swap )
  {
    u_short_int_write ( u_short_int_val_lo, file_out );
    u_short_int_write ( u_short_int_val_hi, file_out );
  }
  else
  {
    u_short_int_write ( u_short_int_val_hi, file_out );
    u_short_int_write ( u_short_int_val_lo, file_out );
  }

  return;
}

//****************************************************************************
bool u_long_int_read ( unsigned long int *u_long_int_val, 
  ifstream &file_in )
{
  bool error;
  unsigned short int u_short_int_val_hi;
  unsigned short int u_short_int_val_lo;

  if ( bmp_byte_swap )
  {
    error = u_short_int_read ( &u_short_int_val_lo, file_in );
    if ( error )
    {
      return error;
    }
    error = u_short_int_read ( &u_short_int_val_hi, file_in );
    if ( error )
    {
      return error;
    }
  }
  else
  {
    error = u_short_int_read ( &u_short_int_val_hi, file_in );
    if ( error )
    {
      return error;
    }
    error = u_short_int_read ( &u_short_int_val_lo, file_in );
    if ( error )
    {
      return error;
    }
  }

  *u_long_int_val = ( u_short_int_val_hi << 16 ) | u_short_int_val_lo;

  return false;
}

//****************************************************************************
void u_long_int_write ( unsigned long int u_long_int_val, 
  ofstream &file_out )
{
  unsigned short int u_short_int_val_hi;
  unsigned short int u_short_int_val_lo;

  u_short_int_val_hi = ( unsigned short ) ( u_long_int_val / 65536 );
  u_short_int_val_lo = ( unsigned short ) ( u_long_int_val % 65536 );

  if ( bmp_byte_swap )
  {
    u_short_int_write ( u_short_int_val_lo, file_out );
    u_short_int_write ( u_short_int_val_hi, file_out );
  }
  else
  {
    u_short_int_write ( u_short_int_val_hi, file_out );
    u_short_int_write ( u_short_int_val_lo, file_out );
  }

  return;
}
//****************************************************************************

bool u_short_int_read ( unsigned short int *u_short_int_val, 
  ifstream &file_in )
{
  char c;
  unsigned char chi;
  unsigned char clo;

  if ( bmp_byte_swap )
    {
    file_in.read ( &c, 1 );
    if ( file_in.eof() )
    {
      return true;
    }
    clo = ( unsigned char ) c;

    file_in.read ( &c, 1 );
    if ( file_in.eof() )
    {
      return true;
    }
    chi = ( unsigned char ) c;
  }
  else
  {
    file_in.read ( &c, 1 );
    if ( file_in.eof() )
    {
      return true;
    }
    chi = ( unsigned char ) c;

    file_in.read ( &c, 1 );
    if ( file_in.eof() )
    {
      return true;
    }
    clo = ( unsigned char ) c;
  }

  *u_short_int_val = ( chi << 8 ) | clo;

  return false;
}
//****************************************************************************

void u_short_int_write ( unsigned short int u_short_int_val, 
  ofstream &file_out )
{
  unsigned char chi;
  unsigned char clo;

  chi = ( unsigned char ) ( u_short_int_val / 256 );
  clo = ( unsigned char ) ( u_short_int_val % 256 );

  if ( bmp_byte_swap )
  {
    file_out << clo << chi;
  }
  else
  {
    file_out << chi << clo;
  }

  return;
}
