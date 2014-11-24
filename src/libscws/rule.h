/**
 * @file rule.h
 * @author Hightman Mar
 * @editor set number ; syntax on ; set autoindent ; set tabstop=4 (vim)
 * $Id$
 */

#ifndef	_SCWS_RULE_20070525_H_
#define	_SCWS_RULE_20070525_H_


#ifdef __cplusplus
extern "C" {
#endif

/* xtree required */
#include "xtree.h"
#include "../cjson/cJSON.h"

#define	SCWS_RULE_MAX			32
#define	SCWS_RULE_SPECIAL		0x80000000
#define	SCWS_RULE_NOSTATS		0x40000000

#define SCWS_RULE_JSON_STRING 0
#define SCWS_RULE_JSON_FILE   1

/* flag: 0x00 ~ 0x4000 */
#define	SCWS_ZRULE_NONE			0x00
#define	SCWS_ZRULE_PREFIX		0x01
#define	SCWS_ZRULE_SUFFIX		0x02
#define	SCWS_ZRULE_INCLUDE		0x04	/* with include */
#define	SCWS_ZRULE_EXCLUDE		0x08	/* with exclude */
#define	SCWS_ZRULE_RANGE		0x10	/* with znum range */

/* data structure */
typedef struct scws_rule_item
{
        short flag;
        char zmin;      // 成词长度
        char zmax;
        char name[17];
        char attr[3];
        float tf;
        float idf;
        unsigned int bit;	/* my bit  */
        unsigned int inc;	/* include */
        unsigned int exc;	/* exclude */
}	*rule_item_t;

/* special attrs ratio list(single chain, 12bytes) */
typedef struct scws_rule_attr *rule_attr_t;
struct scws_rule_attr
{
        char attr1[2];
        char attr2[2];
        unsigned char npath[2];
        short ratio;
        rule_attr_t next;
};

typedef struct scws_rule
{
        xtree_t tree;
        rule_attr_t attr;
        struct scws_rule_item items[SCWS_RULE_MAX];
        int ref;	// hightman.20130110: refcount (zero to really free/close)
}	rule_st, *rule_t;

/* scws ruleset: api */

/* create & load ruleset, by fpath & charset */
rule_t scws_rule_new(const char *fpath, unsigned char *mblen);


/* fork ruleset */
rule_t scws_rule_fork(rule_t r);

/* free the memory & resource for ruleset */
void scws_rule_free(rule_t r);

/* get the rule tree record by str */
rule_item_t scws_rule_get(rule_t r, unsigned char *str, int len);

/* check bit */
int scws_rule_checkbit(rule_t r, const char *str, int len, unsigned int bit);

/* get rule attr x */
int scws_rule_attr_ratio(rule_t r, const char *attr1, const char *attr2, const unsigned char *npath);

/* check exclude or include */
int scws_rule_check(rule_t r, rule_item_t cr, const char *str, int len);


/*
 * dotslash.lu @ 2014-11-18
 * load JSON rules
 * */
rule_t scws_rule_json_new(const char *, int);
void scws_rule_json_set(rule_t, rule_item_t, cJSON *);
static void scws_rule_json_set_attrs(rule_t, rule_item_t, cJSON *);

#ifdef __cplusplus
}
#endif
#endif
