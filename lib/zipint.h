#ifndef _HAD_ZIPINT_H
#define _HAD_ZIPINT_H

/*
  zipint.h -- internal declarations.
  Copyright (C) 1999-2012 Dieter Baron and Thomas Klausner

  This file is part of libzip, a library to manipulate ZIP archives.
  The authors can be contacted at <libzip@nih.at>

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:
  1. Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in
     the documentation and/or other materials provided with the
     distribution.
  3. The names of the authors may not be used to endorse or promote
     products derived from this software without specific prior
     written permission.
 
  THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS
  OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY
  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
  GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
  IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
  OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
  IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <zlib.h>

#ifdef _WIN32
#define ZIP_EXTERN __declspec(dllexport)
/* for dup(), close(), etc. */
#include <io.h>
#endif

#include "zip.h"
#include "config.h"

#ifndef HAVE_FSEEKO
#define fseeko(s, o, w)	(fseek((s), (long int)(o), (w)))
#endif

#ifndef HAVE_FTELLO
#define ftello(s)	((long)ftell((s)))
#endif

#ifndef HAVE_MKSTEMP
int _zip_mkstemp(char *);
#define mkstemp _zip_mkstemp
#endif

#ifdef HAVE_MOVEFILEEXA
#include <windows.h>
#define _zip_rename(s, t)						\
	(!MoveFileExA((s), (t),						\
		     MOVEFILE_COPY_ALLOWED|MOVEFILE_REPLACE_EXISTING))
#else
#define _zip_rename	rename
#endif

/* Windows' open() doesn't understand Unix permissions */
#if !defined(HAVE_OPEN)
#if defined(HAVE__OPEN)
#define open(a, b, c)	_open((a), (b))
#endif
#endif

#if !defined(HAVE_SNPRINTF)
#if defined(HAVE__SNPRINTF)
#define snprintf	_snprintf
#endif
#endif

#if !defined(HAVE_STRCASECMP)
#if defined(HAVE__STRCMPI)
#define strcasecmp	_strcmpi
#elif defined(HAVE__STRICMP)
#define strcasecmp	_stricmp
#endif
#endif

#if !defined(HAVE_STRDUP)
#if defined(HAVE__STRDUP)
#define strdup		_strdup
#endif
#endif



#define CENTRAL_MAGIC "PK\1\2"
#define LOCAL_MAGIC   "PK\3\4"
#define EOCD_MAGIC    "PK\5\6"
#define DATADES_MAGIC "PK\7\8"
#define EOCD64LOC_MAGIC "PK\6\7"
#define EOCD64_MAGIC  "PK\6\6"
#define TORRENT_SIG	"TORRENTZIPPED-"
#define TORRENT_SIG_LEN	14
#define TORRENT_CRC_LEN 8
#define TORRENT_MEM_LEVEL	8
#define CDENTRYSIZE         46u
#define LENTRYSIZE          30
#define MAXCOMLEN        65536
#define MAXEXTLEN        65536
#define EOCDLEN             22
#define EOCD64LOCLEN	    20
#define EOCD64LEN	    56
#define CDBUFSIZE       (MAXCOMLEN+EOCDLEN+EOCD64LOCLEN)
#define BUFSIZE		8192



/* This section contains API that won't materialize like this.  It's
   placed in the internal section, pending cleanup. */

typedef struct zip_source *(*zip_compression_implementation)(struct zip *,
						     struct zip_source *,
						     zip_uint16_t, int);
typedef struct zip_source *(*zip_encryption_implementation)(struct zip *,
						    struct zip_source *,
						    zip_uint16_t, int,
						    const char *);

ZIP_EXTERN zip_compression_implementation zip_get_compression_implementation(
    zip_uint16_t);
ZIP_EXTERN zip_encryption_implementation zip_get_encryption_implementation(
    zip_uint16_t);




/* This section contains API that is of limited use until support for
   user-supplied compression/encryption implementation is finished.
   Thus we will keep it private for now. */

typedef zip_int64_t (*zip_source_layered_callback)(struct zip_source *, void *,
						   void *, zip_uint64_t,
						   enum zip_source_cmd);

ZIP_EXTERN void zip_source_close(struct zip_source *);
ZIP_EXTERN struct zip_source *zip_source_crc(struct zip *, struct zip_source *,
					     int);
ZIP_EXTERN struct zip_source *zip_source_deflate(struct zip *,
						 struct zip_source *,
						 zip_uint16_t, int);
ZIP_EXTERN void zip_source_error(struct zip_source *, int *, int *);
ZIP_EXTERN struct zip_source *zip_source_layered(struct zip *,
						 struct zip_source *,
						 zip_source_layered_callback,
						 void *);
ZIP_EXTERN int zip_source_open(struct zip_source *);
ZIP_EXTERN struct zip_source *zip_source_pkware(struct zip *,
						struct zip_source *,
						zip_uint16_t, int,
						const char *);
