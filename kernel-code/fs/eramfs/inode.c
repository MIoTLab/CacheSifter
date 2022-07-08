/*
 * Resizable simple ram filesystem for Linux.
 *
 * Copyright (C) 2000 Linus Torvalds.
 *               2000 Transmeta Corp.
 *
 * Usage limits added by David Gibson, Linuxcare Australia.
 * This file is released under the GPL.
 */

/*
 * NOTE! This filesystem is probably most useful
 * not as a real filesystem, but as an example of
 * how virtual filesystems can be written.
 *
 * It doesn't get much simpler than this. Consider
 * that this file implements the full semantics of
 * a POSIX-compliant read-write filesystem.
 *
 * Note in particular how the filesystem does not
 * need to implement any data structures of its own
 * to keep track of the virtual data: using the VFS
 * caches is sufficient.
 */

#include <linux/fs.h>
#include <linux/pagemap.h>
#include <linux/highmem.h>
#include <linux/time.h>
#include <linux/init.h>
#include <linux/string.h>
#include <linux/backing-dev.h>
#include <linux/sched.h>
#include <linux/parser.h>
#include <linux/magic.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <linux/module.h>

#include "eramfs.h"

#define ERAMFS_DEFAULT_MODE	0755

static struct kmem_cache *eramfs_inode_cachep;
static const struct super_operations eramfs_ops;
static const struct inode_operations eramfs_dir_inode_operations;

enum {
	Opt_mode,
	Opt_size,
	Opt_err
};

static const match_table_t tokens = {
	{Opt_mode, "mode=%o"},
	{Opt_mode, "size=%d"},
	{Opt_err, NULL}
};

static int __init init_inodecache(void)
{
	eramfs_inode_cachep = kmem_cache_create("eramfs_inode_cache",
			sizeof(struct eramfs_inode_info), 0,
			SLAB_RECLAIM_ACCOUNT, NULL);
	if (!eramfs_inode_cachep)
		return -ENOMEM;
	return 0;
}

static void destroy_inodecache(void)
{
	rcu_barrier();
	kmem_cache_destroy(eramfs_inode_cachep);
}

static void eramfs_i_callback(struct rcu_head *head)
{
	struct inode *inode = container_of(head, struct inode, i_rcu);
	kmem_cache_free(eramfs_inode_cachep, ERAMFS_I(inode));
}

static void eramfs_destroy_inode(struct inode *inode)
{
	call_rcu(&inode->i_rcu, eramfs_i_callback);
}


static const struct address_space_operations eramfs_aops = {
	.readpage	= simple_readpage,
	.write_begin	= simple_write_begin,
	.write_end	= simple_write_end,
	.set_page_dirty	= __set_page_dirty_no_writeback,
};

static unsigned long eramfs_mmu_get_unmapped_area(struct file *file,
		unsigned long addr, unsigned long len, unsigned long pgoff,
		unsigned long flags)
{
	return current->mm->get_unmapped_area(file, addr, len, pgoff, flags);
}

const struct file_operations eramfs_file_operations = {
	.read_iter	= generic_file_read_iter,
	.write_iter	= generic_file_write_iter,
	.mmap		= generic_file_mmap,
	.fsync		= noop_fsync,
	.splice_read	= generic_file_splice_read,
	.splice_write	= iter_file_splice_write,
	.llseek		= generic_file_llseek,
	.get_unmapped_area	= eramfs_mmu_get_unmapped_area,
};

const struct inode_operations eramfs_file_inode_operations = {
	.setattr	= simple_setattr,
	.getattr	= simple_getattr,
};

struct inode *eramfs_get_inode(struct super_block *sb,
				const struct inode *dir, umode_t mode, dev_t dev)
{
	struct inode * inode = new_inode(sb);

	if (inode) {
		inode->i_ino = get_next_ino();
		inode_init_owner(inode, dir, mode);
		inode->i_mapping->a_ops = &eramfs_aops;
		mapping_set_gfp_mask(inode->i_mapping, GFP_HIGHUSER);
		mapping_set_unevictable(inode->i_mapping);
		inode->i_atime = inode->i_mtime = inode->i_ctime = CURRENT_TIME;
		switch (mode & S_IFMT) {
		default:
			init_special_inode(inode, mode, dev);
			break;
		case S_IFREG:
			inode->i_op = &eramfs_file_inode_operations;
			inode->i_fop = &eramfs_file_operations;
			break;
		case S_IFDIR:
			inode->i_op = &eramfs_dir_inode_operations;
			inode->i_fop = &simple_dir_operations;

			/* directory inodes start off with i_nlink == 2 (for "." entry) */
			inc_nlink(inode);
			break;
		case S_IFLNK:
			inode->i_op = &page_symlink_inode_operations;
			break;
		}
	}
	return inode;
}

/*
 * File creation. Allocate an inode, and we're done..
 */
/* SMP-safe */
static int
eramfs_mknod(struct inode *dir, struct dentry *dentry, umode_t mode, dev_t dev)
{
	struct inode * inode = eramfs_get_inode(dir->i_sb, dir, mode, dev);
	int error = -ENOSPC;

	if (inode) {
		d_instantiate(dentry, inode);
		dget(dentry);	/* Extra count - pin the dentry in core */
		error = 0;
		dir->i_mtime = dir->i_ctime = CURRENT_TIME;
	}
	return error;
}

static int eramfs_mkdir(struct inode * dir, struct dentry * dentry, umode_t mode)
{
	int retval = eramfs_mknod(dir, dentry, mode | S_IFDIR, 0);
	if (!retval)
		inc_nlink(dir);
	return retval;
}

