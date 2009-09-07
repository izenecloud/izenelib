#!/usr/bin/perl -s 

$|=1;

if( !defined($overall) || !defined($clean) || !defined($makeagain) || !defined($multithread)  )
{
   print "Usage: ./sdb_test_all.pl -overall=  -clean=  -makeagain=1  -multithread=0\n"; 
   print "eg: ./sdb_test_all.pl -overall=1  -clean=1  -makeagain=1  -multithread=1\n"; 
   exit(0);
}

$|=1;

if($makeagain)
{
    system("cd ../source");
    system("make clean");
    system("make");
    system("cd ../build");
}

system("./t_sdb -db hash -index sdb_hash.dat ../db/wordlist.txt");
#system("./t_sdb -db skiplist -index sdb_skiplist.dat  ../db/wordlist.txt");
system("./t_sdb -db btree -index sdb_btree.dat  ../db/wordlist.txt");
system("./t_sdb -db btree -index sdb_cc_btree.dat  ../db/wordlist.txt");

#system("./t_nsdb -index nsdb.dat -T 0");

system("./t_overflow_btree -index osdb.dat ../db/test2.txt");

system("./t_btree1 -T 0 -index btree1.dat ../db/wordlist.txt");

system("./t_IndexSDB ../db/wordlist.txt");


if($multithread)
{
	system("./t_mul_sdb 0");
}

if($clean)
{
	system("rm -rf *.dat");
}


