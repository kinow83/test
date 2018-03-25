/*
* @ file : db.h
*	brief : 
*/

#include <mysql.h>

/****************************************************************
*	               ������ ���̽� ������														*
****************************************************************/

/*
* @ information database
*/
#define AGENT_INFORMATION "information"
#define AGENT_INFORMATION_MASTER	AGENT_INFORMATION".member"
#define AGENT_INFORMATION_STATUS	AGENT_INFORMATION".status"
#define CREATE_INFORMATION "create database "AGENT_INFORMATION

#define CREATE_INFORMATION_MEMBER " ( \
`ip` varchar(15) NOT NULL default '', \
`name` varchar(24) NOT NULL default '', \
`local` varchar(24) default NULL, \
`admin_name` varchar(24) default NULL, \
`admin_email` varchar(24) default NULL, \
`admin_phone` varchar(24) default NULL, \
`pass` varchar(50) NOT NULL default '', \
PRIMARY KEY  (`ip`) \
) ENGINE=MyISAM"

#define CREATE_INFORMATION_STATUS " ( \
`id` varchar(9) NOT NULL default '', \
`ip` char(15) default NULL, \
`mirror_ip` char(15) default NULL, \
`aliase` varchar(32) default NULL, \
`net_zone` text default NULL, \
`cpu` tinyint(4) NOT NULL default '0', \
`mem` tinyint(4) NOT NULL default '0', \
`disk1_name` varchar(8) default NULL, \
`disk1` tinyint(4) default NULL, \
`disk2_name` varchar(8) default NULL, \
`disk2` tinyint(4) default NULL, \
`save_time` tinyint(4) NOT NULL default '1', \
`backup_time` tinyint(4) NOT NULL default '-1', \
`alive` tinyint(1) unsigned default NULL, \
`admin_name` varchar(32) default NULL, \
`admin_email` varchar(45) default NULL, \
`admin_phone` varchar(18) default NULL, \
`checks` int(2) NOT NULL default '0' \
) ENGINE=MyISAM"



/*
* @ statics database
*/
#define STATICS "statics"
#define STATICS_AGENT STATICS".agent"

#define DB_STATICS "create database "STATICS

#define CREATE_STATICS " ( \
`date` char(10) NOT NULL default '', \
`time` char(2) NOT NULL default '', \
`tbyte` int unsigned NOT NULL default '0', \
`gbyte` int unsigned NOT NULL default '0', \
`mbyte` int unsigned NOT NULL default '0', \
`kbyte` int unsigned NOT NULL default '0', \
`byte` int unsigned NOT NULL default '0' \
) ENGINE=MyISAM"


/*
* @ agent database
*/
#define AGENT "agent"
#define LIVE "live"

#define CREATE_AGENT "(\
`src_ip` char(15) NOT NULL default '', \
`src_port` char(5) NOT NULL default '', \
`dest_ip` char(15) NOT NULL default '',	\
`dest_port` char(5) NOT NULL default '', \
`url` varchar(128) default NULL, \
`date` char(10) NOT NULL default '', \
`time` char(8) NOT NULL default '' \
) ENGINE=MyISAM"
	
#define CREATE_LIVE "(\
`src_ip` char(15) NOT NULL default '', \
`src_port` char(5) NOT NULL default '', \
`dest_ip` char(15) NOT NULL default '',	\
`dest_port` char(5) NOT NULL default '', \
`url` varchar(128) default NULL, \
`size` char(32) default NULL, \
`date` char(10) NOT NULL default '', \
`time` char(8) NOT NULL default '' \
) ENGINE=MyISAM"
///////////////////////////////////////////////////////////////////////////////////////////////////////



// ���� ���� ���
#define isLeap(x)	(\
				(\
					( (x) % 4 == 0 ) && \
					( (x) % 100 != 0 ) \
				)|| \
				( (x) % 400 == 0 ) \
			)


// �������� �̻� ���� Ȯ��
int query_invalied(MYSQL con, int query_stat)
{
	if (query_stat != 0){
		printf("%s\n", mysql_error(&con));
		return -1;
	}
	return 0;
}



