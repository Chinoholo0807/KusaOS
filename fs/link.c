/*************************************************************************//**
 *****************************************************************************
 * @file   link.c
 * @brief  
 *****************************************************************************
 *****************************************************************************/

#include "type.h"
#include "stdio.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "fs.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "keyboard.h"
#include "proto.h"


/*****************************************************************************
 *                                do_unlink
 *****************************************************************************/
/**
 * 删除一个文件
 *
 * @我们清除了inode-array里的inode，尽管这并不是必要的
 *     没有删除数据区的内容，所以文件是可恢复的
 * 
 * @return On success, zero is returned.  On error, -1 is returned.
 *****************************************************************************/
PUBLIC int do_unlink()
{
	char pathname[MAX_PATH];

	/* 从message获取参数 */
	int name_len = fs_msg.NAME_LEN;	/* 文件名的长度 */
	int src = fs_msg.source;	/* 调用者的进程号. */
	assert(name_len < MAX_PATH);
	phys_copy((void*)va2la(TASK_FS, pathname),
		  (void*)va2la(src, fs_msg.PATHNAME),
		  name_len);
	pathname[name_len] = 0;

	if (strcmp(pathname , "/") == 0) {
		printl("{FS} FS:do_unlink():: cannot unlink the root\n");
		return -1;
	}

	int inode_nr = search_file(pathname);
	if (inode_nr == INVALID_INODE) {	/* 文件没有找到 */
		printl("{FS} FS::do_unlink():: search_file() returns "
			"invalid inode: %s\n", pathname);
		return -1;
	}

	char filename[MAX_PATH];
	struct inode * dir_inode;
	if (strip_path(filename, pathname, &dir_inode) != 0)
		return -1;

	struct inode * pin = get_inode(dir_inode->i_dev, inode_nr);

	if (pin->i_mode != I_REGULAR && (pin->i_mode == I_DIRECTORY && pin->i_size != 0)) { /* 只能删除普通文件或空目录 */
		printl("{FS} cannot remove file %s, because "
		       "it is not a regular file or an empty dir.\n",
		       pathname);
		//printl("{FS} imode is %d \n",pin_i_mode);
		//printl("{FS} isize is %d \n",pin->i_size);
		return -1;
	}

	if (pin->i_cnt > 1 && pin->i_mode != I_DIRECTORY) {	/* 当前文件正在被打开 */
		printl("{FS} cannot remove file %s, because pin->i_cnt is %d.\n",
		       pathname, pin->i_cnt);
		return -1;
	}

	struct super_block * sb = get_super_block(pin->i_dev);

	/*************************/
	/* 释放inode-map中的bit位 */
	/*************************/
	int byte_idx = inode_nr / 8;
	int bit_idx = inode_nr % 8;
	assert(byte_idx < SECTOR_SIZE);	/* 我们的inode-map只有一个扇区 */
	/* 读取扇区2 (跳过bootsect 和超级块): */
	RD_SECT(pin->i_dev, 2);
	assert(fsbuf[byte_idx % SECTOR_SIZE] & (1 << bit_idx));
	fsbuf[byte_idx % SECTOR_SIZE] &= ~(1 << bit_idx);
	WR_SECT(pin->i_dev, 2);

	/**************************/
	/* 释放sector-map中的bit位 */
	/**************************/
	/*
	 *           bit_idx: bit idx in the entire i-map
	 *     ... ____|____
	 *                  \        .-- byte_cnt: how many bytes between
	 *                   \      |              the first and last byte
	 *        +-+-+-+-+-+-+-+-+ V +-+-+-+-+-+-+-+-+
	 *    ... | | | | | |*|*|*|...|*|*|*|*| | | | |
	 *        +-+-+-+-+-+-+-+-+   +-+-+-+-+-+-+-+-+
	 *         0 1 2 3 4 5 6 7     0 1 2 3 4 5 6 7
	 *  ...__/
	 *      byte_idx: byte idx in the entire i-map
	 */
	bit_idx  = pin->i_start_sect - sb->n_1st_sect + 1;
	byte_idx = bit_idx / 8;
	int bits_left = pin->i_nr_sects;
	int byte_cnt = (bits_left - (8 - (bit_idx % 8))) / 8;

	/* 当前sector编号 */
	int s = 2  /* 2: bootsect + superblk */
		+ sb->nr_imap_sects + byte_idx / SECTOR_SIZE;

	RD_SECT(pin->i_dev, s);

	int i;
	/* 清除第一个字节 */
	for (i = bit_idx % 8; (i < 8) && bits_left; i++,bits_left--) {
		assert((fsbuf[byte_idx % SECTOR_SIZE] >> i & 1) == 1);
		fsbuf[byte_idx % SECTOR_SIZE] &= ~(1 << i);
	}

	/* 清除剩余的字节（除了最后一个） */
	int k;
	i = (byte_idx % SECTOR_SIZE) + 1;	/* 第二个字节 */
	for (k = 0; k < byte_cnt; k++,i++,bits_left-=8) {
		if (i == SECTOR_SIZE) {
			i = 0;
			WR_SECT(pin->i_dev, s);
			RD_SECT(pin->i_dev, ++s);
		}
		assert(fsbuf[i] == 0xFF);
		fsbuf[i] = 0;
	}

	/* 清除最后一个字节 */
	if (i == SECTOR_SIZE) {
		i = 0;
		WR_SECT(pin->i_dev, s);
		RD_SECT(pin->i_dev, ++s);
	}
	unsigned char mask = ~((unsigned char)(~0) << bits_left);
	assert((fsbuf[i] & mask) == mask);
	fsbuf[i] &= (~0) << bits_left;
	WR_SECT(pin->i_dev, s);

	/***************************/
	/* 清除inode */
	/***************************/
	pin->i_mode = 0;
	pin->i_size = 0;
	pin->i_start_sect = 0;
	pin->i_nr_sects = 0;
	sync_inode(pin);
	/* 释放内存中inode_table[]对应的项 */
	put_inode(pin);

	/************************************************/
	/* 将对应目录项中的inode编号设为0 */
	/************************************************/
	int dir_blk0_nr = dir_inode->i_start_sect;
	int nr_dir_blks = (dir_inode->i_size + SECTOR_SIZE) / SECTOR_SIZE;
	int nr_dir_entries =
		dir_inode->i_size / DIR_ENTRY_SIZE; /* including unused slots
						     * (the file has been
						     * deleted but the slot
						     * is still there)
						     */
	int m = 0;
	struct dir_entry * pde = 0;
	int flg = 0;
	int dir_size = 0;

	for (i = 0; i < nr_dir_blks; i++) {
		RD_SECT(dir_inode->i_dev, dir_blk0_nr + i);

		pde = (struct dir_entry *)fsbuf;
		int j;
		for (j = 0; j < SECTOR_SIZE / DIR_ENTRY_SIZE; j++,pde++) {
			if (++m > nr_dir_entries)
				break;

			if (pde->inode_nr == inode_nr) {
				/* pde->inode_nr = 0; */
				memset(pde, 0, DIR_ENTRY_SIZE);
				WR_SECT(dir_inode->i_dev, dir_blk0_nr + i);
				flg = 1;
				break;
			}

			if (pde->inode_nr != INVALID_INODE)
				dir_size += DIR_ENTRY_SIZE;
		}

		if (m > nr_dir_entries || /* 已经遍历所有项或者 */
		    flg) /* 文件被找到 */
			break;
	}
	assert(flg);
	if (m == nr_dir_entries) { /* 文件是目录中的最后一个 */
		dir_inode->i_size = dir_size;
		sync_inode(dir_inode);
	}

	return 0;
}

