#include <linux/list.h>

#include "eramfs.h"


void add_to_inode_list_tail(struct eramfs_fs_info *sbi, struct inode *inode, bool in_inode_list)
{
	struct eramfs_inode_info *fi = ERAMFS_I(inode);
    spin_lock(&sbi->list_lock);
    if(in_inode_list)
        list_del(&fi->inode_lru);
	list_add_tail(&fi->inode_lru, &sbi->inode_list);
    spin_unlock(&sbi->list_lock);
}

void add_to_inode_list(struct eramfs_fs_info *sbi, struct inode *inode, bool in_inode_list)
{
	struct eramfs_inode_info *fi = ERAMFS_I(inode);
    spin_lock(&sbi->list_lock);
    if(in_inode_list)
        list_del(&fi->inode_lru);
	list_add(&fi->inode_lru, &sbi->inode_list);
    spin_unlock(&sbi->list_lock);
}

void del_from_inode_list(struct eramfs_fs_info *sbi, struct inode *inode)
{
	struct eramfs_inode_info *fi = ERAMFS_I(inode);
    spin_lock(&sbi->list_lock);
    list_del(&fi->inode_lru);
    spin_unlock(&sbi->list_lock);
}