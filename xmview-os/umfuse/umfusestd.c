/*   This is part of um-ViewOS
 *   The user-mode implementation of OSVIEW -- A Process with a View
 *
 *   
 *
 *   Copyright 2005 Renzo Davoli University of Bologna - Italy
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License, version 2, as
 *   published by the Free Software Foundation.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License along
 *   with this program; if not, write to the Free Software Foundation, Inc.,
 *   51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA.
 *
 *   $Id$
 *
 */
#include <fuse/fuse.h>
#include <errno.h>
#include <stdio.h>
#include <config.h>

#undef UMFUSE_SHOW_STDCALL
#ifdef UMFUSE_SHOW_STDCALL
#define PRINTCALL(X,Y) fprintf(stderr,(X),(Y));
#else
#define PRINTCALL(X,Y) 
#endif
		
/** Get file attributes.
 *
 * Similar to stat().  The 'st_dev' and 'st_blksize' fields are
 * ignored.  The 'st_ino' field is ignored except if the 'use_ino'
 * mount option is given.
 */
static int umstd_getattr (const char *path, struct stat *stat)
{
	PRINTCALL("default getattr %s\n", path);
	return -ENOTSUP;
}

/** Read the target of a symbolic link
 *
 * The buffer should be filled with a null terminated string.  The
 * buffer size argument includes the space for the terminating
 * null character.  If the linkname is too long to fit in the
 * buffer, it should be truncated.  The return value should be 0
 * for success.
 */
static int umstd_readlink (const char *path, char *link, size_t size)
{
	PRINTCALL("default readlink %s\n", path);
	return -EINVAL;
}

/** Read the contents of a directory
 * **** OLD INTERFACE **** DEPRECATED!
 * This operation is the opendir(), readdir(), ..., closedir()
 * sequence in one call. For each directory entry the filldir
 * function should be called.
 */
static int umstd_getdir (const char *path, fuse_dirh_t dir, fuse_dirfil_t dirf)
{
	printf("default getdir %s\n", path);
	return -ENOSYS;
}

/** Create a file node
 *
 * There is no create() operation, mknod() will be called for
 * creation of all non-directory, non-symlink nodes.
 */
static int umstd_mknod (const char *path, mode_t mode, dev_t dev)
{
	PRINTCALL("default mknod %s\n", path);
	return -ENOSYS;
}

/** Create a directory */
static int umstd_mkdir (const char *path, mode_t mode)
{
	PRINTCALL("default mkdir %s\n", path);
	return -ENOSYS;
}

/** Remove a file */
static int umstd_unlink (const char *path)
{
	PRINTCALL("default unlink %s\n", path);
	return -ENOSYS;
}

/** Remove a directory */
static int umstd_rmdir (const char *path)
{
	PRINTCALL("default rmdir %s\n", path);
	return -ENOSYS;
}

/** Create a symbolic link */
static int umstd_symlink (const char *path, const char *newpath)
{
	PRINTCALL("default symlink %s\n", path);
	return -ENOSYS;
}

/** Rename a file */
static int umstd_rename (const char *path, const char *newpath)
{
	PRINTCALL("default rename %s\n", path);
	return -ENOSYS;
}

/** Create a hard link to a file */
static int umstd_link (const char *path, const char *newpath)
{
	PRINTCALL("default link %s\n", path);
	return -ENOSYS;
}

/** Change the permission bits of a file */
static int umstd_chmod (const char *path, mode_t mode)
{
	PRINTCALL("default chmod %s\n", path);
	return -ENOSYS;
}

/** Change the owner and group of a file */
static int umstd_chown (const char *path, uid_t uid, gid_t gid)
{
	PRINTCALL("default chown %s\n", path);
	return -ENOSYS;
}

/** Change the size of a file */
static int umstd_truncate (const char *path, off_t off)
{
	PRINTCALL("default truncat %s\n", path);
	return -ENOSYS;
}

/** Change the access and/or modification times of a file */
static int umstd_utime (const char *path, struct utimbuf *timbuf)
{
	PRINTCALL("default utime %s\n", path);
	return -ENOSYS;
}

/** File open operation
 *
 * No creation, or trunctation flags (O_CREAT, O_EXCL, O_TRUNC)
 * will be passed to open().  Open should check if the operation
 * is permitted for the given flags.  Optionally open may also
 * return an arbitary filehandle in the fuse_file_info structure,
 * which will be passed to all file operations.
 */
static int umstd_open (const char *path, struct fuse_file_info *fileinfo)
{
	PRINTCALL("default open %s\n", path);
	return -ENOSYS;
}

/** Read data from an open file
 *
 * Read should return exactly the number of bytes requested except
 * on EOF or error, otherwise the rest of the data will be
 * substituted with zeroes.  An exception to this is when the
 * 'direct_io' mount option is specified, in which case the return
 * value of the read system call will reflect the return value of
 * this operation.
 */
static int umstd_read (const char *path, char *buf, size_t size, off_t off, struct fuse_file_info *fileinfo)
{
	PRINTCALL("default read %s\n", path);
	return -ENOSYS;
}

/** Write data to an open file
 *
 * Write should return exactly the number of bytes requested
 * except on error.  An exception to this is when the 'direct_io'
 * mount option is specified (see read operation).
 */
static int umstd_write (const char *path, const char *buf, size_t size, off_t off,
		struct fuse_file_info *fileinfo)
{
	PRINTCALL("default write %s\n", path);
	return -ENOSYS;
}

/** Get file system statistics
 *
 * The 'f_frsize', 'f_favail', 'f_fsid' and 'f_flag' fields are ignored
 *
 */
