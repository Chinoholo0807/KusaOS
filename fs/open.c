/*************************************************************************//**
 *****************************************************************************
 * @file   fs/open.c
 * The file contains:
 *   - do_open()
 *   - do_close()
 *   - do_lseek()
 *   - create_file()
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
PRIVATE struct inode * new_inode(int dev, int inode_nr, int start_sect, int imode);
PRIVATE void new_dir_entry(struct inode * dir_inode, int inode_nr, char * filename);

/*****************************************************************************
 *                                do_ls
 *****************************************************************************/
PUBLIC int do_ls(){
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

	//printl("%d\n",name_len);
	//printl("%s\n",pathname);

	int i,j;

	struct inode * dir_inode;
    	char filename[20];
    	strip_path(filename, pathname,&dir_inode);

	/********INSERT**************/

	int dir_blk0_nr = dir_inode->i_start_sect;
   	int nr_dir_blks = (dir_inode->i_size + SECTOR_SIZE - 1) / SECTOR_SIZE;
    	int nr_dir_entries = dir_inode->i_size / DIR_ENTRY_SIZE;
    	int m = 0;

	struct dir_entry * pde;
	int find = 0;
	for (i = 0; i < nr_dir_blks&&!find; i++){
        //printl("%d %d\n", dir_inode->i_dev,nr_dir_blks);
        RD_SECT(dir_inode->i_dev, dir_blk0_nr + i);

        pde = (struct dir_entry *)fsbuf;
		
        for (j = 0; j < SECTOR_SIZE / DIR_ENTRY_SIZE && !find; j++, pde++){
            	if(strlen(pde->name) == 0){
                    	//
            	}else{
			if(strcmp(filename,pde->name)==0){
				find = 1;
				struct inode* tmpinode = dir_inode;
				dir_inode=get_inode(dir_inode->i_dev,pde->inode_nr);
				//(tmpinode);
			}
           	}
        }
        if (m > nr_dir_entries) //[> all entries have been iterated <]
            break;
    }
	
	/*******INSERT****************/
	
	dir_blk0_nr = dir_inode->i_start_sect;
   	nr_dir_blks = (dir_inode->i_size + SECTOR_SIZE - 1) / SECTOR_SIZE;
    	nr_dir_entries = dir_inode->i_size / DIR_ENTRY_SIZE;
    	m = 0;

	for (i = 0; i < nr_dir_blks; i++){
        //printl("start_sect: %d\n", dir_blk0_nr);
        RD_SECT(dir_inode->i_dev, dir_blk0_nr + i);

        pde = (struct dir_entry *)fsbuf;
        for (j = 0; j < SECTOR_SIZE / DIR_ENTRY_SIZE; j++, pde++){
            /*struct inode *n = find_inode(pde->inode_nr);*/
            if(strlen(pde->name) == 0){
                    //
            }else{
		    //printl("%d",pde->inode_nr);
                    printl("%s", pde->name);
		    int l;
                    for(l=strlen(pde->name); l < 15; l++){
                        printl(" ");
                    }
                    if(m % 4 == 3) {
                        printl("\n");
                    }
                    if (++m >= nr_dir_entries){
                        break;
                    }
               }
        }
        if(m%4 != 0){
            printl("\n");
        }
        if (m > nr_dir_entries) //[> all entries have been iterated <]
            break;
    }

	return 0;
}

/*****************************************************************************
 *                                do_mkdir
 *****************************************************************************/
PUBLIC int do_mkdir(){
	char pathName[MAX_PATH];

	// ??????message????????????
	int flages = fs_msg.FLAGS;
	int name_len = fs_msg.NAME_LEN;
	int source = fs_msg.source;
	assert(name_len < MAX_PATH);  // ??????????????????????????????????????????

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

	return 0;
}

/*****************************************************************************
 *                                do_open
 *****************************************************************************/
/**
 * ?????????????????????????????????????????????????????????
 * 
 * @return File descriptor if successful, otherwise a negative error code.
 *****************************************************************************/