// ����Ÿ ���̽� �ʱ�ȭ
int db_initial(char *host, char *user, char *password)
{

	char query[1024];
	int query_stat;	
	MYSQL       *connection=NULL, conn;	
	
	// ��� ����
	mysql_init(&conn);
	
	connection = mysql_real_connect(&conn, host, user, password, NULL, 0, (char *)NULL, 0);
	if (connection == NULL){
			printf("%s\n", mysql_error(&conn));
			return -1;
	}
	
	// @ DATABASE	create information database	
		
	// information.status - ������Ʈ ���� ���� ������ ���̽�
	sprintf(query, "create table %s %s", AGENT_INFORMATION_STATUS, CREATE_INFORMATION_STATUS);	
	query_stat = mysql_real_query(connection, query, strlen(query));
	query_invalied(conn, query_stat);
		
	/*
	// information.master
	sprintf(query, "create table %s %s", AGENT_INFORMATION_MASTER ,CREATE_INFORMATION_MEMBER);	
	query_stat = mysql_real_query(connection, query, strlen(query));
	query_invalied(conn, query_stat);
	*/
	
	// create statics database - Ʈ���� ������ ���̽�
	sprintf(query, DB_STATICS);
	query_stat = mysql_real_query(connection, query, strlen(query));
	query_invalied(conn, query_stat);
		
	mysql_close(connection);		
	return 0;
}


/*
 * @ ������Ʈ���� �Ҵ��� �� ��ȣ�� ���Ѵ�.
 */

int get_agent_num(char *host, char *user, char *password)
{
	
	MYSQL       *connection=NULL, conn;		
	MYSQL_RES   *sql_result;
	MYSQL_ROW    sql_row;
	int num=0;
	char query[100];
	int query_stat;
	
	// ��� ���� //
	mysql_init(&conn);
	
	connection = mysql_real_connect(&conn, host, user, password, NULL, 0, (char *)NULL, 0);
	if (connection == NULL){
			printf("%s\n", mysql_error(&conn));
			return -1;
	}
	
	sprintf(query, "SELECT COUNT(*) FROM %s", AGENT_INFORMATION_STATUS);
	query_stat = mysql_real_query(connection, query, strlen(query));
	query_invalied(conn, query_stat);
		
	sql_result = mysql_store_result( connection );
	
	//mysql fetch
	sql_row = mysql_fetch_row( sql_result );
	
	// ���� �ϴ� ������Ʈ ���� �ٰŷ� ������Ʈ ��ȣ �Ҵ�
	num = atoi(sql_row[0]);	
	
	mysql_free_result( sql_result );
	mysql_close(connection);
		
	return num + 1;
}


/*
 * @ ������Ʈ ���� ���ӽ� ��� ������Ʈ ��ȣ ����ϰ� alive ���¸� 1�� ����� ����.
 * 	 alive�� ������Ʈ�� ���¸� ��Ÿ�� : 1 - ����
 *																	    0 - �ٿ�, �̻�..
 */
