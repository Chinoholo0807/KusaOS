/*************************************************************************//**
 *****************************************************************************
 * @file   fs/open.c
 * The file contains:
 *   - do_open()
 *   - do_close()
 *   - do_lseek()
 *   - create_file()
 *   - do_ls()
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

#define ISDIR -66378

PRIVATE struct inode * create_file(char * path, int flags);
PRIVATE int alloc_imap_bit(int dev);
PRIVATE int alloc_smap_bit(int dev, int nr_sects_to_alloc);
PRIVATE struct inode * new_inode(int dev, int inode_nr, int start_sect);
PRIVATE void new_dir_entry(struct inode * dir_inode, int inode_nr, char * filename);

/*****************************************************************************
 *                                do_open
 *****************************************************************************/
/**
 * 打开一个文件，并返回一个它的文件描述符
 * 
 * @return File descriptor if successful, otherwise a negative error code.
 *****************************************************************************/
PUBLIC int do_open()
{
	int fd = -1;		/* 返回值 */

	char pathname[MAX_PATH];

	/* 从msg中获取参数 */
	int flags = fs_msg.FLAGS;	/* 模式 */
	int name_len = fs_msg.NAME_LEN;	/* 文件长度 */
	int src = fs_msg.source;	/* 调用者的进程号. */
	assert(name_len < MAX_PATH);
	
	phys_copy((void*)va2la(TASK_FS, pathname),
		  (void*)va2la(src, fs_msg.PATHNAME),
		  name_len);
	pathname[name_len] = 0;

	/* 在PROCESS::filp[]找到空闲的地方 */
	int i;
	for (i = 0; i < NR_FILES; i++) {
		if (pcaller->filp[i] == 0) {
			fd = i;
			break;
		}
	}
	
	if ((fd < 0) || (fd >= NR_FILES))
		panic("filp[] is full (PID:%d)", proc2pid(pcaller));

	/* 在f_desc_table[]中找到空闲的地方 */
	for (i = 0; i < NR_FILE_DESC; i++)
		if (f_desc_table[i].fd_inode == 0)
			break;
	
	if (i >= NR_FILE_DESC)
		panic("f_desc_table[] is full (PID:%d)", proc2pid(pcaller));

	int inode_nr = search_file(pathname);

	struct inode * pin = 0;
	
	if (flags & O_CREAT) {
		if (inode_nr) {
			printl("{FS} file exists.\n");
			return -1;
		}
		else {
			pin = create_file(pathname, flags);
		}
	}
	else {
		assert(flags & O_RDWR);

		char filename[MAX_PATH];
		struct inode * dir_inode;
		if (strip_path(filename, pathname, &dir_inode) != 0){
			printl("filename:%s, pathname:%s\n", filename, pathname);
			return -1;
		}
		pin = get_inode(dir_inode->i_dev, inode_nr);
	}

	if (pin) {
		/* 连接进程与文件描述符 */
		pcaller->filp[fd] = &f_desc_table[i];

		/*连接文件描述符与inode */
		f_desc_table[i].fd_inode = pin;

		f_desc_table[i].fd_mode = flags;
		f_desc_table[i].fd_cnt = 1;
		f_desc_table[i].fd_pos = 0;

		int imode = pin->i_mode & I_TYPE_MASK;

		if (imode == I_CHAR_SPECIAL) {
			MESSAGE driver_msg;
			driver_msg.type = DEV_OPEN;
			int dev = pin->i_start_sect;
			driver_msg.DEVICE = MINOR(dev);
			assert(MAJOR(dev) == 4);
			assert(dd_map[MAJOR(dev)].driver_nr != INVALID_DRIVER);
			send_recv(BOTH,
				  dd_map[MAJOR(dev)].driver_nr,
				  &driver_msg);
		}
		else if (imode == I_DIRECTORY) {
			//assert(pin->i_num == ROOT_INODE);
			return 0;
		}
		else {
			assert(pin->i_mode == I_REGULAR);
		}
	}
	else {
		return -1;
	}

	return fd;
}

/*****************************************************************************
 *                                create_file
 *****************************************************************************/