static int eramfs_create(struct inode *dir, struct dentry *dentry, umode_t mode, bool excl)
{
	return eramfs_mknod(dir, dentry, mode | S_IFREG, 0);
}

static int eramfs_symlink(struct inode * dir, struct dentry *dentry, const char * symname)
{
	struct inode *inode;
	int error = -ENOSPC;

	inode = eramfs_get_inode(dir->i_sb, dir, S_IFLNK|S_IRWXUGO, 0);
	if (inode) {
		int l = strlen(symname)+1;
		error = page_symlink(inode, symname, l);
		if (!error) {
			d_instantiate(dentry, inode);
			dget(dentry);
			dir->i_mtime = dir->i_ctime = CURRENT_TIME;
		} else
			iput(inode);
	}
	return error;
}

static void init_once(void *foo)
{
	struct eramfs_inode_info *fi = (struct eramfs_inode_info *) foo;

	inode_init_once(&fi->vfs_inode);
}

static struct inode *eramfs_alloc_inode(struct super_block *sb)
{
	struct eramfs_inode_info *fi;

	fi = kmem_cache_alloc(eramfs_inode_cachep, GFP_NOFS | __GFP_ZERO);
	if (!fi)
		return NULL;

	init_once((void *) fi);

	/* Initialize ucfs-specific inode info */
	INIT_LIST_HEAD(&fi->inode_lru);
	add_to_inode_list_tail(ERAMFS_SB(sb), &fi->vfs_inode, false);
	return &fi->vfs_inode;
}


static const struct inode_operations eramfs_dir_inode_operations = {
	.create		= eramfs_create,
	.lookup		= simple_lookup,
	.link		= simple_link,
	.unlink		= simple_unlink,
	.symlink	= eramfs_symlink,
	.mkdir		= eramfs_mkdir,
	.rmdir		= simple_rmdir,
	.mknod		= eramfs_mknod,
	.rename		= simple_rename,
};

static const struct super_operations eramfs_ops = {
	.alloc_inode = eramfs_alloc_inode,
	.statfs		= simple_statfs,
	.destroy_inode	= eramfs_destroy_inode,
	.drop_inode	= generic_delete_inode,
	.show_options	= generic_show_options,
};

static int eramfs_parse_options(char *options, struct eramfs_mount_opts *opts)
{
	char *this_char, *value, *rest;

	opts->mode = ERAMFS_DEFAULT_MODE;

	// refer from tmpfs
	while (options != NULL) {
		this_char = options;

		for (;;) {

			options = strchr(options, ',');
			if (options == NULL)
				break;
			options++;
			if (!('0' <= *options && *options <= '9')) {
				options[-1] = '\0';
				break;
			}
		}

		if (!*this_char)
			continue;
		if ((value = strchr(this_char,'=')) != NULL) {
			*value++ = 0;
		} else {
			pr_err("eramfs: No value for mount option '%s'\n",
			       this_char);
			goto error;
		}

		if (!strcmp(this_char,"size")) {
			unsigned long long size;
			size = memparse(value, &rest);
			opts->total_size = size;
		} else {
			pr_err("tmpfs: Bad mount option %s\n", this_char);
			goto error;
		}
	}

	return 0;
error:
	return 1;
}

int eramfs_fill_super(struct super_block *sb, void *data, int silent)
{
	struct eramfs_fs_info *fsi;
	struct inode *inode;
	int err;

	save_mount_options(sb, data);

	fsi = kzalloc(sizeof(struct eramfs_fs_info), GFP_KERNEL);
	sb->s_fs_info = fsi;
	if (!fsi)
		return -ENOMEM;

	err = eramfs_parse_options(data, &fsi->mount_opts);
	if (err)
		return err;

	fsi->free_bytes = fsi->mount_opts.total_size;
	INIT_LIST_HEAD(&fsi->inode_list);
	spin_lock_init(&fsi->list_lock);
	spin_lock_init(&fsi->stat_lock);

	sb->s_maxbytes		= MAX_LFS_FILESIZE;
	sb->s_blocksize		= PAGE_SIZE;
	sb->s_blocksize_bits	= PAGE_SHIFT;
	sb->s_magic		= ERAMFS_MAGIC; // 
	sb->s_op		= &eramfs_ops;
	sb->s_time_gran		= 1;

	inode = eramfs_get_inode(sb, NULL, S_IFDIR | fsi->mount_opts.mode, 0);
	sb->s_root = d_make_root(inode);
	if (!sb->s_root)
		return -ENOMEM;

	return 0;
}

struct dentry *eramfs_mount(struct file_system_type *fs_type,
	int flags, const char *dev_name, void *data)
{
	return mount_nodev(fs_type, flags, data, eramfs_fill_super);
}

static void eramfs_kill_sb(struct super_block *sb)
{
	kfree(sb->s_fs_info);
	kill_litter_super(sb);
}

static struct file_system_type eramfs_fs_type = {
	.name		= "eramfs",
	.mount		= eramfs_mount,
	.kill_sb	= eramfs_kill_sb,
	.fs_flags	= FS_USERNS_MOUNT,
};

static int __init init_eramfs_fs(void)
{
	static unsigned long once;
	int err;

	err = init_inodecache();
	if (err)
		return err;
 
 	if (test_and_set_bit(0, &once)) {
		destroy_inodecache();
		return 0;
	}
 		
 	return register_filesystem(&eramfs_fs_type);
}

static void __exit exit_eramfs_fs(void)
{
	destroy_inodecache();
}

module_init(init_eramfs_fs);
module_exit(exit_eramfs_fs);
