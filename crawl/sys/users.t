// sys/users.t -- User data base handling.
//
// Copyright (C) 2003 Martin Grabmueller <mgrabmue@cs.tu-berlin.de>
// 
// This is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.
//  
// This software is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this package; see the file COPYING.  If not, write to the
// Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
// MA 02111-1307, USA.

// Commentary:
//
//* Module for accessing user information.

module sys.users;

//* @code{getuid} returns the real user ID of the calling process.
//* 
//* @code{geteuid} returns the effective user ID of the current
//* process.  The effective ID corresponds to the set ID bit on the
//* file being executed.
public fun getuid (): int;
//* ""
public fun geteuid (): int;


// ATTENTION: Do not change this data type, unless you really know
// what you do, and change the setter function in file users.t.i
// accordingly.
//
public datatype passwd = passwd (name: string, passwd: string,
                                 uid: int, gid: int, gecos: string,
				 dir: string, shell: string);


// ATTENTION: Do not change this data type, unless you really know
// what you do, and change the setter function in file users.t.i
// accordingly.
//
public datatype group = group (name: string, passwd: string, gid: int,
                               members: array of string);


//* Return the password file structure for the user with the given
//* user name or user id, respectively.  If the user name or id is
//* not valid on the system the program runs on, @code{null} is
//* returned.
//
public fun getpwnam (name: string): passwd;
//* ""
public fun getpwuid (uid: int): passwd;


//* Return the next entry from the password file, as a value of the
//* data type @code{sys.users.passwd}.  If called for the first
//* time, the first entry will be returned, then successive entries
//* until the end of the user data base is reached.  The end of the
//* file is indicated by returning @code{null}.
//
public fun getpwent (): passwd;


//* Reset the read pointer for the user data base so that the next
//* call to @code{getpwent} will return the first entry.
//
public fun setpwent ();


//* Close the password file.  Use this function when you are ready
//* with the user data base.
//
public fun endpwent ();


//* Return the group file structure for the group with the given
//* group name or group id, respectively.  If the group name or id is
//* not valid on the system the program runs on, @code{null} is
//* returned.
//
public fun getgrnam (name: string): group;
//* ""
public fun getgrgid (uid: int): group;


//* Return the next entry from the group file, as a value of the
//* data type @code{sys.users.group}.  If called for the first
//* time, the first entry will be returned, then successive entries
//* until the end of the group data base is reached.  The end of the
//* file is indicated by returning @code{null}.
//
public fun getgrent (): group;


//* Reset the read pointer for the group data base so that the next
//* call to @code{getgrent} will return the first entry.
//
public fun setgrent ();


//* Close the group file.  Use this function when you are ready
//* with the group data base.
//
public fun endgrent ();


//* Return a string containing the name of the user logged in on
//* the controlling terminal of the process, or an empty string
//* if this information cannot be determined.
//
public fun getlogin (): string = "ttl_getlogin";

// End of sys/users.t.
