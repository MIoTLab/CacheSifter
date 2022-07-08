#include <linux/list.h>
#include <linux/slab.h>
#include <linux/kthread.h>
#include <linux/printk.h>
#include <linux/mount.h>
#include <linux/security.h>
#include <linux/fsnotify.h>
#include <linux/f2fs_fs.h>
#include <linux/swap.h>
#include <linux/freezer.h>
#include <linux/mm_inline.h>

#include "f2fs.h"
#include "kernelMLP.h"
#include "../mount.h"


#define BG_THREAD_INTERVAL 10 /* seconds */


#define CLASSIFY_INTERVAL 10
#define CLASSIFY_CNT (CLASSIFY_INTERVAL / BG_THREAD_INTERVAL)

#define SHRINK_INTERVAL 20
#define SHRINK_CNT (SHRINK_INTERVAL / BG_THREAD_INTERVAL)

#define ACT_TO_INACT_TIME 20

#define MAX_RAMFS_SIZE (20 * 1024 * 1024UL) // 最大允许active list的数据量，单位字节
#define HIGH_WATERMARK_RATIO (90)
#define FG_RECLAIM_WATERMARK_RATIO (80)
#define LOW_WATERMARK_RATIO  (50)

struct kmem_cache *f2fs_filemgr_entry_slab;
struct filemgr_list filemgr_list;
struct mlp_model stage1_model;
struct mlp_model stage2_model;

extern int fgapp_uid;

void classify_files(void);
void shrink_files(int);
void __shrink_files(int, int);
extern int isolate_lru_page(struct page *page);

int shrink_filemgr_func(void *data)
{
    struct f2fs_sb_info *sbi = data;
	wait_queue_head_t *wq = &sbi->filemgr_thread->filemgr_wait_queue_head;
    long wait_ms = BG_THREAD_INTERVAL * 1000;
    int classify_cnt = 0, shrink_cnt = 0;

    do {

        if (try_to_freeze())
			continue;
		else
			wait_event_interruptible_timeout(*wq,
						kthread_should_stop(),
						msecs_to_jiffies(wait_ms));

        if (kthread_should_stop())
			break;

        prink_list_info();


		if(classify_cnt == CLASSIFY_CNT) {
			classify_files(); // 开始分类
			classify_cnt = 0;
		}

		if(shrink_cnt == SHRINK_CNT) {
			shrink_files(0);
			shrink_cnt = 0;
		}

        prink_list_info();

        classify_cnt++;
		shrink_cnt++;

    } while (!kthread_should_stop());

    return 0;
}

int start_filemgr_thread(struct f2fs_sb_info *sbi)
{
	struct f2fs_filemgr_kthread *filemgr_th;
	int err = 0;

	filemgr_th = kmalloc(sizeof(struct f2fs_filemgr_kthread), GFP_KERNEL);
	if (!filemgr_th) {
		err = -ENOMEM;
		goto out;
	}

	sbi->filemgr_thread = filemgr_th;
	init_waitqueue_head(&sbi->filemgr_thread->filemgr_wait_queue_head);
	sbi->filemgr_thread->filemgr_task = kthread_run(shrink_filemgr_func, sbi, "%s", "create filemgr thread");
	if (IS_ERR(filemgr_th->filemgr_task)) {
		err = PTR_ERR(filemgr_th->filemgr_task);
		kfree(filemgr_th);
		sbi->filemgr_thread = NULL;
	}
out:
    return err;
}

void stop_filemgr_thread(struct f2fs_sb_info *sbi)
{
	struct f2fs_filemgr_kthread *filemgr_th = sbi->filemgr_thread;
	if (!filemgr_th)
		return;
	kthread_stop(filemgr_th->filemgr_task);
	kfree(filemgr_th);
	sbi->filemgr_thread = NULL;
}


