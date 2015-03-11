#ifndef HTTPXX_H
#define HTTPXX_H

#include <httpxx/params.h>
#include <httpxx/uri.h>
#include <httpxx/headers.h>
#include <httpxx/message_parser.h>
#include <httpxx/message_composer.h>

//! httpxx namespace all API belongs to
namespace httpxx
{

/*! \mainpage HTTPXX - Low level C++ HTTP library

  \section intro_section Introduction

  The main idea of this library is to provide maximum flexibility
  and not to anyhow affect user-side code: I/O, threading model,
  software design, etc. - all of this should be implemented by user
  not by HTTPXX. That is why HTTPXX is more about parsing/composing
  HTTP-messages then to provide a complete implementation of the
  HTTP-server out of the box.

  You can easily cover following use-cases by HTTPXX:

  - Asynchronous HTTP-messages exchange;
  - Use any device for I/O: serial ports, UNIX-sockets, SSL-layer,
    shared memory buffers, etc.;
  - Send/receive HTTP-message with infinite length using chunked
    transfer encoding (e.g. to broadcast a
    <a href="http://en.wikipedia.org/wiki/Pi">PI number</a> :) );
  - Stream audio/video data;
  - Add a web-frontend implementation to embedded software component;

  \section features_section Features

  - Simple to use;
  - Fully independent from I/O;
  - Fast and small;
  - Requires only C++99 and STL;
  - Cross-platform;
  - Custom HTTP-method/HTTP-version/URI-type/HTTP-status support;
  - Following HTML entities parsing/composition support:
    - HTTP-message - see MessageParser and MessageComposer;
    - URI - see Uri;
    - GET/POST parameters - see Params;
    - Cookies (TODO);
  - Headers-only library (TODO).

  \section installation_section Installation

  TODO

  \section usage_section Usage

  TODO

  \section example_section Example
  
  \verbinclude httpd_file_browser.cpp

  \section todo_section TODO

  - Memory-buffer stream implementation;
  - Cookies parser/composer;
  - HTTP-request/HTTP-response parsers/composers;
  - Headers-only library.
  
 */

}

#endif
