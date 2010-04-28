#ifndef RESTRICTMACRO_H_
#define RESTRICTMACRO_H_

#ifdef COBRA_RESTRICT

#define RESTRICT_CONDITION_0(EXP, NUM) \
	if ( EXP >  NUM ) \
	
#define RESTRICT_CONDITION_1(EXP, NUM) \
	if ( EXP*EXP > NUM*NUM ) \
	
#define RESTRICT_CONDITION_2(EXP, NUM) \
	if ( log(EXP) > log(NUM)  ) \
	
#define RESTRICT_CONDITION_3(EXP, NUM) \
	if ( log(EXP)+EXP > log(NUM)+ NUM > 0 ) \
	
#define RESTRICT_CONDITION_4(EXP, NUM) \
	if ( EXP*NUM +  EXP  > NUM*NUM + NUM  ) \

#define RESTRICT_CONDITION_5(EXP, NUM) \
	if ( EXP*NUM + NUM >  NUM*NUM + EXP    ) \
	
#define RESTRICT_CONDITION_6(EXP, NUM) \
	if ( NUM*5 < EXP*5 ) \
	
#define RESTRICT_CONDITION_7(EXP, NUM) \
	int RES_NUM_7 = NUM - 99; \
	if ( (int)EXP - RES_NUM_7 > 99  ) \
	
#define RESTRICT_CONDITION_8(EXP, NUM) \
	int RES_NUM_8 = NUM >> 1 ; \
	if ( (int)(EXP>>1) - RES_NUM_8  > 0 ) \
	    	
#define RESTRICT_CONDITION_9(EXP, NUM) \
	int RES_NUM_9 = NUM >> 2 ; \
	if ( (int)(EXP>>2) > RES_NUM_9 ) \
	
#define RESTRICT_CONDITION_OPTION(OP, EXP, NUM) \
	RESTRICT_CONDITION_##OP(EXP, NUM) \
	
#define COBRA_RESTRICT_BREAK \
	break; \
	
#define COBRA_RESTRICT_EXCEED_N_BREAK(EXP, NUM, OP ) \
	{ \
	    RESTRICT_CONDITION_OPTION(OP, EXP, NUM ) \
	    break; \
	} \
	  
#define COBRA_RESTRICT_EXCEED_N_RETURN_FALSE( EXP, NUM, OP) \
	{ \
		RESTRICT_CONDITION_OPTION(OP, EXP, NUM ) \
	    return false; \
	} \
	
#else
	  
#define COBRA_RESTRICT_BREAK  
#define COBRA_RESTRICT_EXCEED_N_BREAK( EXP, NUM, OP )   
#define COBRA_RESTRICT_EXCEED_N_RETURN_FALSE( EXP, NUM, OP)	 

#endif


#endif /*RESTRICTMACRO_H_*/