void prink_list_info(void) 
{
    int cnt = 0;
    struct inode_entry *ie, *next_ie;

    // printk(KERN_ALERT "[filemgr-prink_list_info] INIT_INODE_LIST SIZE = %d， STAGE1_INODE_LIST = %d, ACTIVE_INODE_LIST LIST SIZE = %d, INACTIVE_INODE_LIST = %d\n", 
    //             filemgr_list.nr_ilist[INIT_INODE_LIST], filemgr_list.nr_ilist[STAGE1_INODE_LIST], filemgr_list.nr_ilist[ACTIVE_INODE_LIST], filemgr_list.nr_ilist[INACTIVE_INODE_LIST]);

    // spin_lock(&filemgr_list.list_lock);

    // list_for_each_entry_safe(ie, next_ie, &filemgr_list.ilist[INIT_INODE_LIST], list) {
    //     printk(KERN_ALERT "INIT_INODE_LIST ino[%d] = %lu\n", cnt, ie->inode->i_ino);
    //     cnt++;
    // }

    // cnt = 0;
    // list_for_each_entry_safe(ie, next_ie, &filemgr_list.ilist[STAGE1_INODE_LIST], list) {
    //     printk(KERN_ALERT "STAGE1_INODE_LIST LIST ino[%d] = %lu\n", cnt, ie->inode->i_ino);
    //     cnt++;
    // }

    // cnt = 0;
    // list_for_each_entry_safe(ie, next_ie, &filemgr_list.ilist[ACTIVE_INODE_LIST], list) {
    //     printk(KERN_ALERT "ACTIVE_INODE_LIST LIST ino[%d] = %lu\n", cnt, ie->inode->i_ino);
    //     cnt++;
    // }

    // cnt = 0;
    // list_for_each_entry_safe(ie, next_ie, &filemgr_list.ilist[INACTIVE_INODE_LIST], list) {
    //     printk(KERN_ALERT "INACTIVE LIST ino[%d] = %lu\n", cnt, ie->inode->i_ino);
    //     cnt++;
    // }
    // spin_unlock(&filemgr_list.list_lock);
}

struct inode_entry *find_inode_entry(struct filemgr_list *imlist, unsigned int ino)
{
	struct inode_entry *ie;

	ie = radix_tree_lookup(&imlist->iroot, ino);
	if (ie)
		return ie;
	return NULL;
}

int add_to_inode_list(struct inode *inode, int list_idx)
{
    int ret = 0;
    struct inode_entry *ie;
	struct f2fs_inode_info *fi = F2FS_I(inode);
    //struct timeval start, end;

    // !!! cannot handle from ACTIVE TO INIT
    if(list_idx != INIT_INODE_LIST && list_idx != ACTIVE_INODE_LIST)
        return -EINVAL;

    //do_gettimeofday(&start);
    if(list_idx == INIT_INODE_LIST)
        spin_lock(&filemgr_list.list_lock);
    else if(list_idx == ACTIVE_INODE_LIST)
        spin_lock(&filemgr_list.fill_lock);

    ie = find_inode_entry(&filemgr_list, inode->i_ino);
    if (ie) {
        list_del(&ie->list);
        ie->inode = inode;
        ie->ino = inode->i_ino;
        list_add(&ie->list, &filemgr_list.ilist[list_idx]);
        filemgr_list.nr_ilist[fi->list_index]--;
        filemgr_list.nr_ilist[list_idx]++;
    } else {
        ie = kmem_cache_alloc(f2fs_filemgr_entry_slab, GFP_NOFS);
        if (!ie) {
            ret = -ENOMEM;
            goto out;
        }
        ie->inode = inode;
        ie->ino = inode->i_ino;
        radix_tree_insert(&filemgr_list.iroot, inode->i_ino, ie);
        list_add(&ie->list, &filemgr_list.ilist[list_idx]);
        filemgr_list.nr_ilist[list_idx]++;
    }
    //fi->in_t_time = get_cur_time();
    fi->list_index = list_idx;
out:
    if(list_idx == INIT_INODE_LIST)
        spin_unlock(&filemgr_list.list_lock);
    else if(list_idx == ACTIVE_INODE_LIST)
        spin_unlock(&filemgr_list.fill_lock);
    //do_gettimeofday(&end);
    //printk(KERN_ALERT "[filemgr] add to list time diff = %u, list_idx = %u\n", ((end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec)), list_idx);
    return ret;
}

void remove_inode_entry(struct inode_entry *ie, int del_list)
{
    if(del_list)
        list_del(&ie->list);
    radix_tree_delete(&filemgr_list.iroot, ie->ino); // 接下来两行 必须同时使用，不然就会索引一个空的entry
    kmem_cache_free(f2fs_filemgr_entry_slab, ie);
}

