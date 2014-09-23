General purpose C++ library
=============================================
*A general C++ library containing access methods, information retrieval, and fundamental building blocks.*

### Features
* _Access methods_. `am` is used to encapsulate access methods for both persistent and non-persistent storage engines. Highlights: unified encapsulation interface for all access methods for popular key-valus stores including LevelDB, Tokyocabinet, TokuKV, LMDB, LuxIO; succinct data structures; external sort; fast compressed bitmap,...etc.

* _Information retrieval_. `ir` is used to encapsulate information retrieval libraries, including [Lucene](lucene.apache.org) like file based inverted index, and Zambezi which is state-of-the-art pure memory based inverted index. We also implemented dedicate [DNF](http://en.wikipedia.org/wiki/Disjunctive_normal_form) index to support boolean expression retrieval which is required by many advertising. 

* _Utilities_. `util` is used to encapsulate fundamental building blocks such as Singeleton, compression, serialization, SIMD,...,etc.


### Dependencies
We've just switched to `C++ 11` for SF1R recently, and `GCC 4.8` is required to build SF1R correspondingly. We do not recommend to use Ubuntu for project building due to the nested references among lots of libraries. CentOS / Redhat / Gentoo / CoreOS are preferred platform. You also need `CMake` and `Boost 1.56` to build the repository . Here are the dependent repositories list:

* __[cmake](https://github.com/izenecloud/cmake)__: The cmake modules required to build all iZENECloud C++ projects.


### License
The project is published under the Apache License, Version 2.0:
http://www.apache.org/licenses/LICENSE-2.0
