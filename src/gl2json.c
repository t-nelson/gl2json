
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <unistd.h>
#include <limits.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <json-c/json.h>
#include "glconf.h"

#define eprintf(fmt, ...) fprintf(stderr, fmt, ##__VA_ARGS__)
#define min(a, b)         ((a) < (b) ? (a) : (b))

static const char* default_conf = GLCONF;

struct config
{
  key_t       shm_key;
  const char* config_file;
  int         json_flags;
  size_t      max_users;
};

void usage()
{
  eprintf(" usage: gl2json [OPTION]\n"
          " Read glftpd's shared mem and output it as JSON.\n"
          "   -h                        Print this help message.\n"
          "   -p                        Output human-readable JSON.\n"
          "   -r /path/to/glftpd.conf   Use alternate config file.\n"
        );
}

bool parse_args(int argc, char* argv[], struct config* conf)
{
  int   arg;
  bool  rc  = true;

  assert(conf);

  while (-1 != (arg = getopt(argc, argv, "hpr:")))
  {
    switch (arg)
    {
      case 'p':
        conf->json_flags |= JSON_C_TO_STRING_PRETTY;
        break;
      case 'r':
        conf->config_file = optarg;
        break;
      case 'h':
      case '?':
      default:
        usage();
        rc = false;
        break;
    }
  }

  return rc;
}

bool read_config(struct config* conf)
{
  FILE* fp;
  bool  rc = true;

  assert(conf);

  if (NULL != (fp = fopen(conf->config_file, "r")))
  {
    char*   line      = NULL;
    size_t  line_len  = 0;
    while (rc && -1 != getline(&line, &line_len, fp))
    {
      if ('#' != line[0])
      {
        char  key[32]   = {};
        char  val[512]  = {};
        if (2 == sscanf(line, "%s %[^\n]", key, val))
        {
          if (0 == strcmp(key, "ipc_key"))
          {
            bool  good_val  = false;
            char* endptr    = NULL;
            long ipc_key    = strtol(val, &endptr, 0);
            if (ipc_key > INT_MAX)
              eprintf("Specified ipc_key value(0x%0lx) from config(%s) out of range!\n", (unsigned long)ipc_key, conf->config_file);
            else if (ipc_key == 0 && endptr == val)
              eprintf("Failed to convert ipc_key value(%s) from config(%s) to a number.\n", val, conf->config_file);
            else
            {
              good_val      = true;
              conf->shm_key = (key_t)ipc_key;
            }
            rc &= good_val;
          }
          else if (0 == strcmp(key, "max_users"))
          {
            size_t max, exempt;
            if (2 == sscanf(val, "%zu %zu", &max, &exempt))
              conf->max_users = max + exempt;
            else
            {
              eprintf("Failed to convert max_users values(%s) from config(%s).\n", val, conf->config_file);
              rc = false;
            }
          }
        }
      }
    }
    free(line);
    fclose(fp);
  }
  else if (conf->config_file != default_conf)
  {
    rc = false;
    perror(conf->config_file);
  }

  return rc;
}

int main(int argc, char* argv[])
{
  int           rc    = 1;
  struct config conf  = {};
  
  conf.shm_key     = 0x0000DEAD;
  conf.config_file = default_conf;
  conf.json_flags  = 0;
  conf.max_users   = SIZE_MAX;

  if (parse_args(argc, argv, &conf))
  {
    if (read_config(&conf))
    {
      int shm_id;
      if (-1 != (shm_id = shmget(conf.shm_key, 0, 0)))
      {
        struct shmid_ds stat_buf = {};
        if (-1 != shmctl(shm_id, SHM_STAT, &stat_buf))
        {
          void* shm_ptr;
          if (((void*)-1) != (shm_ptr = shmat(shm_id, NULL, SHM_RDONLY)))
          {
            struct json_object* array_obj;
            if (NULL != (array_obj = json_object_new_array()))
            {
              size_t                i;
              size_t                num_users;
              const struct ONLINE*  users     = shm_ptr;
              
              num_users = min(conf.max_users, stat_buf.shm_segsz / sizeof(struct ONLINE));

              for (i = 0; i < num_users; i++)
              {
                const struct ONLINE* user = &users[i];
                if (user->username[0] != '\0')
                {
                  struct json_object* user_obj;
                  if (NULL != (user_obj = json_object_new_object()))
                  {
                    struct json_object* cur_obj;
                    struct json_object* timeval_obj;
                    int64_t             temp64;

                    cur_obj = json_object_new_string(user->tagline);
                    json_object_object_add(user_obj, "tagline",     cur_obj);
                    cur_obj = json_object_new_string(user->username);
                    json_object_object_add(user_obj, "username",    cur_obj);
                    cur_obj = json_object_new_string(user->status);
                    json_object_object_add(user_obj, "status",      cur_obj);
                    cur_obj = json_object_new_int(user->ssl_flag);
                    json_object_object_add(user_obj, "ssl_flag",    cur_obj);
                    cur_obj = json_object_new_string(user->host);
                    json_object_object_add(user_obj, "host",        cur_obj);
                    cur_obj = json_object_new_string(user->currentdir);
                    json_object_object_add(user_obj, "currentdir",  cur_obj);
                    cur_obj = json_object_new_int(user->groupid);
                    json_object_object_add(user_obj, "groupid",     cur_obj);
                    cur_obj = json_object_new_int(user->login_time);
                    json_object_object_add(user_obj, "login_time",  cur_obj);

                    timeval_obj = json_object_new_object();
                    cur_obj     = json_object_new_int(user->tstart.tv_sec);
                    json_object_object_add(timeval_obj, "tv_sec",   cur_obj);
                    cur_obj     = json_object_new_int(user->tstart.tv_usec);
                    json_object_object_add(timeval_obj, "tv_usec",  cur_obj);
                    json_object_object_add(user_obj, "tstart", timeval_obj);

                    timeval_obj = json_object_new_object();
                    cur_obj     = json_object_new_int(user->txfer.tv_sec);
                    json_object_object_add(timeval_obj, "tv_sec",   cur_obj);
                    cur_obj     = json_object_new_int(user->txfer.tv_usec);
                    json_object_object_add(timeval_obj, "tv_usec",  cur_obj);
                    json_object_object_add(user_obj, "txfer", timeval_obj);

                    temp64  = min(user->bytes_xfer, INT64_MAX);
                    cur_obj = json_object_new_int64(temp64);
                    json_object_object_add(user_obj, "bytes_xfer",  cur_obj);

                    temp64  = min(user->bytes_txfer, INT64_MAX);
                    cur_obj = json_object_new_int64(temp64);
                    json_object_object_add(user_obj, "bytes_txfer", cur_obj);

                    cur_obj = json_object_new_int(user->procid);
                    json_object_object_add(user_obj, "procid",      cur_obj);

                    json_object_array_add(array_obj, user_obj);
                  }
                }
              }
              printf("%s\n", json_object_to_json_string_ext(array_obj, conf.json_flags));
              json_object_put(array_obj);
              rc = 0;
            }
            shmdt(shm_ptr);
          }
          else
            perror("shmat");
        }
        if (stat_buf.shm_cpid == getpid())
          shmctl(shm_id, IPC_RMID, NULL);
      }
      else if (ENOENT == errno)
        eprintf("shmget: No shared memory segment for key(0x%08X)\n", conf.shm_key);
      else
        perror("shmget");
    }
  }

  return rc;
}