int shrink_inode_from_inactive_list(struct list_head *evict_list, unsigned long long size)
{
    struct inode_entry *ie, *next_ie;
    int nr_evict = 0;
    ssize_t inode_size;

    if(size <= 0)
        return 0;

    list_for_each_entry_safe_reverse(ie, next_ie, &filemgr_list.ilist[INACTIVE_INODE_LIST], list) {
        if(ie && ie->inode) {
            uid_t file_uid;
            file_uid = i_uid_read(ie->inode);
            if(file_uid != fgapp_uid) {
                inode_size = i_size_read(ie->inode);
                list_del(&ie->list);
                list_add(&ie->list, evict_list);
                nr_evict++;
                size -= inode_size;
                filemgr_list.active_used_bytes -= inode_size;
                filemgr_list.nr_ilist[INACTIVE_INODE_LIST]--;
                if(size <= 0)
                    break;
            }
        }
    }

    // 如果空间还没有释放完
    if(size > 0) {
        list_for_each_entry_safe_reverse(ie, next_ie, &filemgr_list.ilist[INACTIVE_INODE_LIST], list) {
            if(ie && ie->inode) {
                inode_size = i_size_read(ie->inode);
                list_del(&ie->list);
                list_add(&ie->list, evict_list);
                nr_evict++;
                size -= inode_size;
                filemgr_list.active_used_bytes -= inode_size;
                filemgr_list.nr_ilist[INACTIVE_INODE_LIST]--;
                if(size <= 0)
                    break;
            }
        }
    }
    return nr_evict;
}

void shrink_inode_from_active_list(void)
{
    struct f2fs_inode_info *fi;
    struct inode_entry *ie, *next_ie;
    unsigned long now = get_cur_time();

    list_for_each_entry_safe_reverse(ie, next_ie, &filemgr_list.ilist[ACTIVE_INODE_LIST], list) {
        if(ie && ie->inode) {
            fi = F2FS_I(ie->inode);
            if(now - fi->create_time > ACT_TO_INACT_TIME) {
                list_del(&ie->list);
                list_add(&ie->list, &filemgr_list.ilist[INACTIVE_INODE_LIST]);
                fi->list_index = INACTIVE_INODE_LIST;
                filemgr_list.nr_ilist[ACTIVE_INODE_LIST]--;
                filemgr_list.nr_ilist[INACTIVE_INODE_LIST]++;
            }
        }
    }
}

int init_file_manager(void)
{
    int i;
    struct mlp_conf stage1_conf, stage2_conf;

    f2fs_filemgr_entry_slab = kmem_cache_create("f2fs_filemgr_entry_slab", 
                            sizeof(struct inode_entry), 
                            0, SLAB_RECLAIM_ACCOUNT, NULL);

    if(!f2fs_filemgr_entry_slab)
        return -ENOMEM;

    INIT_RADIX_TREE(&filemgr_list.iroot, GFP_NOFS);

	for(i = 0; i < NR_INODE_LIST; i++) {
		INIT_LIST_HEAD(&filemgr_list.ilist[i]);
        filemgr_list.nr_ilist[i] = 0;
    }


	filemgr_list.active_used_bytes = 0;
	filemgr_list.high_watermark_bytes = MAX_RAMFS_SIZE * HIGH_WATERMARK_RATIO / 100;
	filemgr_list.low_watermark_bytes = MAX_RAMFS_SIZE * LOW_WATERMARK_RATIO / 100;
    filemgr_list.fg_reclaim_watermark = MAX_RAMFS_SIZE * FG_RECLAIM_WATERMARK_RATIO / 100;

	spin_lock_init(&filemgr_list.list_lock);
	spin_lock_init(&filemgr_list.fill_lock);

    // stage1 model
    stage1_conf.m_w0 = 512; stage1_conf.n_w0 = 26;
    stage1_conf.m_w1 = 200; stage1_conf.n_w1 = 512;
    stage1_conf.m_w2 = 2; stage1_conf.n_w2 = 200;
    stage1_conf.m_w3 = 2; stage1_conf.n_w3 = 2;

    stage1_conf.m_b0 = 512; stage1_conf.n_b0 = 1;
    stage1_conf.m_b1 = 200; stage1_conf.n_b1 = 1;
    stage1_conf.m_b2 = 2; stage1_conf.n_b2 = 1;
    stage1_conf.m_b3 = 2; stage1_conf.n_b3 = 1;

    stage1_conf.m_scale = 26; stage1_conf.n_scale = 1;
    stage1_conf.m_mean = 26; stage1_conf.n_mean = 1;

    // stage2 model
    stage2_conf.m_w0 = 512; stage2_conf.n_w0 = 66;
    stage2_conf.m_w1 = 200; stage2_conf.n_w1 = 512;
    stage2_conf.m_w2 = 2; stage2_conf.n_w2 = 200;
    stage2_conf.m_w3 = 2; stage2_conf.n_w3 = 2;

    stage2_conf.m_b0 = 512; stage2_conf.n_b0 = 1;
    stage2_conf.m_b1 = 200; stage2_conf.n_b1 = 1;
    stage2_conf.m_b2 = 2; stage2_conf.n_b2 = 1;
    stage2_conf.m_b3 = 2; stage2_conf.n_b3 = 1;

    stage2_conf.m_scale = 66; stage2_conf.n_scale = 1;
    stage2_conf.m_mean = 66; stage2_conf.n_mean = 1;

	init_mlp(&stage1_model, stage1_conf);
    init_mlp2(&stage2_model, stage2_conf);

	return 0;
}