#if ( FUSE_MINOR_VERSION >= 5 )
static int umstd_statfs (const char *path, struct statvfs *stat)
#else
static int umstd_statfs (const char *path, struct statfs *stat)
#endif
{
	PRINTCALL("default statfs %s\n", path);
	return -ENOSYS;
}

/** Possibly flush cached data
 *
 * BIG NOTE: This is not equivalent to fsync().  It's not a
 * request to sync dirty data.
 *
 * Flush is called on each close() of a file descriptor.  So if a
 * filesystem wants to return write errors in close() and the file
 * has cached dirty data, this is a good place to write back data
 * and return any errors.  Since many applications ignore close()
 * errors this is not always useful.
 *
 * NOTE: The flush() method may be called more than once for each
 * open().  This happens if more than one file descriptor refers
 * to an opened file due to dup(), dup2() or fork() calls.  It is
 * not possible to determine if a flush is final, so each flush
 * should be treated equally.  Multiple write-flush sequences are
 * relatively rare, so this shouldn't be a problem.
 */
static int umstd_flush (const char *path, struct fuse_file_info *fileinfo)
{
	PRINTCALL("default flush %s\n", path);
	return 0; //maybe flush is not relevant
}

/** Release an open file
 *
 * Release is called when there are no more references to an open
 * file: all file descriptors are closed and all memory mappings
 * are unmapped.
 *
 * For every open() call there will be exactly one release() call
 * with the same flags and file descriptor.  It is possible to
 * have a file opened more than once, in which case only the last
 * release will mean, that no more reads/writes will happen on the
 * file.  The return value of release is ignored.
 */
static int umstd_release (const char *path, struct fuse_file_info *fileinfo)
{
	PRINTCALL("default release %s\n", path);
	return 0;
}

/** Synchronize file contents
 *
 * If the datasync parameter is non-zero, then only the user data
 * should be flushed, not the meta data.
 */
static int umstd_fsync (const char *path, int flags, struct fuse_file_info *fileinfo)
{
	PRINTCALL("default fsync %s\n", path);
	return 0;
}

/** Set extended attributes */
static int umstd_setxattr (const char *path, const char *name, const char *attr, size_t size, int flags)
{
	PRINTCALL("default setxattr %s\n", path);
	return -ENOSYS;
}

/** Get extended attributes */
static int umstd_getxattr (const char *path, const char *name, char *attr, size_t size)
{
	PRINTCALL("default getxattr %s\n", path);
	return -ENOSYS;
}

/** List extended attributes */
static int umstd_listxattr (const char *path, char *addrlist, size_t size)
{
	PRINTCALL("default listxattr %s\n", path);
	return -ENOSYS;
}

/** Remove extended attributes */
static int umstd_removexattr (const char *path, const char *name)
{
	PRINTCALL("default removexattr %s\n", path);
	return -ENOSYS;
}

/** Open directory */
static int umstd_opendir (const char *path, struct fuse_file_info *fileinfo)
{
	PRINTCALL("default opendir %s\n", path);
	return 0;
}

#if 0
/** Read directory */
static int umstd_readdir (const char *path, void *buf, fuse_fill_dir_t filler, 
		off_t offset, struct fuse_file_info *fileinfo)
{
	printf("default readdir %s\n", path);
	return -ENOSYS;
}
#endif 

/** Release directory */
static int umstd_releasedir (const char *path, struct fuse_file_info *fileinfo)
{
	PRINTCALL("default removexattr %s\n", path);
	return 0;
}

/** Synchronize directory contents */
static int umstd_fsyncdir (const char *path, int user_meta, struct fuse_file_info *fileinfo)
{
	PRINTCALL("default fsyncdir %s\n", path);
	return -ENOSYS;
}

#if 0
/**
 * Check file access permissions
 *
 */
static int umstd_access (const char *path, int mode)
{
	PRINTCALL("default access %s\n", path);
	return -ENOSYS;
}

/**
 * Create and open a file
 */
static int umstd_create (const char *path, mode_t mode, struct fuse_file_info *fileinfo)
{
	PRINTCALL("default create %s\n", path);
	return -ENOSYS;
}

/**
 * Change the size of an open file
 */
static int umstd_ftruncate (const char *path, off_t length, struct fuse_file_info *fileinfo)
{
	PRINTCALL("default ftruncate %s\n", path);
	return -ENOSYS;
}

static int umstd_fgetattr (const char *path, struct stat *buf, struct fuse_file_info *fileinfo)
{
	PRINTCALL("default ftruncate %s\n", path);
	return -ENOSYS;
}
#endif

struct fuse_operations defaultservice={
	.getattr = umstd_getattr,
	.readlink = umstd_readlink,
	.getdir = umstd_getdir,
	.mknod = umstd_mknod,
	.mkdir = umstd_mkdir,
	.unlink = umstd_unlink,
	.rmdir = umstd_rmdir,
	.symlink = umstd_symlink,
	.rename = umstd_rename,
	.link = umstd_link,
	.chmod = umstd_chmod,
	.chown = umstd_chown,
	.truncate = umstd_truncate,
	.utime = umstd_utime,
	.open = umstd_open,
	.read = umstd_read,
	.write = umstd_write,
	.statfs = umstd_statfs,
	.flush = umstd_flush,
	.release = umstd_release,
	.fsync = umstd_fsync,
	.setxattr = umstd_setxattr,
	.getxattr = umstd_getxattr,
	.listxattr = umstd_listxattr,
	.removexattr = umstd_removexattr,
	.opendir = umstd_opendir,
	.readdir = NULL,
	.releasedir = umstd_releasedir,
	.fsyncdir = umstd_fsyncdir,
	.init = NULL,
	.destroy = NULL,
#if ( FUSE_MINOR_VERSION >= 5 )
	.access = NULL,
	.create = NULL,
	.ftruncate = NULL,
	.fgetattr = NULL,
#endif
};