int connect_agent(int agent_num, char *host, char *user, char *password)
{
	
	MYSQL       *connection=NULL, conn;
	char query[100];
	int query_stat;
	
	// ��� ����
	mysql_init(&conn);
	
	connection = mysql_real_connect(&conn, host, user, password, NULL, 0, (char *)NULL, 0);
	if (connection == NULL){
			printf("%s\n", mysql_error(&conn));
			return -1;
	}
	
	//id:������Ʈ ��ȣ //alive:�ý��� ����
	sprintf(query, "INSERT INTO %s (id, save_time, backup_time, alive, checks) VALUES(%d, -1, 1, 1, 0)", AGENT_INFORMATION_STATUS, agent_num);
	puts(query);
	query_stat = mysql_real_query(connection, query, strlen(query));	
	query_invalied(conn, query_stat);
	
	mysql_close(connection);	
	return 0;
		
}
	
	
// ������Ʈ�� ���� ���ӽ� ������Ʈ�� �ʿ��� ������ ���̽��� ������ش�.
int	create_agent_db(int agent, int year, int mon, char *host, char *user, char *password)
{
	
	MYSQL       *connection=NULL, conn;
	char query[1024];
	int query_stat;
	int day=0;
	int hour=0;	
	u_char		month_max_day[ 13 ] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };	
	
	month_max_day[2] = isLeap( year ) ? 29 : 28;
		
	// ��� ����
	mysql_init(&conn);
	
	connection = mysql_real_connect(&conn, host, user, password, NULL, 0, (char *)NULL, 0);
	if (connection == NULL){
			printf("%s\n", mysql_error(&conn));
			return -1;
	}

	// ��ο� ��Ŷ�� ������ �ش� ������Ʈ ��� �����.(������Ʈ �α� ��� ����)
	sprintf(query, "create database %s%d" ,AGENT, agent);
	query_stat = mysql_real_query(connection, query, strlen(query));
	query_invalied(conn, query_stat);

	// ���̺� ����
	// �ش� AG ������Ʈ ��� 31��(�Ѵ�)�� ���̺��� ����� ���̺���� ������Ʈ �̸�00, 01, 02, 03 ....���� �̷������.
	for( day = 1;  day <= month_max_day[mon];  day++ )
	{
		for( hour = 0;  hour < 24;  hour++ ){
					
			sprintf(query, "create table %s%d.AG%02d%02d %s" , AGENT, agent, day, hour, CREATE_AGENT);			
			query_stat = mysql_real_query(connection, query, strlen(query));
			query_invalied(conn, query_stat);
			
		}
	}

	// live ��Ŷ�� ���� ���̺�
	sprintf(query, "create table %s%d.%s %s" , AGENT, agent, LIVE, CREATE_LIVE);
	query_stat = mysql_real_query(connection, query, strlen(query));
	query_invalied(conn, query_stat);

	// STATICS ����� �ش� ������Ʈ ���̺� ����
	sprintf(query, "create table %s%d %s" , STATICS_AGENT, agent, CREATE_STATICS);
	query_stat = mysql_real_query(connection, query, strlen(query));
	query_invalied(conn, query_stat);
	
	
	////////////////////////////////////////////////////////////////////
	// policy ���� �̵� - ��å ���� ��� ����
	////////////////////////////////////////////////////////////////////	
	query_stat = mysql_select_db( connection, "policy" );
	query_invalied(conn, query_stat);
	
	// POLICY ����� group ���̺� ����
	sprintf(query, "create table agent%d_group (netobj varchar(128) NOT NULL default '0', host varchar(15) NOT NULL default '0' ) ENGINE=MyISAM" , agent);
	query_stat = mysql_real_query(connection, query, strlen(query));
	query_invalied(conn, query_stat);
	
	// POLICY ����� netobj ���̺� ����
	sprintf(query, "create table agent%d_netobj (netobj varchar(128) NOT NULL) ENGINE=MyISAM" , agent);
	query_stat = mysql_real_query(connection, query, strlen(query));
	query_invalied(conn, query_stat);
	
	// POLICY ����� policy ���̺� ����
	sprintf(query, "create table agent%d_policy (netobj varchar(128) NOT NULL default '0', type tinyint(3) unsigned NOT NULL default '0', catagory tinyint(3) unsigned NOT NULL default '0', no bigint(20) unsigned NOT NULL default '0') ENGINE=MyISAM" , agent);
	query_stat = mysql_real_query(connection, query, strlen(query));
	query_invalied(conn, query_stat);
	
	mysql_close(connection);
	return 0;
}


// ������Ʈ �α� ��� ��� �� ������Ʈ �α׵��� ��� �������� (�ӵ� ����� ���� drop)
// ������ ��� �ٽ� �������ش� - (��, ������ empty�̹Ƿ� �ʱ�ȭ �ϴ� ����)
int	create_backup_after(char* agent, int year, int mon, char *host, char *user, char *password)
{
	MYSQL       *create_connection=NULL, create_conn;	
	char query[1024];
	int query_stat;
	int day=0;
	int hour=0;
	u_char		month_max_day[ 13 ] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	
	month_max_day[2] = isLeap( year ) ? 29 : 28;
	
	// ��� ����
	mysql_init(&create_conn);
	create_connection = mysql_real_connect(&create_conn, host, user, password, NULL, 0, (char *)NULL, 0);
	if (create_connection == NULL){
			printf("%s\n", mysql_error(&create_conn));
			return -1;
	}
	
	// ��ο� ��Ŷ�� ������ �ش� ������Ʈ ��� �����.
	sprintf(query, "create database %s" ,agent);
	query_stat = mysql_real_query(create_connection, query, strlen(query));
	query_invalied(create_conn, query_stat);


	// ���̺� ����
	
	// �ش� ������Ʈ ��� 31��(�Ѵ�)�� ���̺��� ����� ���̺���� ������Ʈ �̸�00, 01, 02, 03 ....���� �̷������.
	for( day = 1;  day <= month_max_day[mon];  day++ )
	{		
		for( hour = 0;  hour < 24;  hour++ )
		{						
			sprintf(query, "create table %s.AG%02d%02d %s" , agent, day, hour, CREATE_AGENT);			
			query_stat = mysql_real_query(create_connection, query, strlen(query));
			query_invalied(create_conn, query_stat);
		}
	}

	// live ��Ŷ�� ���� ���̺�
	sprintf(query, "create table %s.%s %s" , agent, LIVE, CREATE_AGENT);
	query_stat = mysql_real_query(create_connection, query, strlen(query));
	query_invalied(create_conn, query_stat);
	
	puts("[BACKUP] Re_create_database FINISH");	
	mysql_close(create_connection);
	
	return 0;
	
}