void destroy_file_manager(void)
{
    struct inode_entry *ie, *next_ie;
    int i;

    for(i = 0; i < NR_INODE_LIST; i++) {
    	list_for_each_entry_safe(ie, next_ie, &filemgr_list.ilist[i], list) {
            radix_tree_delete(&filemgr_list.iroot, ie->inode->i_ino);
            list_del(&ie->list);
            kmem_cache_free(f2fs_filemgr_entry_slab, ie);
        }
        list_for_each_entry_safe(ie, next_ie, &filemgr_list.ilist[i], list)
            remove_inode_entry(ie, 1);
	}

    free_mlp(&stage1_model);
    free_mlp(&stage2_model);
	kmem_cache_destroy(f2fs_filemgr_entry_slab);
}

void fill_inactive_list(void)
{
    if(filemgr_list.nr_ilist[ACTIVE_INODE_LIST] == 0)
        return;
	shrink_inode_from_active_list();
}

static void print_inode_stat(struct inode *inode, const char *str)
{
    struct f2fs_inode_info *fi = F2FS_I(inode);

    printk(KERN_ALERT "[filemgr] %ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%s\n", 
        fi->ac_stat_stage1[0], fi->ac_stat_stage1[1], fi->ac_stat_stage1[2], fi->ac_stat_stage1[3], fi->ac_stat_stage1[4], fi->ac_stat_stage1[5], fi->ac_stat_stage1[6], fi->ac_stat_stage1[7], fi->ac_stat_stage1[8], 
        fi->ac_stat_stage1[9], fi->ac_stat_stage1[10], fi->ac_stat_stage1[11], fi->ac_stat_stage1[12], fi->ac_stat_stage1[13], fi->ac_stat_stage1[14], fi->ac_stat_stage1[15], fi->ac_stat_stage1[16], fi->ac_stat_stage1[17], 
        fi->ac_stat_stage1[18], fi->ac_stat_stage1[19], fi->ac_stat_tl[0], fi->ac_stat_tl[1], fi->ac_stat_tl[2], fi->ac_stat_tl[3], fi->ac_stat_tl[4], fi->ac_stat_tl[5],str
    );
}

static void print_inode_stat2(struct inode *inode, const char *str)
{
    struct f2fs_inode_info *fi = F2FS_I(inode);

    printk(KERN_ALERT "[filemgr] %ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%s\n", 
        fi->ac_stat_stage1[0], fi->ac_stat_stage1[1], fi->ac_stat_stage1[2], fi->ac_stat_stage1[3], fi->ac_stat_stage1[4], fi->ac_stat_stage1[5], fi->ac_stat_stage1[6], fi->ac_stat_stage1[7], fi->ac_stat_stage1[8], 
        fi->ac_stat_stage1[9], fi->ac_stat_stage1[10], fi->ac_stat_stage1[11], fi->ac_stat_stage1[12], fi->ac_stat_stage1[13], fi->ac_stat_stage1[14], fi->ac_stat_stage1[15], fi->ac_stat_stage1[16], fi->ac_stat_stage1[17], 
        fi->ac_stat_stage1[18], fi->ac_stat_stage1[19], fi->ac_stat_stage2[0], fi->ac_stat_stage2[1], fi->ac_stat_stage2[2], fi->ac_stat_stage2[3], fi->ac_stat_stage2[4], fi->ac_stat_stage2[5], fi->ac_stat_stage2[6], 
        fi->ac_stat_stage2[7], fi->ac_stat_stage2[8], fi->ac_stat_stage2[9], fi->ac_stat_stage2[10], fi->ac_stat_stage2[11], fi->ac_stat_stage2[12], fi->ac_stat_stage2[13], fi->ac_stat_stage2[14], fi->ac_stat_stage2[15], 
        fi->ac_stat_stage2[16], fi->ac_stat_stage2[17], fi->ac_stat_stage2[18], fi->ac_stat_stage2[19], fi->ac_stat_stage2[20], fi->ac_stat_stage2[21], fi->ac_stat_stage2[22], fi->ac_stat_stage2[23], fi->ac_stat_stage2[24], 
        fi->ac_stat_stage2[25], fi->ac_stat_stage2[26], fi->ac_stat_stage2[27], fi->ac_stat_stage2[28], fi->ac_stat_stage2[29], fi->ac_stat_stage2[30], fi->ac_stat_stage2[31], fi->ac_stat_stage2[32], fi->ac_stat_stage2[33], 
        fi->ac_stat_stage2[34], fi->ac_stat_stage2[35], fi->ac_stat_stage2[36], fi->ac_stat_stage2[37], fi->ac_stat_stage2[38], fi->ac_stat_stage2[39],
        fi->ac_stat_tl[0], fi->ac_stat_tl[1], fi->ac_stat_tl[2], fi->ac_stat_tl[3], fi->ac_stat_tl[4], fi->ac_stat_tl[5],str
    );
}