/**
 * 创建一个文件，返回它的inode的指针
 *
 * @param[in] path   The full path of the new file
 * @param[in] flags  Attribiutes of the new file
 *
 * @return           Ptr to i-node of the new file if successful, otherwise 0.
 * 
 * @see open()
 * @see do_open()
 *****************************************************************************/
PRIVATE struct inode * create_file(char * path, int flags)
{
	char filename[MAX_PATH];
	struct inode * dir_inode;
	if (strip_path(filename, path, &dir_inode) != 0)
		return 0;

	int inode_nr = alloc_imap_bit(dir_inode->i_dev);
	int free_sect_nr = alloc_smap_bit(dir_inode->i_dev,
					  NR_DEFAULT_FILE_SECTS);
	struct inode *newino;
	if (flags == ISDIR){
		newino = new_inode(dir_inode->i_dev, inode_nr, free_sect_nr, I_DIRECTORY);
		new_dir_entry(dir_inode, newino->i_num, filename,'d');
	}
	else{
		newino = new_inode(dir_inode->i_dev, inode_nr, free_sect_nr, I_REGULAR);
		new_dir_entry(dir_inode, newino->i_num, filename,'f');
	}

	return newino;
}

/*****************************************************************************
 *                                do_close
 *****************************************************************************/
/**
 * 处理CLOSE消息
 * 
 * @return Zero if success.
 *****************************************************************************/
PUBLIC int do_close()
{
	int fd = fs_msg.FD;
	put_inode(pcaller->filp[fd]->fd_inode);
	if (--pcaller->filp[fd]->fd_cnt == 0)
		pcaller->filp[fd]->fd_inode = 0;
	pcaller->filp[fd] = 0;

	return 0;
}

/*****************************************************************************
 *                                do_mkdir
 *****************************************************************************/
/**
 * 处理MKDIR消息.
 * 
 * @return the result
 *****************************************************************************/
PUBLIC int do_mkdir()
{
	char pathName[MAX_PATH];

	// 取得message中的信息，详见lib/ls.c
	int flages = fs_msg.FLAGS;
	int name_len = fs_msg.NAME_LEN;
	int source = fs_msg.source;
	assert(name_len < MAX_PATH);  // 路径名称长度不得超过最大长度

	phys_copy((void*)va2la(TASK_FS, pathName), (void*)va2la(source, fs_msg.PATHNAME), name_len);
    	pathName[name_len] = 0;

	struct inode* dir_inode = create_file(pathName, ISDIR);
	if (dir_inode)
	{
		printl("creating directory %s succeeded!\n", pathName);
		put_inode(dir_inode);
		return 0;
	}
	else
	{
		printl("creating directory %s failed!\n", pathName);
		return -1;
	}
}

/*****************************************************************************
 *                                do_lseek
 *****************************************************************************/
/**
 * 处理LSEEK消息.
 * 
 * @return The new offset in bytes from the beginning of the file if successful,
 *         otherwise a negative number.
 *****************************************************************************/
PUBLIC int do_lseek()
{
	int fd = fs_msg.FD;
	int off = fs_msg.OFFSET;
	int whence = fs_msg.WHENCE;

	int pos = pcaller->filp[fd]->fd_pos;
	int f_size = pcaller->filp[fd]->fd_inode->i_size;

	switch (whence) {
	case SEEK_SET:
		pos = off;
		break;
	case SEEK_CUR:
		pos += off;
		break;
	case SEEK_END:
		pos = f_size + off;
		break;
	default:
		return -1;
		break;
	}
	if ((pos > f_size) || (pos < 0)) {
		return -1;
	}
	pcaller->filp[fd]->fd_pos = pos;
	return pos;
}

/*****************************************************************************
 *                                alloc_imap_bit
 *****************************************************************************/
/**
 * 在inode-map中分配一位，这也意味着新文件的i-node有了确定的位置
 * 
 * @param dev  In which device the inode-map is located.
 * 
 * @return  I-node nr.
 *****************************************************************************/
