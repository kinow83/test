/*
   * @file  : config.h
   * @brief    : ÀÏ¹Ý ¼³Á¤
   */


#define MAX_AGENT 30       // ÃÖ´ë ¿¡ÀÌÀüÆ®
#define MASTER_PORT 9000   // ¿¡ÀÌÀüÆ®¿Í Åë½Å Æ÷Æ®

// DBMS ¼³Á¤ ÆÄÀÏ
#define LOCALHOST "localhost"
#define HOST         "127.0.0.1"
#define USER         "root"
#define PASSWORD ""

// ¹é¾÷½Ã°£Àº ¸Å¿ù Ã¹Â°´Þ 40ºÐ 10ÃÊ¿¡ ÀÌ·ç¾îÁø´Ù.
int backup_min =  40;
int backup_sec =  10;