PUBLIC int do_rename(){
	char pathname[MAX_PATH];
	char name[MAX_PATH];
	

	/* 从message获取参数 */
	int name_len = fs_msg.NAME_LEN;	/* 文件名的长度 */
	int change_len = fs_msg.BUF_LEN;
	int src = fs_msg.source;	/* 调用者的进程号. */
	assert(name_len < MAX_PATH);
	phys_copy((void*)va2la(TASK_FS, pathname),
		  (void*)va2la(src, fs_msg.PATHNAME),
		  name_len);
	pathname[name_len] = 0;
	phys_copy((void*)va2la(TASK_FS, name),
		  (void*)va2la(src, fs_msg.BUF),
		  name_len);
	name[change_len] = 0;

	if (strcmp(pathname , "/") == 0) {
		printl("{FS} FS:do_rename():: cannot rename the root\n");
		return -1;
	}

	int inode_nr = search_file(pathname);
	if (inode_nr == INVALID_INODE) {	/* 文件没有找到 */
		printl("{FS} FS::do_unlink():: search_file() returns "
			"invalid inode: %s\n", pathname);
		return -1;
	}

	char filename[MAX_PATH];
	struct inode * dir_inode;
	if (strip_path(filename, pathname, &dir_inode) != 0)
		return -1;

	struct inode * pin = get_inode(dir_inode->i_dev, inode_nr);

	if (pin->i_cnt > 1 && pin->i_mode != I_DIRECTORY) {	/* 当前文件正在被打开 */
		printl("{FS} cannot remove file %s, because pin->i_cnt is %d.\n",
		       pathname, pin->i_cnt);
			put_inode(pin);
		return -1;
	}

	int dir_blk0_nr = dir_inode->i_start_sect;
	int nr_dir_blks = (dir_inode->i_size + SECTOR_SIZE) / SECTOR_SIZE;
	int nr_dir_entries =
		dir_inode->i_size / DIR_ENTRY_SIZE; /* including unused slots
						     * (the file has been
						     * deleted but the slot
						     * is still there)
						     */
	int m = 0,i=0;
	struct dir_entry * pde = 0;
	int flg = 0;

	for (i = 0; i < nr_dir_blks; i++) {
		RD_SECT(dir_inode->i_dev, dir_blk0_nr + i);

		pde = (struct dir_entry *)fsbuf;
		int j;
		for (j = 0; j < SECTOR_SIZE / DIR_ENTRY_SIZE; j++,pde++) {
			if (++m > nr_dir_entries)
				break;

			if (pde->inode_nr == inode_nr) {
				pde->name[0]=0;
				strcat(pde->name,name);
				WR_SECT(dir_inode->i_dev, dir_blk0_nr + i);
				flg = 1;
				break;
			}
		}

		if (m > nr_dir_entries || /* 已经遍历所有项或者 */
		    flg) /* 文件被找到 */
			break;
	}

put_inode(pin);
	return 0;
}