// ������Ʈ �α� ��� ���
int db_backup(char *host, char *user, char *password, u_short pre_year, u_short pre_mon, u_short now_year, u_short now_mon, char * dir)
{
		
	int query_stat;
	char agent_backup[255];	
	char query[255];
	char back_path[64];
	const char showdb[] = "show databases";
	int backup_year, backup_mon;
	
	MYSQL       *backup_connection=NULL, backup_conn;
	MYSQL_RES   *sql_result;
	MYSQL_ROW   sql_row;
	
	// ��� ����
	mysql_init(&backup_conn);	
	backup_connection = mysql_real_connect(&backup_conn, host, user, password, NULL, 0, (char *)NULL, 0);
	if (backup_connection == NULL){
			printf("%s\n", mysql_error(&backup_conn));
			return -1;
	}
	
	
	// show database
	query_stat = mysql_real_query( backup_connection, showdb, strlen(showdb) );
	query_invalied( backup_conn, query_stat );
	
	sql_result = mysql_store_result( backup_connection );
	
	//mysql fetch
	while( sql_row = mysql_fetch_row(sql_result) ){
		
		// ������Ʈ �α� ������ ���̽� : "agent"�� ����
		if(strstr(sql_row[0], AGENT))
		{
			// ��� ��ƾ: 2008�� 7���� ����� 2008�� 8�� 1�� ������ �̷�� ����.			
			// ����� ���丮(path)
			sprintf(back_path,"%s/%s", dir, sql_row[0]);
						
			if( access(back_path, 0) != 0){
				if(mkdir(back_path)){
					printf("[BACKUP]Don't Worry %s exist\n",back_path);
				}
			}
			
//1. ���:  @ mysqldump(backup) & Gzip
			sprintf(agent_backup, "mysqldump --opt --lock-all-tables %s -p\"%s\" | gzip > %s/%s-%d-%d.gz ", sql_row[0] , password, back_path, sql_row[0] , pre_year , pre_mon);			
			printf("[BACKUP]Doing..: %s\n",agent_backup);
						
			system(agent_backup);			
			
//2. ����: statics ��񿡼� �ش� ������Ʈ ���̺� ����
			sprintf(query, "drop database %s", sql_row[0]);			
			query_stat = mysql_real_query( backup_connection, query, strlen(query) );
			printf("[BACKUP] %s\n",query);			
			query_invalied( backup_conn, query_stat );
			
			
//3. �����: ��� ������ ���ο� �޿� ���� ����
			if (create_backup_after(sql_row[0], now_year, now_mon, host, user, password) !=0 )
			{
				printf("[DBMS] Re_create database %s", sql_row[0]);
			}
					
		}
	}
		
	mysql_free_result(sql_result);
	printf("[BACKUP]FINISH...\n");	
	mysql_close(backup_connection);
	
	return 0;	
}



// �ǽð����� ������Ʈ �ý��� ���¸� ü��
int check_alive(char *host, char *user, char *password)
{
	int query_stat;	
	char query[255];		
	char show_check[128];
	char debug[128];
	int i;	

	MYSQL_RES   *sql_result;
	MYSQL_ROW   sql_row;
	MYSQL       *alive_connection=NULL, alive_conn;
	
	// ��� ����
	mysql_init(&alive_conn);	
	alive_connection = mysql_real_connect(&alive_conn, host, user, password, NULL, 0, (char *)NULL, 0);
	if (alive_connection == NULL){
			printf("%s\n", mysql_error(&alive_conn));
			return -1;
	}
	
	// DB���� save_time, backup_time ��������
	sprintf(show_check,"SELECT id, save_time, backup_time FROM %s", AGENT_INFORMATION_STATUS);	
	query_stat = mysql_real_query( alive_connection, show_check, strlen(show_check) );
	query_invalied( alive_conn, query_stat );
	
	sql_result = mysql_store_result( alive_connection );	
	while( sql_row = mysql_fetch_row(sql_result))
	{
		
		// save_time, backup_time ���� ��ġ	-- ������Ʈ ��� �̻��� ���� -> alive = 0
		if( !strcmp(sql_row[1], sql_row[2]) )
		{			
			sprintf(query,"UPDATE %s SET alive=0 where id=%s", AGENT_INFORMATION_STATUS, sql_row[0]);			
			query_stat = mysql_real_query( alive_connection, query, strlen(query) );
			query_invalied( alive_conn, query_stat );
		}
		
		// save_time, backup_time	���� ����ġ	-- ������Ʈ ��� ���� -> alive = 1
		else
		{			
			sprintf(query,"UPDATE %s SET alive=1, backup_time=%s where id=%s", AGENT_INFORMATION_STATUS, sql_row[1], sql_row[0]);			
			query_stat = mysql_real_query( alive_connection, query, strlen(query) );			
			query_invalied( alive_conn, query_stat );
		}
	
	}
	mysql_free_result(sql_result);
	mysql_close(alive_connection);
	return 0;		
}


