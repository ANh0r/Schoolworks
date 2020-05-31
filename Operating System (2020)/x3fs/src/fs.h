#ifndef _FS_H
#define _FS_H

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <time.h>

extern int fd;

#define report_error(msg, val...)                       \
    do                                                  \
    {                                                   \
        fprintf(stderr, "%s: " msg "\n", __FUNCTION__); \
        retval = (-1, ##val);                           \
        goto out;                                       \
    } while (0)

#define min(a, b) ((a) < (b) ? (a) : (b))

#define BLOCK_SIZE 4096
#define BLK_FREE 0
#define BLK_END 1

typedef uint16_t bid_t; // block id
typedef uint8_t blk_t[BLOCK_SIZE];

#define offset_of(x) (sizeof(blk_t) * x)

typedef struct superblock
{
    uint16_t magic;
#define MAGIC_SUPERBLOCK 0x1510u
    int total_size;
    int total_block_num;
    int free_block_num;
    int fat_block_num;
    int fcb_num_per_block;
    int data_start_bid;
    int fat_crc;
} sb_t;
#define sb_check_magic(x) (((sb_t *)x)->magic == MAGIC_SUPERBLOCK)

#define FNAME_LENGTH 9
typedef struct fcb
{
    char fname[FNAME_LENGTH]; // 9
    uint32_t size;            // 4
    bid_t bid;                // 2
    uint8_t attrs;            // 1
    time_t created_time;      // 4
    time_t modified_time;     // 4
} fcb_t;
#define check_filename_length(x) (strlen(x) <= FNAME_LENGTH)
#define DIR_MASK 0b10u
#define EXIST_MASK 0b1u
#define fcb_isdir(fcb) ((fcb)->attrs & DIR_MASK)
#define fcb_isfile(fcb) (!((fcb)->attrs & DIR_MASK))
#define fcb_exist(fcb) ((fcb)->attrs & EXIST_MASK)

typedef struct opened_file
{
    bool not_empty;
    fcb_t fcb;
    bid_t at_bid;
    int fcb_id;
    off_t off; // offset
    bool is_fcb_modified;
} of_t;
#define MAX_FD 0x0F
#define check_opened_fd(x) ((x) >= 0 && (x) <= MAX_FD)
extern of_t ofs[MAX_FD];

typedef struct directory
{
    uint16_t magic;
#define MAGIC_DIR 0xD151u
    int item_num;
    bid_t bid;
    bid_t parent_bid;
    fcb_t fcb[0];
} dir_t;
#define dir_check_magic(x) (((dir_t *)x)->magic == MAGIC_DIR)

extern sb_t *sb;
#define root_bid (1 + 2 * sb->fat_block_num)
extern bid_t *fat;
extern dir_t *cur_dir;

// init fs image
int fs_loadfrom(const char *filename);
int fs_writeto(const char *filename);

// operations
int fs_mkdir(const char *path);
int fs_rmdir(const char *path);
int fs_ls();
int fs_stat();
int fs_cd(const char *path);
int fs_create(const char *path);
int fs_open(const char *path);
int fs_close(int);
#ifndef SEEK_SET
#define SEEK_SET 0 /* set file offset to offset */
#endif
#ifndef SEEK_CUR
#define SEEK_CUR 1 /* set file offset to current plus offset */
#endif
#ifndef SEEK_END
#define SEEK_END 2 /* set file offset to EOF plus offset */
#endif
int fs_seek(int, int, int);
int fs_write(int, const char *, size_t);
int fs_read(int, const char *, size_t);
int fs_rm(const char *path);
int fs_rename(const char *old, const char *new);
int fs_exit();

// helper
bid_t find_free_block();
int find_available_fd();
char *format_size(uint32_t);
char *get_dirname(dir_t *);
char *get_abspath(dir_t *);

#endif