PRIVATE int alloc_imap_bit(int dev)
{
	int inode_nr = 0;
	int i, j, k;

	int imap_blk0_nr = 1 + 1; /* boot sector和super block */
	struct super_block * sb = get_super_block(dev);

	for (i = 0; i < sb->nr_imap_sects; i++) {
		RD_SECT(dev, imap_blk0_nr + i);

		for (j = 0; j < SECTOR_SIZE; j++) {
			/* skip `11111111' bytes */
			if (fsbuf[j] == 0xFF)
				continue;
			/* skip `1' bits */
			for (k = 0; ((fsbuf[j] >> k) & 1) != 0; k++) {}
			/* i: sector 号; j: byte 号; k: bit 号 */
			inode_nr = (i * SECTOR_SIZE + j) * 8 + k;
			fsbuf[j] |= (1 << k);
			/* write the bit to imap */
			WR_SECT(dev, imap_blk0_nr + i);
			break;
		}

		return inode_nr;
	}

	/*inode-map没有空闲的地方 */
	panic("inode-map is probably full.\n");

	return 0;
}

/*****************************************************************************
 *                                alloc_smap_bit
 *****************************************************************************/
/**
 * 在sector-map中分配多位，这也意味着为文件内容分配了扇区。
 * 
 * @param dev  In which device the sector-map is located.
 * @param nr_sects_to_alloc  How many sectors are allocated.
 * 
 * @return  The 1st sector nr allocated.
 *****************************************************************************/
PRIVATE int alloc_smap_bit(int dev, int nr_sects_to_alloc)
{
	/* int nr_sects_to_alloc = NR_DEFAULT_FILE_SECTS; */

	int i; /* sector index */
	int j; /* byte index */
	int k; /* bit index */

	struct super_block * sb = get_super_block(dev);

	int smap_blk0_nr = 1 + 1 + sb->nr_imap_sects;
	int free_sect_nr = 0;

	for (i = 0; i < sb->nr_smap_sects; i++) { /* smap_blk0_nr + i :
						     当前sector号. */
		RD_SECT(dev, smap_blk0_nr + i);

		/* byte offset in current sect */
		for (j = 0; j < SECTOR_SIZE && nr_sects_to_alloc > 0; j++) {
			k = 0;
			if (!free_sect_nr) {
				/* 重复直到找到一个空闲bit */
				if (fsbuf[j] == 0xFF) continue;
				for (; ((fsbuf[j] >> k) & 1) != 0; k++) {}
				free_sect_nr = (i * SECTOR_SIZE + j) * 8 +
					k - 1 + sb->n_1st_sect;
			}

			for (; k < 8; k++) { /* 重复直到所需bit被写完 */
				assert(((fsbuf[j] >> k) & 1) == 0);
				fsbuf[j] |= (1 << k);
				if (--nr_sects_to_alloc == 0)
					break;
			}
		}

		if (free_sect_nr) /* 找到了空闲的bit位，写进去 */
			WR_SECT(dev, smap_blk0_nr + i);

		if (nr_sects_to_alloc == 0)
			break;
	}

	//assert(nr_sects_to_alloc == 0);

	return free_sect_nr;
}

/*****************************************************************************
 *                                new_inode
 *****************************************************************************/
/**
 * 生成一个新的inode并写入硬盘
 * 
 * @param dev  Home device of the i-node.
 * @param inode_nr  I-node nr.
 * @param start_sect  Start sector of the file pointed by the new i-node.
 * 
 * @return  Ptr of the new i-node.
 *****************************************************************************/
PRIVATE struct inode * new_inode(int dev, int inode_nr, int start_sect, int imode)
{
	struct inode * new_inode = get_inode(dev, inode_nr);

	new_inode->i_mode = imode;
	new_inode->i_size = 0;
	new_inode->i_start_sect = start_sect;
	new_inode->i_nr_sects = NR_DEFAULT_FILE_SECTS;

	new_inode->i_dev = dev;
	new_inode->i_cnt = 1;
	new_inode->i_num = inode_nr;
	
	new_inode->i_node_length = 0;
    	new_inode->i_sects_pos[0] = start_sect;

	/* 将它写入硬盘中的inode-array区 */
	sync_inode(new_inode);

	return new_inode;
}

