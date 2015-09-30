// game.t -- Game functions for the Turtle web server.
//
// Copyright (C) 2003 Martin Grabmueller <mgrabmue@cs.tu-berlin.de>
// 
// This is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.

// Commentary:
//
//* Game functionality for the Turtle web server.

module game;

import io, html, http, config;

public fun handle_game_request (req: http.request): http.response
  var body: html.tag;

  body := html.page ("Game Index", 
    [html.ul ([html.s ("One"), html.s ("Two"), html.s ("Three")])]);

  return http.ok (http.version (req), http.std_html_headers (),
                  null, -1L, req, http.file (req), -1, body);
end;

// End of game.t.
