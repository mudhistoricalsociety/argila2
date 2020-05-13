/*------------------------------------------------------------------------\
|  decl.h : Program Declarations                      www.yourmud.org     | 
|  Copyright (C) 2006, Project Argila: Auroness                           |
|                                                                         |
|  All rights reserved. See Licensing documents included.                 |
|                                                                         |
|  Based upon Shadows of Isildur RPI Engine                               |
|  Copyright C. W. McHenry [Traithe], 2004                                |
|  Derived under license from DIKU GAMMA (0.0).                           |
\------------------------------------------------------------------------*/

extern int pclose ();
extern int gettimeofday ();
extern int getrlimit ();
extern int select ();

#ifndef LINUX
#if defined (_sys_socket_h)
extern int socket (int domain, int type, int protocol);
extern int setsockopt (int s, int level, int optname, char *optval, int optlen);
extern int bind (int s, struct sockaddr_in *name, int namelen);
extern int accept (int s, struct sockaddr_in *addr, int *addrlen);
extern int listen (int s, int backlog);
#endif
#endif