PUBLIC int do_open()
{
	int fd = -1;		/* ????????? */

	char pathname[MAX_PATH];

	/* ???msg??????????????? */
	int flags = fs_msg.FLAGS;	/* ?????? */
	int name_len = fs_msg.NAME_LEN;	/* ???????????? */
	int src = fs_msg.source;	/* ?????????????????????. */
	assert(name_len < MAX_PATH);
	phys_copy((void*)va2la(TASK_FS, pathname),
		  (void*)va2la(src, fs_msg.PATHNAME),
		  name_len);
	pathname[name_len] = 0;

	/* ???PROCESS::filp[]????????????????????? */
	int i;
	for (i = 0; i < NR_FILES; i++) {
		if (pcaller->filp[i] == 0) {
			fd = i;
			break;
		}
	}
	if ((fd < 0) || (fd >= NR_FILES))
		panic("filp[] is full (PID:%d)", proc2pid(pcaller));

	/* ???f_desc_table[]???????????????????????? */
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
		if (strip_path(filename, pathname, &dir_inode) != 0)
			return -1;
		pin = get_inode(dir_inode->i_dev, inode_nr);
	}

	if (pin) {
		/* ?????????????????????????????? */
		pcaller->filp[fd] = &f_desc_table[i];

		/*????????????????????????inode */
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

/********************************************************
 *                              do_is_dir
 * ????????????????????????1
 * ?????????????????????0
 * ?????????????????????-1
 * ****************************************************/
PUBLIC int do_is_dir(){
        char pathname[MAX_PATH];

        /* ???msg??????????????? */
        int name_len = fs_msg.NAME_LEN; /* ???????????? */
        int src = fs_msg.source;        /* ?????????????????????. */
        assert(name_len < MAX_PATH);
        phys_copy((void*)va2la(TASK_FS, pathname),
                  (void*)va2la(src, fs_msg.PATHNAME),
                  name_len);
        pathname[name_len] = 0;

        int inode_nr = search_file(pathname);

        char filename[MAX_PATH];
        struct inode * dir_inode;
        if (strip_path(filename, pathname, &dir_inode) != 0)
                return -1;
        struct inode * pin = 0;
        pin = get_inode(dir_inode->i_dev, inode_nr);
        if(pin==0){
		//put_inode(pin);
                return -1;
        }
        int imode = pin->i_mode & I_TYPE_MASK;
        if(imode == I_DIRECTORY){
		//put_inode(pin);
                return 1;
	}
        else {
		put_inode(pin);
		return 0;
	}	
}

/*****************************************************************************
 *                                create_file
 *****************************************************************************/
/**
 * ?????????????????????????????????inode?????????
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
		new_dir_entry(dir_inode, newino->i_num, filename);
	}
	else{
		newino = new_inode(dir_inode->i_dev, inode_nr, free_sect_nr, I_REGULAR);
		new_dir_entry(dir_inode, newino->i_num, filename);
	}

	return newino;
}

/*****************************************************************************
 *                                do_close
 *****************************************************************************/
/**
 * ??????CLOSE??????
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
 *                                do_lseek
 *****************************************************************************/
/**
 * ??????LSEEK??????.
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
 * ???inode-map?????????????????????????????????????????????i-node?????????????????????
 * 
 * @param dev  In which device the inode-map is located.
 * 
 * @return  I-node nr.
 *****************************************************************************/
PRIVATE int alloc_imap_bit(int dev)
{
	int inode_nr = 0;
	int i, j, k;

	int imap_blk0_nr = 1 + 1; /* boot sector???super block */
	struct super_block * sb = get_super_block(dev);

	for (i = 0; i < sb->nr_imap_sects; i++) {
		RD_SECT(dev, imap_blk0_nr + i);

		for (j = 0; j < SECTOR_SIZE; j++) {
			/* skip `11111111' bytes */
			if (fsbuf[j] == 0xFF)
				continue;
			/* skip `1' bits */
			for (k = 0; ((fsbuf[j] >> k) & 1) != 0; k++) {}
			/* i: sector ???; j: byte ???; k: bit ??? */
			inode_nr = (i * SECTOR_SIZE + j) * 8 + k;
			fsbuf[j] |= (1 << k);
			/* write the bit to imap */
			WR_SECT(dev, imap_blk0_nr + i);
			break;
		}

		return inode_nr;
	}

	/*inode-map????????????????????? */
	panic("inode-map is probably full.\n");

	return 0;
}

/*****************************************************************************
 *                                alloc_smap_bit
 *****************************************************************************/
/**
 * ???sector-map??????????????????????????????????????????????????????????????????
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
						     ??????sector???. */
		RD_SECT(dev, smap_blk0_nr + i);

		/* byte offset in current sect */
		for (j = 0; j < SECTOR_SIZE && nr_sects_to_alloc > 0; j++) {
			k = 0;
			if (!free_sect_nr) {
				/* ??????????????????????????????bit */
				if (fsbuf[j] != 0x00) continue;
				for (; ((fsbuf[j] >> k) & 1) != 0; k++) {}
				free_sect_nr = (i * SECTOR_SIZE + j) * 8 +
					k - 1 + sb->n_1st_sect;
			}
			fsbuf[j] = 0x00;
			for (; k < 8; k++) { /* ??????????????????bit????????? */
				assert(((fsbuf[j] >> k) & 1) == 0);
				fsbuf[j] |= (1 << k);
				if (--nr_sects_to_alloc == 0)
					break;
			}
		}

		if (free_sect_nr) /* ??????????????????bit??????????????? */
			WR_SECT(dev, smap_blk0_nr + i);

		if (nr_sects_to_alloc == 0)
			break;
	}

	assert(nr_sects_to_alloc == 0);

	return free_sect_nr;
}

/*****************************************************************************
 *                                new_inode
 *****************************************************************************/
/**
 * ??????????????????inode???????????????
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

	/* ????????????????????????inode-array??? */
	sync_inode(new_inode);

	return new_inode;
}

/*****************************************************************************
 *                                new_dir_entry
 *****************************************************************************/
/**
 *????????????????????????????????????
 * 
 * @param dir_inode  I-node of the directory.
 * @param inode_nr   I-node nr of the new file.
 * @param filename   Filename of the new file.
 *****************************************************************************/
PRIVATE void new_dir_entry(struct inode *dir_inode,int inode_nr,char *filename)
{
	/* ????????????????????? */
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

			if (pde->inode_nr == 0) { /* ???????????????slot */
				new_de = pde;
				break;
			}
		}
		if (m > nr_dir_entries ||/* ?????????????????? */
		    new_de)              /* ?????????????????? */
			break;
	}
	if (!new_de) { /* ?????????????????? */
		new_de = pde;
		dir_inode->i_size += DIR_ENTRY_SIZE;
	}
	new_de->inode_nr = inode_nr;
	strcpy(new_de->name, filename);

	/* ???????????? */
	WR_SECT(dir_inode->i_dev, dir_blk0_nr + i);

	/* ??????inode?????? */
	sync_inode(dir_inode);
}