void classify1(struct list_head *list, int nr_inodes)
{
    // 进行分类，这个函数处理完之后，每一个f2fs_inode_info的ftype必须有一个确定的类型
    struct inode_entry *ie;
	struct f2fs_inode_info *fi;
    struct inode **i_array;
    struct matrix *input, *out, *norm_input;
    int i = 0, j = 0;
    // struct timeval start, end;

    // do_gettimeofday(&start);
    i_array = (struct inode **)kmalloc(nr_inodes * sizeof(struct inode *), GFP_ATOMIC);
    if(!i_array)
        goto error_out;

    input = matrix_alloc(nr_inodes, 26); // 第一次分类是维度是26的输入矩阵
    if (!input) // NO MEMORY
        goto error_kfree;

    // fill the matrix // TODO: 
    list_for_each_entry(ie, list, list) {
        fi = F2FS_I(ie->inode);
        if (fi->last_access_time != 0)
            fi->ac_stat_tl[5] = fi->last_access_time - fi->create_time;
        else
            fi->ac_stat_tl[5] = 20;

        if (fi->ac_stat_tl[5] > 20)
            fi->ac_stat_tl[5] = 20;

        if (fi->ac_stat_tl[5] < 0)
            fi->ac_stat_tl[5] = 0;

        for (j = 0; j < 20; j++)
            input->data[i][j] = fi->ac_stat_stage1[j] * KERNEL_TIMES;
        
        input->data[i][20] = fi->ac_stat_tl[0] * KERNEL_TIMES;
        input->data[i][21] = fi->ac_stat_tl[1] * KERNEL_TIMES;
        input->data[i][22] = fi->ac_stat_tl[2] * KERNEL_TIMES;
        input->data[i][23] = fi->ac_stat_tl[3] * KERNEL_TIMES;
        input->data[i][24] = fi->ac_stat_tl[4] * KERNEL_TIMES;
        input->data[i][25] = fi->ac_stat_tl[5] * KERNEL_TIMES;
        i_array[i] = ie->inode;
        i++;
    } 

    norm_input = mlp_normalize(&stage1_model, input);
    matrix_release(input);

    if(!norm_input)
        goto error_kfree;

    out = mlp_transform(&stage1_model, norm_input);

    matrix_release(norm_input);

    if (!out)
        goto error_kfree;

    // 输出的结果是一个二类分类结果，1或者0
    for(i = 0; i < out->rows; i++) {
        struct inode *inode = i_array[i];
        if (out->data[i][0] == 0) {
            set_inode_ftype(inode, INODE_CLEAN);
            //print_inode_stat(inode, "INODE_CLEAN");
        } else {
        	F2FS_I(inode)->ac_stat_stage2 = kmalloc(sizeof(unsigned long) * 40, GFP_KERNEL|__GFP_ZERO);
            if(F2FS_I(inode)->ac_stat_stage2) { // is NULL, no memory
                set_inode_ftype(inode, INODE_STAGE1);
            } else {
                set_inode_ftype(inode, INODE_WRITEBACK);
            }
            
            //print_inode_stat(inode, "INODE_STAGE1");
        }
    }
    matrix_release(out);
    kfree(i_array);
    // do_gettimeofday(&end);
    // printk(KERN_ALERT "[filemgr] classify 1 time diff = %u\n", ((end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec)) / nr_inodes);
    return;
error_kfree:
    kfree(i_array);
error_out:
    list_for_each_entry(ie, list, list)
        set_inode_ftype(ie->inode, INODE_WRITEBACK); // 设置ftype
}