ZIP_EXTERN zip_int64_t zip_source_read(struct zip_source *, void *,
				       zip_uint64_t);
ZIP_EXTERN int zip_source_stat(struct zip_source *, struct zip_stat *);
ZIP_EXTERN struct zip_source *zip_source_window(struct zip *, struct zip_source *,
						zip_uint64_t, zip_uint64_t);


/* This function will probably remain private.  It is not needed to
   implement compression/encryption routines.  (We should probably
   rename it to _zip_source_pop.) */

ZIP_EXTERN struct zip_source *zip_source_pop(struct zip_source *);



/* state of change of a file in zip archive */

enum zip_state { ZIP_ST_UNCHANGED, ZIP_ST_DELETED, ZIP_ST_REPLACED,
		 ZIP_ST_ADDED, ZIP_ST_RENAMED };

/* error source for layered sources */

enum zip_les { ZIP_LES_NONE, ZIP_LES_UPPER, ZIP_LES_LOWER, ZIP_LES_INVAL };

/* directory entry: general purpose bit flags */

#define ZIP_GPBF_ENCRYPTED		0x0001	/* is encrypted */
#define ZIP_GPBF_DATA_DESCRIPTOR	0x0008	/* crc/size after file data */
#define ZIP_GPBF_STRONG_ENCRYPTION	0x0040  /* uses strong encryption */
#define ZIP_GPBF_ENCODING_UTF_8		0x0800  /* file name encoding is UTF-8 */

/* encoding type */
enum zip_encoding_type {
    ZIP_ENCODING_UNKNOWN, /* not yet analyzed */
    ZIP_ENCODING_ASCII,   /* plain ASCII */
    ZIP_ENCODING_UTF8,    /* possibly UTF-8 */
    ZIP_ENCODING_CP437    /* Code Page 437 */
};

/* error information */

struct zip_error {
    int zip_err;	/* libzip error code (ZIP_ER_*) */
    int sys_err;	/* copy of errno (E*) or zlib error code */
    char *str;		/* string representation or NULL */
};

/* zip archive, part of API */

struct zip {
    char *zn;			/* file name */
    FILE *zp;			/* file */
    int open_flags;		/* flags passed to zip_open */
    struct zip_error error;	/* error information */

    unsigned int flags;		/* archive global flags */
    unsigned int ch_flags;	/* changed archive global flags */

    char *default_password;	/* password used when no other supplied */

    struct zip_cdir *cdir;	/* central directory */
    char *ch_comment;		/* changed archive comment */
    int ch_comment_len;		/* length of changed zip archive
				 * comment, -1 if unchanged */
    zip_uint64_t nentry;	/* number of entries */
    zip_uint64_t nentry_alloc;	/* number of entries allocated */
    struct zip_entry *entry;	/* entries */
    int nfile;			/* number of opened files within archive */
    int nfile_alloc;		/* number of files allocated */
    struct zip_file **file;	/* opened files within archive */
};

/* file in zip archive, part of API */

struct zip_file {
    struct zip *za;		/* zip archive containing this file */
    struct zip_error error;	/* error information */
    int eof;
    struct zip_source *src;	/* data source */
};

/* zip archive directory entry (central or local) */

#define ZIP_DIRENT_COMP_METHOD	0x0001
#define ZIP_DIRENT_FILENAME	0x0002
#define ZIP_DIRENT_COMMENT	0x0004
#define ZIP_DIRENT_EXTRAFIELD	0x0008
#define ZIP_DIRENT_ALL		0xffff

struct zip_dirent_settable {
    zip_uint32_t valid;
    zip_int32_t comp_method;		/* (cl) compression method used (uint16 and ZIP_CM_DEFAULT (-1)) */
    char *filename;			/* (cl) file name (NUL-terminated) */
    char *comment;			/* (c)  file comment */
    zip_uint16_t comment_len;		/* (c)  length of file comment */
    char *extrafield;			/* (cl) extra field */
    zip_uint16_t extrafield_len;	/* (cl) length of extra field */
};

struct zip_dirent {
    unsigned short version_madeby;	/* (c)  version of creator */
    unsigned short version_needed;	/* (cl) version needed to extract */
    unsigned short bitflags;		/* (cl) general purpose bit flag */
    time_t last_mod;			/* (cl) time of last modification */
    unsigned int crc;			/* (cl) CRC-32 of uncompressed data */
    unsigned int comp_size;		/* (cl) size of compressed data */
    unsigned int uncomp_size;		/* (cl) size of uncompressed data */
    unsigned short disk_number;		/* (c)  disk number start */
    unsigned short int_attrib;		/* (c)  internal file attributes */
    unsigned int ext_attrib;		/* (c)  external file attributes */
    unsigned int offset;		/* (c)  offset of local header */
    unsigned short fn_type;		/*      file name encoding (autorecognition) */
    char *filename_converted;		/*      file name (autoconverted) */
    unsigned short fc_type;		/*      file comment encoding (autorecognition) */
    char *comment_converted;		/*      file comment (autoconverted) */
    zip_uint32_t comment_converted_len;	/*      file comment length (autoconverted) */
    struct zip_dirent_settable settable;
};

