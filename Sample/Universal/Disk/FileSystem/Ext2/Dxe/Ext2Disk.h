/*++

Copyright (c) 2012, kontais                                                                   
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  Ext2Disk.h

Abstract:

  https://ext4.wiki.kernel.org/index.php/Ext4_Disk_Layout
  http://www.nongnu.org/ext2-doc/ext2.pdf

--*/

#ifndef _EXT2_DISK_H_
#define _EXT2_DISK_H_

/*
 * The first boot and super blocks are given in absolute disk addresses.
 * The byte-offset forms are preferred, as they don't imply a sector size.
 */
#define BBSIZE    1024
#define SBSIZE    1024

/*
 * Structure of a blocks group descriptor
 */
typedef struct ext2_group_desc
{
  UINT32  bg_block_bitmap;        /* Blocks bitmap block */
  UINT32  bg_inode_bitmap;        /* Inodes bitmap block */
  UINT32  bg_inode_table;         /* Inodes table block */
  UINT16  bg_free_blocks_count;   /* Free blocks count */
  UINT16  bg_free_inodes_count;   /* Free inodes count */
  UINT16  bg_used_dirs_count;     /* Directories count */
  UINT16  bg_pad;
  UINT32  bg_reserved[3];
} EXT2_GRP_DESC;


/*
 * Constants relative to the data blocks
 */
#define EXT2_NDIR_BLOCKS                12
#define EXT2_IND_BLOCK                  EXT2_NDIR_BLOCKS
#define EXT2_DIND_BLOCK                 (EXT2_IND_BLOCK + 1)
#define EXT2_TIND_BLOCK                 (EXT2_DIND_BLOCK + 1)
#define EXT2_N_BLOCKS                   (EXT2_TIND_BLOCK + 1)


/*
 * Structure of an inode on the disk
 */
typedef struct _EXT2_INODE {
  UINT16  i_mode;         /* 0: File mode */
  UINT16  i_uid;          /* 2: Low 16 bits of Owner Uid */
  UINT32  i_size;         /* 4: Size in bytes */
  UINT32  i_atime;        /* 8: Access time */
  UINT32  i_ctime;        /* 12: Creation time */
  UINT32  i_mtime;        /* 16: Modification time */
  UINT32  i_dtime;        /* 20: Deletion Time */
  UINT16  i_gid;          /* 24: Low 16 bits of Group Id */
  UINT16  i_links_count;  /* 26: Links count */
  UINT32  i_blocks;       /* 28: Blocks count */
  UINT32  i_flags;        /* 32: File flags */
  union {
    struct {
      UINT32  l_i_reserved1;
    } linux1;
    struct {
      UINT32  h_i_translator;
    } hurd1;
    struct {
      UINT32  m_i_reserved1;
    } masix1;
  } osd1;                         /* 36: OS dependent 1 */
  UINT32  i_block[EXT2_N_BLOCKS];/* 40: Pointers to blocks */
  UINT32  i_generation;   /* 100: File version (for NFS) */
  UINT32  i_file_acl;     /* 104: File ACL */
  UINT32  i_dir_acl;      /* 108: Directory ACL */
  UINT32  i_faddr;        /* 112: Fragment address */
  union {
    struct {
      UINT8   l_i_frag;       /* 116: Fragment number */
      UINT8   l_i_fsize;      /* 117: Fragment size */
      UINT16  i_pad1;
      UINT16  l_i_uid_high;   /* 120: these 2 fields    */
      UINT16  l_i_gid_high;   /* 122: were reserved2[0] */
      UINT32  l_i_reserved2;
    } linux2;
    struct {
      UINT8   h_i_frag;       /* Fragment number */
      UINT8   h_i_fsize;      /* Fragment size */
      UINT16  h_i_mode_high;
      UINT16  h_i_uid_high;
      UINT16  h_i_gid_high;
      UINT32  h_i_author;
    } hurd2;
    struct {
      UINT8   m_i_frag;       /* Fragment number */
      UINT8   m_i_fsize;      /* Fragment size */
      UINT16  m_pad1;
      UINT32  m_i_reserved2[2];
    } masix2;
  } osd2;                         /* OS dependent 2 */
} EXT2_INODE;


/*
 * Structure of the super block
 */
