#ifndef RESTRICTMACRO_H_
#define RESTRICTMACRO_H_

#ifdef COBRA_RESTRICT

#define COBRA_RESTRICT_BREAK \
	  break; \
	  
#define COBRA_RESTRICT_EXCEED_N_BREAK( EXP, NUM ) \
	if (EXP > NUM) \
	break; \
    
#define COBRA_RESTRICT_EXCEED_N_RETURN_FALSE( EXP, NUM ) \
	if ( EXP > NUM ) \
	return false; \
	
#else
	  
#define COBRA_RESTRICT_BREAK  
#define COBRA_RESTRICT_N_BREAK( EXP, NUM )   
#define COBRA_RESTRICT_N_RETURN_FALSE( EXP, NUM )	 

#endif


#endif /*RESTRICTMACRO_H_*/
