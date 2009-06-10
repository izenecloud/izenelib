/**
 * @mainpage ID Process
 *
 * IDManager controls many kinds of IDs: Document ID, term ID, or any other Types of ID
 * Which are requested from specific manager. 
 *
 * ID or Key data can be easily taken out from IDManager when the matched Key or ID is 
 * given. If new key is inserted into the IDManager, it generates new ID which doesn't 
 * have duplication in the vocabulary of IDManager. Different ID is managed in different 
 * vocabulary storage.
 * 
 * SuffixToTermIdMap is a class to  makes IDManager possible to offer wildcard service.
 * This manager uses suffix array as a term id dictionary.
 * Whenever ID Manager generates new id for the given string, it stores a list of string which
 * is given ID. If it reaches to the certain amount, ID Manager sends this list to the SuffixToTermIdMap class 
 * for building initial term id dictionary. ID Manager can get the term Id list as a result of wildcard
 * search by just send wildcard string to the module.
 *
 * SequentialDB is used for recovering the unexpected termination.
 *
 *
 * @htmlonly
 * <center><img src="IDManager_Class_Diagram.jpg" alt="Class Diagram of IDManager" border="0"><center>
 * @endhtmlonly
 */

