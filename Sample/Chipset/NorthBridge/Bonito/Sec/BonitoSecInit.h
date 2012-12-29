#ifndef _BONITO_SEC_INIT_H_
#define _BONITO_SEC_INIT_H_

#define  BONITO_INIT(r,v)   REG_INIT_WRITE(REG_INIT_MOD_WORD, PHYS_TO_UNCACHED(r), v)
#define  BONITO_BIS(r,b)    REG_INIT_RMW(REG_INIT_MOD_WORD,   PHYS_TO_UNCACHED(r), ~0, b)
#define  BONITO_BIC(r,b)    REG_INIT_RMW(REG_INIT_MOD_WORD,   PHYS_TO_UNCACHED(r), ~(b), 0)
#define  BONITO_RMW(r,c,s)  REG_INIT_RMW(REG_INIT_MOD_WORD,   PHYS_TO_UNCACHED(r), ~(c), s)

#endif /* _BONITO_SEC_INIT_H_ */