typedef struct _EXT2_SUPER_BLOCK {
  UINT32  s_inodes_count;         /* Inodes count */
  UINT32  s_blocks_count;         /* Blocks count */
  UINT32  s_r_blocks_count;       /* Reserved blocks count */
  UINT32  s_free_blocks_count;    /* Free blocks count */
  UINT32  s_free_inodes_count;    /* Free inodes count */
  UINT32  s_first_data_block;     /* First Data Block */
  UINT32  s_log_block_size;       /* Block size */
  UINT32  s_log_frag_size;        /* Fragment size */
  UINT32  s_blocks_per_group;     /* # Blocks per group */
  UINT32  s_frags_per_group;      /* # Fragments per group */
  UINT32  s_inodes_per_group;     /* # Inodes per group */
  UINT32  s_mtime;                /* Mount time */
  UINT32  s_wtime;                /* Write time */
  UINT16  s_mnt_count;            /* Mount count */
  UINT16  s_max_mnt_count;        /* Maximal mount count */
  UINT16  s_magic;                /* Magic signature */
  UINT16  s_state;                /* File system state */
  UINT16  s_errors;               /* Behaviour when detecting errors */
  UINT16  s_minor_rev_level;      /* minor revision level */
  UINT32  s_lastcheck;            /* time of last check */
  UINT32  s_checkinterval;        /* max. time between checks */
  UINT32  s_creator_os;           /* OS */
  UINT32  s_rev_level;            /* Revision level */
  UINT16  s_def_resuid;           /* Default uid for reserved blocks */
  UINT16  s_def_resgid;           /* Default gid for reserved blocks */
  /*
   * These fields are for EXT2_DYNAMIC_REV superblocks only.
   *
   * Note: the difference between the compatible feature set and
   * the incompatible feature set is that if there is a bit set
   * in the incompatible feature set that the kernel doesn't
   * know about, it should refuse to mount the filesystem.
   * 
   * e2fsck's requirements are more strict; if it doesn't know
   * about a feature in either the compatible or incompatible
   * feature set, it must abort and not try to meddle with
   * things it doesn't understand...
   */
  UINT32  s_first_ino;            /* First non-reserved inode */
  UINT16  s_inode_size;           /* size of inode structure */
  UINT16  s_block_group_nr;       /* block group # of this superblock */
  UINT32  s_feature_compat;       /* compatible feature set */
  UINT32  s_feature_incompat;     /* incompatible feature set */
  UINT32  s_feature_ro_compat;    /* readonly-compatible feature set */
  UINT8   s_uuid[16];             /* 128-bit uuid for volume */
  CHAR8   s_volume_name[16];      /* volume name */
  CHAR8   s_last_mounted[64];     /* directory where last mounted */
  UINT32  s_algorithm_usage_bitmap; /* For compression */
  /*
   * Performance hints.  Directory preallocation should only
   * happen if the EXT2_COMPAT_PREALLOC flag is on.
   */
  UINT8   s_prealloc_blocks;      /* Nr of blocks to try to preallocate*/
  UINT8   s_prealloc_dir_blocks;  /* Nr to preallocate for dirs */
  UINT16  s_padding1;
  /*
   * Journaling support valid if EXT3_FEATURE_COMPAT_HAS_JOURNAL set.
   */
  UINT8   s_journal_uuid[16];     /* uuid of journal superblock */
  UINT32  s_journal_inum;         /* inode number of journal file */
  UINT32  s_journal_dev;          /* device number of journal file */
  UINT32  s_last_orphan;          /* start of list of inodes to delete */
  UINT32  s_hash_seed[4];         /* HTREE hash seed */
  UINT8   s_def_hash_version;     /* Default hash version to use */
  UINT8   s_reserved_char_pad;
  UINT16  s_reserved_word_pad;
  UINT32  s_default_mount_opts;
  UINT32  s_first_meta_bg;        /* First metablock block group */
  UINT32  s_reserved[190];        /* Padding to the end of the block */
} EXT2_SUPER_BLOCK;


/*
 * The second extended file system magic number
 */
#define E2FS_MAGIC    0xEF53

/*
 * Revision levels
 */
#define E2FS_REV0    0  /* The good old (original) format */
#define E2FS_REV1    1   /* V2 format w/ dynamic inode sizes */

#define E2FS_CURRENT_REV   E2FS_REV0
#define E2FS_MAX_SUPP_REV  E2FS_REV1

#define E2FS_REV0_INODE_SIZE 128

/*
 * compatible/incompatible features
 */
#define EXT2F_COMPAT_PREALLOC      0x0001
#define EXT2F_COMPAT_HASJOURNAL    0x0004
#define EXT2F_COMPAT_RESIZE        0x0010
#define EXT2F_COMPAT_DIRHASHINDEX  0x0020

#define EXT2F_ROCOMPAT_SPARSESUPER 0x0001
#define EXT2F_ROCOMPAT_LARGEFILE   0x0002
#define EXT2F_ROCOMPAT_BTREE_DIR   0x0004
#define EXT4F_ROCOMPAT_HUGE_FILE   0x0008
#define EXT4F_ROCOMPAT_GDT_CSUM    0x0010
#define EXT4F_ROCOMPAT_DIR_NLINK   0x0020
#define EXT4F_ROCOMPAT_EXTRA_ISIZE 0x0040

#define EXT2F_INCOMPAT_COMP        0x0001
#define EXT2F_INCOMPAT_FTYPE       0x0002
#define EXT4F_INCOMPAT_META_BG     0x0010
#define EXT4F_INCOMPAT_EXTENTS     0x0040
#define EXT4F_INCOMPAT_64BIT       0x0080
#define EXT4F_INCOMPAT_MMP         0x0100
#define EXT4F_INCOMPAT_FLEX_BG     0x0200

/*
 * Features supported in this implementation
 *
 */
#define EXT2F_COMPAT_SUPP      0x0000
#define EXT2F_ROCOMPAT_SUPP    (EXT2F_ROCOMPAT_SPARSESUPER | EXT2F_ROCOMPAT_LARGEFILE)
#define EXT2F_INCOMPAT_SUPP    EXT2F_INCOMPAT_FTYPE

/*
 * Structure of a directory entry
 */
#define EXT2_NAME_LEN 255

typedef struct ext2_dir_entry_2 {
  UINT32  inode;                  /* Inode number */
  UINT16  rec_len;                /* Directory entry length */
  UINT8   name_len;               /* Name length */
  UINT8   file_type;
  CHAR8   name[EXT2_NAME_LEN];    /* File name */
} EXT2_DIR_ENTRY;

/* Ext2 directory file types */
#define EXT2_FT_UNKNOWN         0
#define EXT2_FT_REG_FILE        1 
#define EXT2_FT_DIR             2
#define EXT2_FT_CHRDEV          3
#define EXT2_FT_BLKDEV          4
#define EXT2_FT_FIFO            5
#define EXT2_FT_SOCK            6
#define EXT2_FT_SYMLINK         7

#define EXT2_FT_MAX             8

#endif