void classify2(struct list_head *list, int nr_inodes)
{
    // 进行分类，这个函数处理完之后，每一个f2fs_inode_info的ftype必须有一个确定的类型
    struct inode_entry *ie;
	struct f2fs_inode_info *fi;
    struct inode **i_array;
    struct matrix *input, *out, *norm_input;
    int i = 0, j = 0;
    // struct timeval start, end;

    // do_gettimeofday(&start);
    i_array = (struct inode **)kmalloc(nr_inodes * sizeof(struct inode *), GFP_ATOMIC);
    if(!i_array)
        goto error_out;

    input = matrix_alloc(nr_inodes, 66); // 第一次分类是维度是96的输入矩阵
    if (!input) // NO MEMORY
        goto error_kfree;

    // fill the matrix // TODO: 
    list_for_each_entry(ie, list, list) {
        fi = F2FS_I(ie->inode);
        if (fi->last_access_time != 0)
            fi->ac_stat_tl[5] = fi->last_access_time - fi->create_time;
        else
            fi->ac_stat_tl[5] = 60;

        if (fi->ac_stat_tl[5] > 60)
            fi->ac_stat_tl[5] = 60;

        if (fi->ac_stat_tl[5] < 0)
            fi->ac_stat_tl[5] = 0;

        for (j = 0; j < 20; j++)
            input->data[i][j] = fi->ac_stat_stage1[j] * KERNEL_TIMES;
        
        for (j = 20; j < 60; j++)
            input->data[i][j] = fi->ac_stat_stage2[j-20] * KERNEL_TIMES;

        input->data[i][60] = fi->ac_stat_tl[0] * KERNEL_TIMES;
        input->data[i][61] = fi->ac_stat_tl[1] * KERNEL_TIMES;
        input->data[i][62] = fi->ac_stat_tl[2] * KERNEL_TIMES;
        input->data[i][63] = fi->ac_stat_tl[3] * KERNEL_TIMES;
        input->data[i][64] = fi->ac_stat_tl[4] * KERNEL_TIMES;
        input->data[i][65] = fi->ac_stat_tl[5] * KERNEL_TIMES;
        i_array[i] = ie->inode;
        i++;
    } 

    norm_input = mlp_normalize(&stage2_model, input);
    matrix_release(input);

    if(!norm_input)
        goto error_kfree;

    out = mlp_transform2(&stage2_model, norm_input);
    matrix_release(norm_input);

    if (!out)
        goto error_kfree;

    // 输出的结果是一个二类分类结果，1或者0
    for(i = 0; i < out->rows; i++) {
        struct inode *inode = i_array[i];

        if (out->data[i][0] == 1) {
            set_inode_ftype(inode, INODE_UNEVICTABLE);
            //print_inode_stat2(inode, "INODE_UNEVICTABLE");
        } else {
            set_inode_ftype(inode, INODE_WRITEBACK);
            //print_inode_stat2(inode, "INODE_WRITEBACK");
        }
    }
    matrix_release(out);
    kfree(i_array);
    // do_gettimeofday(&end);
    // printk(KERN_ALERT "[filemgr] classify 2 time diff = %u\n", ((end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec)) / nr_inodes);
    return;
error_kfree:
    kfree(i_array);
error_out:
    list_for_each_entry(ie, list, list)
        set_inode_ftype(ie->inode, INODE_WRITEBACK); // 设置ftype
}

ssize_t handle_unevictable_inode(struct inode *inode)
{
    int ret = 0;
    ssize_t inode_size;
    struct f2fs_inode_info *fi;

    mutex_lock(&inode->i_mutex);

    inode_size = i_size_read(inode);
    if (filemgr_list.active_used_bytes + inode_size > filemgr_list.high_watermark_bytes) { // 没有空间了
        mutex_unlock(&inode->i_mutex);
        fill_inactive_list();
        __shrink_files(1, 1); // 前台回收
        mutex_lock(&inode->i_mutex);
    }

    if (filemgr_list.active_used_bytes + inode_size > filemgr_list.high_watermark_bytes) { // 依然没有空间，不管理这个文件了
        ret = -1;
        goto out;
    }

    fi = F2FS_I(inode);
    fi->create_time = get_cur_time();

    ret = inode_size;
out:
    mutex_unlock(&inode->i_mutex);
    return ret;
}

void handle_writeback_inode(struct inode *inode)
{
    int ret = 0;
    ssize_t inode_size;
    pgoff_t start_index, end_index;

    mutex_lock(&inode->i_mutex);

    inode_size = i_size_read(inode);
    clear_inode_flag(F2FS_I(inode), FI_RAMFS);
    mapping_clear_unevictable(inode->i_mapping);

    start_index = 0;
	end_index = (inode_size + PAGE_SIZE - 1) >> PAGE_SHIFT;

    // add pages to unevictable list
    for (start_index = 0; start_index <= end_index; start_index++) {
        struct page *page;
        

		rcu_read_lock();
		page = radix_tree_lookup(&inode->i_mapping->page_tree, start_index);
		rcu_read_unlock();
        
		if (page) {
            lock_page(page);
            if(PageUnevictable(page)) { // 如果在unevitable list当中
                struct zone *zone = page_zone(page);
                struct lruvec *lruvec;
                
                spin_lock_irq(&zone->lru_lock);
                lruvec = mem_cgroup_page_lruvec(page, zone);

                del_page_from_lru_list(page, lruvec, LRU_UNEVICTABLE); // 从unevitable list中删除
                ClearPageUnevictable(page);
                ClearPageActive(page);
                ClearPageReferenced(page);
                add_page_to_lru_list(page, lruvec, LRU_INACTIVE_FILE); // 插入到inactive list当中
                spin_unlock_irq(&zone->lru_lock);
            }

            set_page_dirty(page); // 调用这个函数之前已经是dirty了
            unlock_page(page);
        }
	}

out:
    mutex_unlock(&inode->i_mutex);
}

