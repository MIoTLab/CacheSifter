#ifndef _LINUX_ERAMFS_H
#define _LINUX_ERAMFS_H

#include <linux/types.h>
#include <linux/page-flags.h>
#include <linux/fs.h>
#include <linux/magic.h>
#include <linux/kobject.h>
#include <linux/sched.h>

#define ERAMFS_MAGIC		0x83a83514	/* some random number */

struct eramfs_mount_opts {
	umode_t mode;
	int total_size;
};

struct eramfs_fs_info {
	struct eramfs_mount_opts mount_opts;
	spinlock_t stat_lock;
	unsigned long long free_bytes;
	spinlock_t list_lock;
	struct list_head inode_list;
};

struct eramfs_inode_info {
	struct inode vfs_inode;
	short ftype; // file type
    short prior;
	struct list_head inode_lru;
};

static inline struct eramfs_inode_info *ERAMFS_I(struct inode *inode)
{
	return container_of(inode, struct eramfs_inode_info, vfs_inode);
}

static inline struct eramfs_fs_info *ERAMFS_SB(struct super_block *sb)
{
	return sb->s_fs_info;
}

static inline struct eramfs_fs_info *ERAMFS_I_SB(struct inode *inode)
{
	return ERAMFS_SB(inode->i_sb);
}

static inline struct eramfs_fs_info *ERAMFS_M_SB(struct address_space *mapping)
{
	return ERAMFS_I_SB(mapping->host);
}

static inline struct eramfs_fs_info *ERAMFS_P_SB(struct page *page)
{
	return ERAMFS_M_SB(page->mapping);
}

static inline void update_free_space(struct eramfs_fs_info *sbi, long long size, bool inc)
{
	spin_lock(&sbi->stat_lock);
	if(inc)
		sbi->free_bytes += size;
	else
		sbi->free_bytes -= size;
	spin_unlock(&sbi->stat_lock);
}

void add_to_inode_list_tail(struct eramfs_fs_info *sbi, struct inode *inode, bool in_inode_list);
void add_to_inode_list(struct eramfs_fs_info *sbi, struct inode *inode, bool in_inode_list);
void del_from_inode_list(struct eramfs_fs_info *sbi, struct inode *inode);

#endif