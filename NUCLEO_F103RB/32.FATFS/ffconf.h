/* ffconf.h - FatFs Configuration File */

/* Function Configurations */

#define FF_FS_READONLY     0    /* 0: Read/Write, 1: Read-only */
#define FF_FS_MINIMIZE     0    /* 0-3: Minimization level */

/* Code Page Setting */
#define FF_CODE_PAGE       437  /* 437: US, 932: Japanese, 949: Korean, 936: Chinese */

/* Long File Name Support */
#define FF_USE_LFN         2    /* 0: Disable, 1: Static, 2: Dynamic, 3: Dynamic + malloc */
#define FF_MAX_LFN         255  /* Maximum LFN length */

/* File System Options */
#define FF_FS_RPATH        0    /* 0-2: Relative path support */
#define FF_VOLUMES         1    /* Number of volumes (logical drives) */
#define FF_STR_VOLUME_ID   0    /* 0: Number, 1: String */
#define FF_MULTI_PARTITION 0    /* 0: Single partition, 1: Multiple partitions */

/* System Configurations */
#define FF_MIN_SS          512  /* Minimum sector size */
#define FF_MAX_SS          512  /* Maximum sector size */
#define FF_USE_TRIM        0    /* 0: Disable, 1: Enable TRIM */

/* File Lock and Timeout */
#define FF_FS_LOCK         0    /* 0: Disable, 1-10: Enable file lock */
#define FF_FS_TIMEOUT      1000 /* Timeout period in milliseconds */

/* Re-entrancy */
#define FF_FS_REENTRANT    0    /* 0: Disable, 1: Enable re-entrancy */

/* Additional Functions */
#define FF_USE_FIND        0    /* Enable f_findfirst() and f_findnext() */
#define FF_USE_MKFS        1    /* Enable f_mkfs() - FORMAT function */
#define FF_USE_FASTSEEK    0    /* Enable fast seek feature */
#define FF_USE_EXPAND      0    /* Enable f_expand() */
#define FF_USE_CHMOD       0    /* Enable f_chmod() and f_utime() */
#define FF_USE_LABEL       0    /* Enable volume label functions */
#define FF_USE_FORWARD     0    /* Enable f_forward() */
#define FF_USE_STRFUNC     2    /* 0:Disable 1:Enable without LF-CRLF 2:Enable with LF-CRLF */

/* Important: Set FF_USE_MKFS to 1 if you want to format SD card from STM32 */