void handle_inode_by_type(struct list_head *inode_list)
{
    struct inode_entry *ie, *next_ie;
    ssize_t dsize = 0;
    static unsigned int nr_bar = 0, nr_mem = 0, nr_cln = 0, nr_stage1 = 0;
    struct f2fs_inode_info *fi;
    // struct timeval start, end;

    // do_gettimeofday(&start);

    list_for_each_entry_safe_reverse(ie, next_ie, inode_list, list) { // 遍历处理每一个f2fs_inode_indo的类型
        if (ie && ie->inode) {
            list_del(&ie->list);
            if (is_inode_ftype(ie->inode, INODE_UNEVICTABLE)) { // INODE_UNEVICTABLE
                ssize_t isize = handle_unevictable_inode(ie->inode);
                if(isize > 0) { //
                    filemgr_list.active_used_bytes += isize;
                    filemgr_list.nr_ilist[ACTIVE_INODE_LIST]++;
                    list_add(&ie->list, &filemgr_list.ilist[ACTIVE_INODE_LIST]); // 新分类的文件都进入ACTIVE LIST
                    nr_mem++;
                } else { // No memory
                    handle_writeback_inode(ie->inode);
                    remove_inode_entry(ie, 0); // dont manage this type of files
                    nr_cln++;
                }

            } else if (is_inode_ftype(ie->inode, INODE_CLEAN)) { // INODE_CLEAN
                dsize += i_size_read(ie->inode);
                clear_inode_flag(F2FS_I(ie->inode), FI_RAMFS);
                unlink_file_by_inode(ie->inode, 1);
                remove_inode_entry(ie, 0);
                nr_bar++;
            } else if(is_inode_ftype(ie->inode, INODE_STAGE1)) {
                filemgr_list.nr_ilist[STAGE1_INODE_LIST]++;
                list_add(&ie->list, &filemgr_list.ilist[STAGE1_INODE_LIST]); // 新分类的文件都进入ACTIVE LIST
                nr_stage1++;
            } else { // WRITEBACK处理
            	fi = F2FS_I(ie->inode);
            	if(fi->ac_stat_stage1) {
            		kfree(fi->ac_stat_stage1);
            	}
            	if(fi->ac_stat_stage2) {
            		kfree(fi->ac_stat_stage2);
            	}
                handle_writeback_inode(ie->inode);
                remove_inode_entry(ie, 0); // dont manage this type of files
                nr_cln++;
            }
        }
    }
    // do_gettimeofday(&end);
    // printk(KERN_ALERT "[filemgr-handle_inode_by_type] delete size = %lu kb, nr_bar = %u, nr_mem = %u, nr_wb = %u, nr_stage1 = %u\n", dsize / 1024, nr_bar, nr_mem, nr_cln, nr_stage1);
}