/*****************************************************************************
 *                                new_dir_entry
 *****************************************************************************/
/**
 *在根目录中创建一个目录项
 * 
 * @param dir_inode  I-node of the directory.
 * @param inode_nr   I-node nr of the new file.
 * @param filename   Filename of the new file.
 *****************************************************************************/
PRIVATE void new_dir_entry(struct inode *dir_inode,int inode_nr,char *filename,char type)
{
	/* 构造一个目录项 */
	int dir_blk0_nr = dir_inode->i_start_sect;
	int nr_dir_blks = (dir_inode->i_size + SECTOR_SIZE) / SECTOR_SIZE;
	int nr_dir_entries =
		dir_inode->i_size / DIR_ENTRY_SIZE; /**
						     * including unused slots
						     * (the file has been
						     * deleted but the slot
						     * is still there)
						     */
	int m = 0;
	struct dir_entry * pde;
	struct dir_entry * new_de = 0;

	int i, j;
	for (i = 0; i < nr_dir_blks; i++) {
		RD_SECT(dir_inode->i_dev, dir_blk0_nr + i);

		pde = (struct dir_entry *)fsbuf;
		for (j = 0; j < SECTOR_SIZE / DIR_ENTRY_SIZE; j++,pde++) {
			if (++m > nr_dir_entries)
				break;

			if (pde->inode_nr == 0) { /* 是一个空的slot */
				new_de = pde;
				break;
			}
		}
		if (m > nr_dir_entries ||/* 找完了所有项 */
		    new_de)              /* 找到了空闲的 */
			break;
	}
	if (!new_de) { /* 到达目录底部 */
		new_de = pde;
		dir_inode->i_size += DIR_ENTRY_SIZE;
	}
	new_de->inode_nr = inode_nr;
	new_de->type = type;
	strcpy(new_de->name, filename);

	/* 写目录项 */
	WR_SECT(dir_inode->i_dev, dir_blk0_nr + i);

	/* 更新inode结点 */
	sync_inode(dir_inode);
}

/*****************************************************************************
 *                               do_ls
 *****************************************************************************/
PUBLIC int do_ls()
{
	
    char pathname[MAX_PATH];

    /* get parameters from the message */
    int flags = fs_msg.FLAGS;   /* access mode */
    int name_len = fs_msg.NAME_LEN; /* length of filename */
    int src = fs_msg.source;    /* caller proc nr. */
    assert(name_len < MAX_PATH);

    phys_copy((void*)va2la(TASK_FS, pathname),
          (void*)va2la(src, fs_msg.PATHNAME),
          name_len);
    pathname[name_len] = 0;

    int i, j;

    /*printl("DO something \n");*/
    /*char pathname[MAX_PATH] = "passwd";*/
    /*int inode_nr = search_file(pathname);*/

    //struct inode * dir_inode = root_inode;
    struct inode * dir_inode;
    char filename[20];
    strip_path(filename, pathname,&dir_inode);

    int dir_blk0_nr = dir_inode->i_start_sect;
    int nr_dir_blks = (dir_inode->i_size + SECTOR_SIZE - 1) / SECTOR_SIZE;
    int nr_dir_entries = dir_inode->i_size / DIR_ENTRY_SIZE;
    int m = 0;

    struct dir_entry * pde;

    printl("\ninode        filename\n");
    printl("============================\n");

    for (i = 0; i < nr_dir_blks; i++)
    {
        RD_SECT(dir_inode->i_dev, dir_blk0_nr + i);

        pde = (struct dir_entry *)fsbuf;

        for (j = 0; j < SECTOR_SIZE / DIR_ENTRY_SIZE; j++, pde++)
        {
            /*struct inode *n = find_inode(pde->inode_nr);*/
            printl("  %2d        %s\n", pde->inode_nr , pde->name);
            if (++m >= nr_dir_entries){
                printl("\n");
                break;
            }
        }
        if (m > nr_dir_entries) //[> all entries have been iterated <]
            break;
    }

    printl("============================\n");

    return 0;
}