// information.status �� �ش� ������Ʈ ip ����
int insert_ip(int agent_num, char * agnet_ip, char *host, char *user, char *password)
{
	
	MYSQL       *ip_connection=NULL, ip_conn;
	int query_stat;	
	char query[128];
	
	// ��� ����
	mysql_init(&ip_conn);	
	ip_connection = mysql_real_connect(&ip_conn, host, user, password, NULL, 0, (char *)NULL, 0);
	if (ip_connection == NULL){
			printf("%s\n", mysql_error(&ip_conn));
			return -1;
	}
	
	
	sprintf(query,"UPDATE  %s SET ip='%s', save_time=-1, backup_time=1, alive=1, checks=0 where id=%d",AGENT_INFORMATION_STATUS, agnet_ip, agent_num);
	puts(query);	
	query_stat = mysql_real_query( ip_connection, query, strlen(query) );
	query_invalied( ip_conn, query_stat );
	mysql_close(ip_connection);
	
	return 0;	
}




// ������ �α� ����
int review(int agent_num, char *dir, u_short year, u_short mon, char *host, char *user, char *password)
{
	
	MYSQL       *review_connection=NULL, review_conn;
	int query_stat;	
	char query1[] = "CREATE DATABASE review";	// ���� �� �α� ���� : "review" ������ ���̽�.
	char query2[] = "DROP DATABASE review";
	const char review[] = "review";
	char gzip_file[64];	
	char ungzip_file[64];	
	char ungzip[64];
	char gzip[64];
	
	
	// �α� ���⸦ ���ϴ� ������Ʈ, ��, �� �� �ٰŷ� ��� ���� ���� ã��
	sprintf(gzip_file,      "%s/agent%d/agent%d-%d-%d.gz", dir, agent_num, agent_num, year, mon);
	sprintf(ungzip_file,    "%s/agent%d/agent%d-%d-%d",    dir, agent_num, agent_num, year, mon);
	sprintf(ungzip, "gzip -d %s/agent%d/agent%d-%d-%d.gz", dir, agent_num, agent_num, year, mon);
	sprintf(gzip,      "gzip %s/agent%d/agent%d-%d-%d",    dir, agent_num, agent_num, year, mon);
	
	
	// ��� ����
	mysql_init(&review_conn);
	
	review_connection = mysql_real_connect(&review_conn, host, user, password, NULL, 0, (char *)NULL, 0);
	if (review_connection == NULL){
			printf("%s\n", mysql_error(&review_conn));
			return -1;	
	}
	
	// ��� ��� ����
	query_stat = mysql_real_query( review_connection, query2, strlen(query2) );
	query_invalied( review_conn, query_stat );
		
	usleep(10);
	
	// ��� ��� ����
	query_stat = mysql_real_query( review_connection, query1, strlen(query1) );
	query_invalied( review_conn, query_stat );
		
	mysql_close(review_connection);
	
	// ���� �� �α� ���� ������ �����ϴ��� Ȯ��
	if( access(gzip_file, 0) != 0){
		printf("Can not open BACK UP FILE: %s\n", gzip);
		return -1;
	}
	
	// ���� review(���������� �α� �������) ������ �ִٸ�...	
	if( access(ungzip_file, 0) != 0)
	{
		//���� ����		
		puts(ungzip);
		system(ungzip);
	}
	
	// ����
	sprintf(query1, "mysql -uroot -p\"%s\" %s < %s", password, review, ungzip_file);
	system(query1);
	
	// ����
	system(gzip);
	
	puts("[REVIEW UPLOAD]");
	
	return 0;	
}

