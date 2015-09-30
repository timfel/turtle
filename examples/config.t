// config.t -- Configuration data for the Turtle web server.
//
// Copyright (C) 2003 Martin Grabmueller <mgrabmue@cs.tu-berlin.de>
// 
// This is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.

// Commentary:
//
//* Configuration data for the Turtle web server.

module config;

//* Default port to bind to.  May be overwritten with the
//* @code{--port} command line option.
//
public var server_port: int := 5454;
public var document_root: string := "/home/mgrabmue/public_html/turtle/";

public var wiki_root: string := "/home/mgrabmue/public_html/turtle_wiki/";
public var wiki_prefix: string := "/wiki/";
public var wiki_logo_prefix: string := "/";
public var wiki_page_size_limit: int := 4096;
public var wiki_page_count_limit: int := 1000;

public var game_prefix: string := "/game/";

public var fork_children: bool := true;

public var debug_mode: bool := false;

// End of config.t.
