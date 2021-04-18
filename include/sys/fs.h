/*************************************************************************//**
 *****************************************************************************
 * @file   include/sys/fs.h
 * @brief  Header file for File System.
 *****************************************************************************
 *****************************************************************************/

#ifndef	_ORANGES_FS_H_
#define	_ORANGES_FS_H_

/**
 * @struct dev_drv_map fs.h "include/sys/fs.h"
 * @brief  The Device_nr.\ - Driver_nr.\ MAP.
 */
struct dev_drv_map {
	int driver_nr; /**< The proc nr.\ of the device driver. */
};

/**
 * @def   MAGIC_V1
 * @brief Magic number of FS v1.0
 */
#define	MAGIC_V1	0x111

/**
 * @struct super_block fs.h "include/fs.h"
 * @brief  The 2nd sector of the FS
 *
 * 超级块结构体
 * 如果有成员改动就需要修改SUPER_BLOCK_SIZE！！！！！
 */
struct super_block {
	u32	magic;		  /**< 魔数 */
	u32	nr_inodes;	  /**< 最多inode数目 */
	u32	nr_sects;	  /**< 扇区数目 */
	u32	nr_imap_sects;	  /**< inode-map占用的扇区数 */
	u32	nr_smap_sects;	  /**< sector-map占用的扇区数 */
	u32	n_1st_sect;	  /**< 第一个数据扇区的扇区号 */
	u32	nr_inode_sects;   /**< inode_array占用多少扇区 */
	u32	root_inode;       /**< 根目录区的i-node号是多少 */
	u32	inode_size;       /**< INODE大小 */
	u32	inode_isize_off;  /**< Offset of `struct inode::i_size' */
	u32	inode_start_off;  /**< Offset of `struct inode::i_start_sect' */
	u32	dir_ent_size;     /**< DIR_ENTRY_SIZE */
	u32	dir_ent_inode_off;/**< Offset of `struct dir_entry::inode_nr' */
	u32	dir_ent_fname_off;/**< Offset of `struct dir_entry::name' */

	/*
	 * 这个部分只存在于内存中
	 */
	int	sb_dev; 	/**< the super block's home device */
};

/**
 * @def   SUPER_BLOCK_SIZE
 * @brief The size of super block \b in \b the \b device.
 *
 * 超级块在设备里的大小，而不是内存中的大小
 * 因为有只存在于内存中的部分，所以实际结构体的size会大一点
 */
#define	SUPER_BLOCK_SIZE	56

/**
 * @struct inode
 * @brief  i-node
 *
 * start_sect和nr_sects定位文件在设备中的位置
 * size为占用的字节数
 * 如果size < (nr_sects * SECTOR_SIZE)那么剩下的用于后边再写内容
 * 如果成员改了，就需要修改INODE_SIZE！！！！
 */
struct inode {
	u32	i_mode;		/**< 区分文件类型 */
	u32	i_size;		/**< 文件大小 */
	u32	i_start_sect;	/**< 文件的起始扇区 */
	u32	i_nr_sects;	/**< 总扇区数 */
	u8	_unused[16];	/**< Stuff for alignment */

	/* 只存在于内存里的部分 */
	int	i_dev;
	int	i_cnt;		/**< 共享这个inode的进程数量  */
	int	i_num;		/**< inode编号.  */
};

/**
 * @def   INODE_SIZE
 * @brief The size of i-node stored \b in \b the \b device.
 *
 * inode在设备里的大小，而不是内存中的大小
 * 因为有只存在于内存中的部分，所以实际结构体的size会大一点
 */
#define	INODE_SIZE	32

/**
 * @def   MAX_FILENAME_LEN
 * @brief 文件名的最大长度
 * @see   dir_entry
 */
#define	MAX_FILENAME_LEN	12

/**
 * @struct dir_entry
 * @brief  目录项
 */
struct dir_entry {
	int	inode_nr;		/**< inode编号. */
	char	name[MAX_FILENAME_LEN];	/**< 文件名 */
};

/**
 * @def   DIR_ENTRY_SIZE
 * @brief 目录项在设备中的大小
 *
 * 和在内存中的大小一致
 */
#define	DIR_ENTRY_SIZE	sizeof(struct dir_entry)

/**
 * @struct file_desc
 * @brief  文件描述符
 */
struct file_desc {
	int		fd_mode;	/**< 读或写*/
	int		fd_pos;		/**< 当前读写位置. */
	int		fd_cnt;		/**< 多少进程共用 */
	struct inode*	fd_inode;	/**< 指向inode的指针 */
};


/**
 * Since all invocations of `rw_sector()' in FS look similar (most of the
 * params are the same), we use this macro to make code more readable.
 */
#define RD_SECT(dev,sect_nr) rw_sector(DEV_READ, \
				       dev,				\
				       (sect_nr) * SECTOR_SIZE,		\
				       SECTOR_SIZE, /* read one sector */ \
				       TASK_FS,				\
				       fsbuf);
#define WR_SECT(dev,sect_nr) rw_sector(DEV_WRITE, \
				       dev,				\
				       (sect_nr) * SECTOR_SIZE,		\
				       SECTOR_SIZE, /* write one sector */ \
				       TASK_FS,				\
				       fsbuf);

	
#endif /* _ORANGES_FS_H_ */