void classify_files(void)
{
    int nr_inode = 0;
    struct inode_entry *ie, *next_ie;
    LIST_HEAD(inode_list);
    unsigned long now = get_cur_time();

    // step 1: 先遍历STAGE1_INODE_LIST的list中已经第一次分类完的inode，这里可能不需要迁移到临时的list中
    list_for_each_entry_safe_reverse(ie, next_ie, &filemgr_list.ilist[STAGE1_INODE_LIST], list) {
        struct f2fs_inode_info *fi = F2FS_I(ie->inode);
        if(now - fi->create_time >= 60) { 
            list_del(&ie->list); 
            filemgr_list.nr_ilist[STAGE1_INODE_LIST]--;
            if(!ie->inode) {
                remove_inode_entry(ie, 0);
                continue;
            }
            list_add(&ie->list, &inode_list); // 加入到临时的list
            nr_inode++;
        }
    }

    // step 2: 处理第二次分类的结果
    if(nr_inode > 0) {
        classify2(&inode_list, nr_inode); 
        spin_lock(&filemgr_list.fill_lock);
        handle_inode_by_type(&inode_list);
        spin_unlock(&filemgr_list.fill_lock);
    }

    // step 3:  先遍历INIT_INODE_LIST的list中的inode，进行第一次分类
    nr_inode = 0;
    spin_lock(&filemgr_list.list_lock);
    list_for_each_entry_safe_reverse(ie, next_ie, &filemgr_list.ilist[INIT_INODE_LIST], list) {
        struct f2fs_inode_info *fi = F2FS_I(ie->inode);
        if(now - fi->create_time >= 20) {
            list_del(&ie->list);
            filemgr_list.nr_ilist[INIT_INODE_LIST]--;
            if(!ie->inode) {
                remove_inode_entry(ie, 0);
                continue;
            }
            list_add(&ie->list, &inode_list); // 加入到临时的list
            nr_inode++;
        }
    }
    spin_unlock(&filemgr_list.list_lock); // 此时INIT_INODE_LIST的list已经没有数据了，可以解锁了

    // step 4:  处理第一次分类的结果
    if(nr_inode > 0) {
        classify1(&inode_list, nr_inode); // 进行第一次分类，将数据写到STAGE1_INODE_LIST
        spin_lock(&filemgr_list.fill_lock);
        handle_inode_by_type(&inode_list);
        spin_unlock(&filemgr_list.fill_lock);
    }

    fill_inactive_list(); // 只有第二次分类才会产生active的数据
}

void __shrink_files(int is_fg, int is_lock)
{
    int nr_evict;
    struct inode_entry *ie;
    LIST_HEAD(inode_evict_list);
    ssize_t evict_size;

    if(!is_fg && filemgr_list.active_used_bytes < filemgr_list.low_watermark_bytes) // 如果是后台，且还没达到水线，就马上返回
        return;

    if(is_fg)
        evict_size = filemgr_list.active_used_bytes - filemgr_list.fg_reclaim_watermark; // 回收到低水线以下;
    else
        evict_size = filemgr_list.active_used_bytes - filemgr_list.low_watermark_bytes;
        

    nr_evict = shrink_inode_from_inactive_list(&inode_evict_list, evict_size);

    for(; nr_evict; nr_evict--) {
    	ie = list_last_entry(&inode_evict_list, struct inode_entry, list);
        if(ie && ie->inode) {
            unlink_file_by_inode(ie->inode, is_lock);
            remove_inode_entry(ie, 1);
        }
    }
}

void shrink_files(int is_fg)
{
    spin_lock(&filemgr_list.fill_lock);
    __shrink_files(is_fg, 1);
    spin_unlock(&filemgr_list.fill_lock);
}

int filemgr_unlink(struct inode *dir, struct dentry *dentry, int is_lock)
{
	struct inode *target = dentry->d_inode;
	int error = 0;

	if (!dir->i_op->unlink)
		return -EPERM;

    if(is_lock)
	    mutex_lock(&target->i_mutex);

    if(!is_inode_flag_set(F2FS_I(target), FI_RAMFS_DELETED)){ // 如果没有被用户删除，那么i_count=3，所以先-1
        atomic_add_unless(&target->i_count, -1, 2);
        clear_inode_flag(F2FS_I(target), FI_RAMFS_DELETED);
    }
    //printk(KERN_ALERT "[filemgr] delete: %s\n", dentry->d_name.name);
    error = dir->i_op->unlink(dir, dentry);

    if (!error) {
        dont_mount(dentry);
        detach_mounts(dentry);
    }

    if(is_lock)
	    mutex_unlock(&target->i_mutex);

	/* We don't d_delete() NFS sillyrenamed files--they still exist. */
	if (!error)
		d_delete(dentry);
	return error;
}

int unlink_file_by_inode(struct inode *inode, int is_lock)
{
	int err = 0;
	struct inode *dir;
	struct hlist_node *next;
	struct dentry *dentry;

    if(!inode || !inode->i_mapping)
        return -1;

    next = inode->i_dentry.first;

	if(next) {
		dentry = hlist_entry(next, struct dentry, d_u.d_alias); // do not get the reference of this dentry
        if(!IS_ERR(dentry)) {
            if(dentry->d_parent && dentry->d_parent->d_inode) {
                dir = dentry->d_parent->d_inode;
                mutex_lock_nested(&dir->i_mutex, I_MUTEX_PARENT);
                ihold(inode);
                err = filemgr_unlink(dir, dentry, is_lock);
                dput(dentry);
                mutex_unlock(&dir->i_mutex);
                if (inode)
		            iput(inode);
            }
        }
    }

    return err;
}

