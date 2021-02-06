#include <mysql.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/time.h>
#include <unistd.h>

#define SL(s) (s), strlen(s)
using namespace std;

static void fatal(MYSQL *mysql, const char *msg) {
  fprintf(stderr, "%s: %s\n", msg, mysql_error(mysql));
  exit(1);
}

/* Helper function to do the waiting for events on the socket. */
static int wait_for_mysql(MYSQL *mysql, int status) {
  struct pollfd pfd;
  int timeout, res;

  pfd.fd = mysql_get_socket(mysql);
  pfd.events = (status & MYSQL_WAIT_READ ? POLLIN : 0) |
               (status & MYSQL_WAIT_WRITE ? POLLOUT : 0) |
               (status & MYSQL_WAIT_EXCEPT ? POLLPRI : 0);
  if (status & MYSQL_WAIT_TIMEOUT)
    timeout = 1000 * mysql_get_timeout_value(mysql);
  else
    timeout = -1;
  res = poll(&pfd, 1, timeout);
  if (res == 0)
    return MYSQL_WAIT_TIMEOUT;
  else if (res < 0)
    return MYSQL_WAIT_TIMEOUT;
  else {
    int status = 0;
    if (pfd.revents & POLLIN)
      status |= MYSQL_WAIT_READ;
    if (pfd.revents & POLLOUT)
      status |= MYSQL_WAIT_WRITE;
    if (pfd.revents & POLLPRI)
      status |= MYSQL_WAIT_EXCEPT;
    return status;
  }
}

static void run_query_sync2(const char *host, const char *user,
                            const char *password, int max_mult) {
  MYSQL mysql;
  int count = 5000;
  char q[2048];
  string qs = "";
  int mq_count = 0;
  int status = 0;

  mysql_init(&mysql);

  if (mysql_real_connect(&mysql, host, user, password, "test", 0, NULL,
                         CLIENT_MULTI_STATEMENTS) == NULL) {
    fprintf(stderr, "%s\n", mysql_error(&mysql));
    mysql_close(&mysql);
    exit(1);
  }
  while (count--) {
    //printf("wait %d\n", count);

    if (mq_count < max_mult) {
      snprintf(q, sizeof(q), "INSERT INTO test (a) VALUES (%d);", count);
      qs += q;
      mq_count++;
      continue;
    } else {
      if (mysql_query(&mysql, qs.c_str())) {
        fprintf(stderr, "%s - %s\n", mysql_error(&mysql), qs.c_str());
        mysql_close(&mysql);
        exit(1);
      }

      do {
        /* did current statement return data? */
        MYSQL_RES *result = mysql_store_result(&mysql);
        if (result) {
          /* yes; process rows and free the result set */
          // process_result_set(&mysql, result);
          mysql_free_result(result);
        } else /* no result set or error */
        {
          if (mysql_field_count(&mysql) == 0) {
            // printf("%lld rows affected\n", mysql_affected_rows(&mysql));
          } else /* some error occurred */
          {
            printf("Could not retrieve result set\n");
            break;
          }
        }
        /* more results? -1 = no, >0 = error, 0 = yes (keep looping) */
        if ((status = mysql_next_result(&mysql)) > 0)
          printf("Could not execute statement\n");
      } while (status == 0);

      qs = "";
      mq_count = 0;
    }
  }

  mysql_close(&mysql);
}

static void run_query_sync(const char *host, const char *user,
                           const char *password) {
  MYSQL mysql;
  int count = 5000;
  char q[2048];

  mysql_init(&mysql);

  if (mysql_real_connect(&mysql, host, user, password, "test", 0, NULL, 0) ==
      NULL) {
    fprintf(stderr, "%s\n", mysql_error(&mysql));
    mysql_close(&mysql);
    exit(1);
  }
  while (count--) {
    //printf("wait %d\n", count);
    // snprintf(q, sizeof(q), "call test_proc()");
    snprintf(q, sizeof(q), "INSERT INTO test (a) VALUES (%d)", count);
    if (mysql_query(&mysql, q)) {
      fprintf(stderr, "%s\n", mysql_error(&mysql));
      mysql_close(&mysql);
      exit(1);
    }
  }

  mysql_close(&mysql);
}

static void run_query_async(const char *host, const char *user,
                            const char *password) {
  int err, status;
  MYSQL mysql, *ret;
  int count = 5000;
  char q[2048];
  int wait_count = 0;

  mysql_init(&mysql);
  mysql_options(&mysql, MYSQL_OPT_NONBLOCK, 0);

  status = mysql_real_connect_start(&ret, &mysql, host, user, password, "test",
                                    0, NULL, 0);
  while (status) {
    status = wait_for_mysql(&mysql, status);
    status = mysql_real_connect_cont(&ret, &mysql, status);
  }

  if (!ret)
    fatal(&mysql, "Failed to mysql_real_connect()");

  while (count--) {
    wait_count = 0;
    // snprintf(q, sizeof(q), "call test_proc()");
    snprintf(q, sizeof(q), "INSERT INTO test (a) VALUES (%d)", count);
    status = mysql_real_query_start(&err, &mysql, SL(q));
    while (status) {
      wait_count++;
      //printf("wait %d-%d\n", count, wait_count);
      status = wait_for_mysql(&mysql, status);
      status = mysql_real_query_cont(&err, &mysql, status);
    }
    if (err)
      fatal(&mysql, "mysql_real_query() returns error");
  }


  mysql_close(&mysql);
}

int main(int argc, char **argv) {
  if (argv[1][0] == '1') {
    printf("run_query_async\n");
    run_query_async("localhost", "root", NULL);
  } else if (argv[1][0] == '2') {
    printf("run_query_sync\n");
    run_query_sync("localhost", "root", NULL);
  } else if (argv[1][0] == '3') {
      int c = atoi(argv[2]);
    printf("run_query_sync2\n");
    run_query_sync2("localhost", "root", NULL, c);
  }
}

/*
DROP PROCEDURE IF EXISTS test_proc;
DELIMITER //
CREATE PROCEDURE test_proc()
BEGIN
    set @s := 0;
    select sleep(1) into @s;
END //
DELIMITER ;
*/