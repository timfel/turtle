/* sys/net.t.i -- C implementation for net.t.              -*-c-*-  */

/* This file contains the implementation for the various functions in
   the `sys.net' module.

   For details on how to hand-code Turtle modules, see the Turtle
   reference manual.  */

#include <stdlib.h>

#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

/* Return a unsigned integer from the location pointed to by P, where
   it is stored in network byte order.  */
#define UINT32(p) ((unsigned char)*p +		\
                  ((unsigned char)*(p+1)<<8) +	\
                  ((unsigned char)*(p+2)<<16) +	\
                  ((unsigned char)*(p+3)<<24))
/* Return a unsigned short from the location pointed to by P, where it
   is stored in network byte order.  */
#define UINT16(p) ((unsigned char)*p +		\
                  ((unsigned char)*(p+1)<<8))

#define SYS_CALL(res, call)				\
do {							\
  res = call;						\
  sys_errno_errno_pI = TTL_INT_TO_VALUE (errno);	\
}  while (0)

/* Function socket: fun(int, int, int): int.  */
#define sys_net_socket_pF3pIpIpI_pI_implementation \
{							\
  int domain, type, protocol, fd;			\
							\
  domain = TTL_VALUE_TO_INT (env->locals[0]);		\
  type = TTL_VALUE_TO_INT (env->locals[1]);		\
  protocol = TTL_VALUE_TO_INT (env->locals[2]);		\
  SYS_CALL (fd, (socket (domain, type, protocol)));	\
  acc = TTL_INT_TO_VALUE (fd);				\
}

static int
sockaddr_to_sockaddr_in (ttl_value sockaddr, struct sockaddr_in * sa)
{
  ttl_binary_array b = TTL_VALUE_TO_OBJ
    (ttl_binary_array, TTL_VALUE_TO_OBJ (ttl_array, sockaddr)->data[2]);
  sa->sin_family = AF_INET;
  sa->sin_port =
    htons (TTL_VALUE_TO_INT (TTL_VALUE_TO_OBJ (ttl_array, sockaddr)->data[1]));
  sa->sin_addr.s_addr =
    (b->data[0] << 24) |
    (b->data[1] << 16) |
    (b->data[2] << 8) | 
    (b->data[3] << 0);
  sa->sin_addr.s_addr = htonl (sa->sin_addr.s_addr);
  return 0;
}


static int
sockaddr_in_to_sockaddr (struct sockaddr_in * sa, ttl_value sockaddr)
{
  ttl_binary_array b = TTL_VALUE_TO_OBJ
    (ttl_binary_array, TTL_VALUE_TO_OBJ (ttl_array, sockaddr)->data[2]);
  int port = ntohs (sa->sin_port);
  long addr = ntohl (sa->sin_addr.s_addr);
  sa->sin_family = AF_INET;
  sa->sin_port =
    htons (TTL_VALUE_TO_INT (TTL_VALUE_TO_OBJ (ttl_array, sockaddr)->data[1]));
  sa->sin_addr.s_addr = (b->data[0] << 24) |
    (b->data[1] << 16) | (b->data[2] << 8) | b->data[3];
  sa->sin_addr.s_addr = htonl (sa->sin_addr.s_addr);

  TTL_VALUE_TO_OBJ (ttl_array, sockaddr)->data[1] = TTL_INT_TO_VALUE (port);
  b->data[0] = (addr >> 24) & 0xff;
  b->data[1] = (addr >> 16) & 0xff;
  b->data[2] = (addr >>  8) & 0xff;
  b->data[3] = (addr >>  0) & 0xff;
  return 0;
}


/* Function bind: fun(int, sys.net.sockaddr): int.  */
#define sys_net_bind_pF2pIusockaddr_pI_implementation \
{								\
  struct sockaddr_in sa;					\
  int ret;							\
								\
  acc = env->locals[1];						\
  TTL_NULL_CHECK;						\
  acc = TTL_VALUE_TO_OBJ (ttl_array, acc)->data[2];		\
  TTL_NULL_CHECK;						\
  sockaddr_to_sockaddr_in (env->locals[1], &sa);		\
  SYS_CALL (ret, (bind (TTL_VALUE_TO_INT (env->locals[0]),	\
			&sa, sizeof (sa))));			\
  acc = TTL_INT_TO_VALUE (ret);					\
}

/* Function connect: fun(int, sys.net.sockaddr): int.  */
#define sys_net_connect_pF2pIusockaddr_pI_implementation \
{								\
  struct sockaddr_in sa;					\
  int ret;							\
								\
  acc = env->locals[1];						\
  TTL_NULL_CHECK;						\
  acc = TTL_VALUE_TO_OBJ (ttl_array, acc)->data[2];		\
  TTL_NULL_CHECK;						\
  sockaddr_to_sockaddr_in (env->locals[1], &sa);		\
  SYS_CALL (ret, (connect (TTL_VALUE_TO_INT (env->locals[0]),	\
			   &sa, sizeof (sa))));			\
  acc = TTL_INT_TO_VALUE (ret);					\
}

/* Function listen: fun(int, int): int.  */
#define sys_net_listen_pF2pIpI_pI_implementation \
{							\
  int sockfd, backlog, ret;				\
							\
  sockfd = TTL_VALUE_TO_INT (env->locals[0]);		\
  backlog = TTL_VALUE_TO_INT (env->locals[1]);		\
  SYS_CALL (ret, (listen (sockfd, backlog)));		\
  acc = TTL_INT_TO_VALUE (ret);				\
}

/* Function iaccept: fun(int, sys.net.sockaddr): int.  */
#define sys_net_iaccept_pF2pIusockaddr_pI_implementation \
{								\
  struct sockaddr_in sa;					\
  socklen_t sa_len;						\
  int sockfd = TTL_VALUE_TO_INT (env->locals[0]);		\
  int ret;							\
								\
  SYS_CALL (ret, (accept (sockfd, &sa, &sa_len)));		\
  if (ret >= 0)							\
    sockaddr_in_to_sockaddr (&sa, env->locals[1]);		\
  acc = TTL_INT_TO_VALUE (ret);					\
}

/* Function igethostbyname: fun(string, internal.binary.binary): bool.  */
#define sys_net_igethostbyname_pF2pSubinary_pB_implementation \
{									\
  struct hostent *hostent;						\
  unsigned long addr;							\
  char * name;								\
  ttl_binary_array b;							\
									\
  acc = env->locals[0];							\
  TTL_NULL_CHECK;							\
  name = ttl_malloc_c_string (env->locals[0]);				\
  b = TTL_VALUE_TO_OBJ (ttl_binary_array, env->locals[1]);		\
									\
  if((hostent = gethostbyname (name)) == NULL)				\
    acc = TTL_BOOL_TO_VALUE (0);					\
  else if (hostent->h_addrtype == AF_INET && hostent->h_length == 4)	\
    {									\
      addr = UINT32 (hostent->h_addr_list[0]);				\
      addr = ntohl (addr);						\
      b->data[0] = (addr >> 24) & 0xff;					\
      b->data[1] = (addr >> 16) & 0xff;					\
      b->data[2] = (addr >>  8) & 0xff;					\
      b->data[3] = (addr >>  0) & 0xff;					\
      acc = TTL_BOOL_TO_VALUE (1);					\
    }									\
  else									\
    acc = TTL_BOOL_TO_VALUE (0);					\
  free (name);								\
}

/* End of sys/net.t.i.  */
