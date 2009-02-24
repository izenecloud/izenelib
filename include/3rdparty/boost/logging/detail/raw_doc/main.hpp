namespace boost { namespace logging {

/** 
@page main_intro Boost Logging Library v2 : Introduction

- @ref main_motivation
- @ref main_common_usage
- @ref main_feeback 
- @ref page_changelog 

@section main_motivation Motivation

Applications today are becoming increasingly complex. Part of making them easier to develop/maintain is to do logging. 
Logging allows you to later see what happened in your application. It can be a great help when debugging and/or testing it. 
The great thing about logging is that you can use it on systems in production and/or in use - if an error occurs, 
by examining the log, you can get a picture of where the problem is.

Good logging is mandatory in support projects, you simply can't live without it.

Used properly, logging is a very powerful tool. Besides aiding debugging/ testing, it can also show you 
how your application is used (which modules, etc.), how time-consuming certain parts of your program are, 
how much bandwidth your application consumes, etc. - it's up to you how much information you log, and where.

<b>Features</b>

- a simple and clear separation of @ref namespace_concepts "concepts"
    - concepts are also easily separated into namespaces
- a very flexible interface
- you don't pay for what you don't use
- allows for internationalization (i18n) - can be used with Unicode characters
- fits a lot of @ref common_scenarios "scenarios": from @ref common_scenarios_6 "very simple" (dumping all to one log) 
  to @ref scenario::usage "very complex" (multiple logs, some enabled/some not, levels, etc).
- allows you to choose how you use logs in your code (by defining your own LOG_ macros, suiting your application)
- allows you to use Log levels (debug, error, fatal, etc). However this is an orthogonal concept - the library
  will work whether you use levels, categories or whatever , or not.
- efficient filtering of log messages - that is, if a log is turned off, the message is not processed at all
- thread-safe - the library allows you several degrees of thread-safety
- allows for formatters and destinations 
    - formatters format the message (like, prepending extra information - an index, the time, thread id, etc)
    - destinations specify where the message is to be written
    - formatters and destinations are orthogonal to the rest of the library - if you want you can use them, otherwise
      you can define your own writing mechanism
- easy manipulation of the logs (turning on/off, setting formatters, destinations, etc)
- allows you to use @ref tag "tags" (extra information about the context of the log: file/line, function name, thread id, etc.)
- allows for @ref scoped_logs "scoped logs"
- has @ref formatter::high_precision_time_t "high precision time formatter"
- easy to configure at run-time, if you wish
  - @ref formatter::named_spacer
  - @ref destination::named
- @ref caching "cache messages before logs are initialized"
- allows for @ref profile "profiling itself"


\n\n
@section main_common_usage Common Usage

To get you started, here's the <b>most common usage</b>:
\n

@copydoc mul_levels_one_logger 

@ref scenarios_code_mom "Click to see the code"
\n\n\n

To see more examples, check out @ref common_scenarios.


\n\n\n
@section main_feeback Feedback

I certainly welcome all feedback. So, be it a suggestion, or criticism, do write to me:
- contact me: http://torjo.com/contact.html
- if there's a feature you'd like, you can contact me, or drop a comment here:
  http://torjo.blogspot.com/2007/11/boost-logging-v2-your-killer-feature.html \n
  (this way, others can contribute with comments as well)


\n\n\n
@section last_v2 Last version of v2.

The Boost Logging Lib v2 was rejected on 17th of March 2008. I will be working on v3.
So, you can consider v0.23.2 as the last version of v2. From now on, 
- I will be doing only small fixes, if any major bugs, I'll fix them
- I will be working on v3. It might take a while until first version of v3 appears - I assume End of April
- to stay tuned on development, and to make sure your oppinion counts, please check out http://torjo.blogspot.com



\n\n\n
@section main_changelog Changelog

@ref page_changelog "See the changelog".



*/

}}