/* zip archive central directory */

struct zip_cdir {
    struct zip_dirent *entry;	 	/* directory entries */
    int nentry;			 	/* number of entries */

    unsigned int size;		 	/* size of central direcotry */
    unsigned int offset;	 	/* offset of central directory in file */
    char *comment;		 	/* zip archive comment */
    unsigned short comment_len;	 	/* length of zip archive comment */
    unsigned short comment_type; 	/* archive comment encoding (autorecognition) */
    char *comment_converted;     	/* archive comment (autoconverted) */
    zip_uint32_t comment_converted_len;	/* archive comment length (autoconverted) */
};



struct zip_source {
    struct zip_source *src;
    union {
	zip_source_callback f;
	zip_source_layered_callback l;
    } cb;
    void *ud;
    enum zip_les error_source;
    int is_open;
};

/* entry in zip archive directory */

struct zip_entry {
    enum zip_state state;
    struct zip_source *source;
    struct zip_dirent_settable changes;
};



extern const char * const _zip_err_str[];
extern const int _zip_nerr_str;
extern const int _zip_err_type[];



#define ZIP_ENTRY_DATA_CHANGED(x)	\
			((x)->state == ZIP_ST_REPLACED  \
			 || (x)->state == ZIP_ST_ADDED)

#define ZIP_IS_RDONLY(za)	((za)->ch_flags & ZIP_AFL_RDONLY)



int _zip_cdir_compute_crc(struct zip *, uLong *);
void _zip_cdir_free(struct zip_cdir *);
int _zip_cdir_grow(struct zip_cdir *, int, struct zip_error *);
struct zip_cdir *_zip_cdir_new(int, struct zip_error *);
int _zip_cdir_write(struct zip_cdir *, FILE *, struct zip_error *);

void _zip_dirent_finalize(struct zip_dirent *);
void _zip_dirent_init(struct zip_dirent *);
int _zip_dirent_read(struct zip_dirent *, FILE *, const unsigned char **,
		     zip_uint32_t *, int, struct zip_error *);
void _zip_dirent_torrent_normalize(struct zip_dirent *);
int _zip_dirent_write(struct zip_dirent *, FILE *, int, struct zip_error *);

void _zip_entry_free(struct zip_entry *);
void _zip_entry_init(struct zip *, int);
struct zip_entry *_zip_entry_new(struct zip *);

void _zip_error_clear(struct zip_error *);
void _zip_error_copy(struct zip_error *, struct zip_error *);
void _zip_error_fini(struct zip_error *);
void _zip_error_get(struct zip_error *, int *, int *);
void _zip_error_init(struct zip_error *);
void _zip_error_set(struct zip_error *, int, int);
void _zip_error_set_from_source(struct zip_error *, struct zip_source *);
const char *_zip_error_strerror(struct zip_error *);

int _zip_file_fillbuf(void *, size_t, struct zip_file *);
unsigned int _zip_file_get_offset(struct zip *, int, struct zip_error *);

int _zip_filerange_crc(FILE *, off_t, off_t, uLong *, struct zip_error *);

enum zip_encoding_type _zip_guess_encoding(const zip_uint8_t * const,
					   zip_uint32_t);
zip_uint8_t *_zip_cp437_to_utf8(const zip_uint8_t * const, zip_uint32_t,
				zip_uint32_t *, struct zip_error *error);

struct zip *_zip_open(const char *, FILE *, int, int, int *);

struct zip_source *_zip_source_file_or_p(struct zip *, const char *, FILE *,
					 zip_uint64_t, zip_int64_t, int,
					 const struct zip_stat *);
struct zip_source *_zip_source_new(struct zip *);
struct zip_source *_zip_source_zip_new(struct zip *, struct zip *, zip_uint64_t, int,
				       zip_uint64_t, zip_int64_t, const char *);

int _zip_changed(struct zip *, int *);
const char *_zip_get_name(struct zip *, zip_uint64_t, int, struct zip_error *);
int _zip_local_header_read(struct zip *, int);
void *_zip_memdup(const void *, size_t, struct zip_error *);
int _zip_name_locate(struct zip *, const char *, int, struct zip_error *);
struct zip *_zip_new(struct zip_error *);
zip_uint16_t _zip_read2(const unsigned char **);
zip_uint32_t _zip_read4(const unsigned char **);
zip_uint64_t _zip_read8(const unsigned char **);
zip_int64_t _zip_replace(struct zip *, zip_uint64_t, const char *,
			 struct zip_source *);
int _zip_set_name(struct zip *, zip_uint64_t, const char *);
void _zip_u2d_time(time_t, unsigned short *, unsigned short *);
int _zip_unchange(struct zip *, zip_uint64_t, int);
void _zip_unchange_data(struct zip_entry *);

#endif /* zipint.h */
