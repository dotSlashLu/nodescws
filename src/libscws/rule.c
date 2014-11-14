/**
 * @file rule.c (auto surame & areaname & special group)
 * @author Hightman Mar
 * @editor set number ; syntax on ; set autoindent ; set tabstop=4 (vim)
 * $Id$
 */

#ifdef HAVE_CONFIG_H
#    include "config.h"
#endif

#ifdef WIN32
#    include "config_win32.h"
#endif

#ifdef __cplusplus
extern C {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "../cjson/cJSON.h"
#include "rule.h"

static inline int _rule_index_get(rule_t r, const char *name)
{
        int i;
        for (i = 0; i < SCWS_RULE_MAX; i++)
        {
                if (r->items[i].name[0] == '\0')
                        break;

                if (!strcasecmp(r->items[i].name, name))
                        return i;
        }
        return -1;
}

rule_t scws_rule_json_new(const char *fpath)
{
        FILE *fp;
        cJSON *rule_json;
        rule_t rules;
        rule_item_t rule;

        // read file
        if ((fp = fopen(fpath, "r")) == NULL)
                return NULL;
        fseek(fp, 0, SEEK_END);
        long len = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        char *content = (char*)malloc(len + 1);
        fread(content, 1, len, fp);
        fclose(fp);
        rule_json = cJSON_Parse(content);                       // parse json
        
        // alloc rules
        rules = (rule_t)malloc(sizeof(rule_st));
        memset(rules, 0, sizeof(rule_st));
        rules->ref = 1;                                     // gc counter
        if ((rules->tree = xtree_new(0, 1)) == NULL) {
                free(rules);
                return NULL;
        }

        char *directives[] = {
                "special",      // 
                "nostats",      // 停用词
                "attrs",        // 词性语法规则
                "noname",       // 名字停用词
                "symbol",       // 双字节符号
                "pubname",      // 姓和外文名共同部分
                "pubname2",     // 
                "pubname3",     // 
                "surname",      // 单姓
                "surname2",     // 复姓
                "areaname",     // 地区
                "areaname2",    // 双字地区
                "munit",        // 量词
                "chnum0",       // 中文数词
                "chnum1",
                "chnum2",
                "chnum3",
                "chnum4",
                "chnum5",
                "alpha",        // 多字节字母
                "foreign"       // 外文名
        };
        char *directive_attrs[] = {
                // "line"       // We don't need read by line or read by char any more
                                // every entry in json is an array
                "type",
                "exclude",
                "include",
                "znum",
                "tf",
                "idf",
                "attr"
        };
        size_t i;
        char *directive;
        for (i = 0; i < (sizeof(directives) / sizeof(char *)); i++) {
                directive = directives[i];
                printf("Processing rule entry: %s\n", directive);
                cJSON *json_rule_entry = cJSON_GetObjectItem(rule_json, directive);
                if (json_rule_entry == NULL) continue;

                
                strcpy(rules->items[i].name, directive);
                rules->items[i].tf = 5.0;
                rules->items[i].idf = 3.5;
                strncpy(rules->items[i].attr, "un", 2);

                // set rule.bit
                if (!strcmp(directive, "special"))
                        rules->items[i].bit = SCWS_RULE_SPECIAL;
                else if (!strcmp(directive, "attrs"))
                        rules->items[i].bit = SCWS_RULE_NOSTATS;
                else {
                        rules->items[i].bit = (1 << i);
                        i++;
                }

                rule = NULL;
                cJSON *json_rule_attrs; 
                if ((json_rule_attrs = cJSON_GetObjectItem(json_rule_entry, "attrs")) != NULL 
                        && json_rule_attrs->type == cJSON_Object)
                        // TODO
                        continue;
                         
                scws_set_rule_attrs(rule, cJSON *attrs);
        }

        return NULL;
}
void scws_set_rule_attrs(rule_item_t rule, cJSON *attrs)
{
        int len, i;
        cJSON *result;
        char *line, *ptr1, *ptr2;
        rule_attr_t rule_attr, rule_tail;

        if (attrs->type != cJSON_Array) return;
        len = cJSON_GetArrayLength();
        for (i = 0; i < len; i++) {
                result = cJSON_GetArrayItem(attrs);
                line = result->valuestring; 
                while (isspace(line)) line++;
                if ((ptr1 = strchr(line, '+')) == NULL) return;
                *ptr1++ = '\0';
                
                if ((ptr2 = strchr(line, '=')) == NULL) return;
                *ptr2++ = '\0';

                // init rule_attr
                rule_attr = (rule_attr_t)malloc(sizeof(struct scws_rule_attr));
                memset(rule_attr, 0, sizeof(struct scws_rule_attr));

                // set ratio
                rule_attr->ratio = (short)atoi(ptr2);
                if (rule_attr->ratio < 1) 
                        rule_attr->ratio = 1;
                rule_attr->npath[0] = rule_attr->npath[1] = 0xff;
                
                // read attr1 & npath1
                rule_attr->attr1[0] = *line++;
                if (*line && *line != '(' && !isspace(*line))
                        rule_attr[1] = *line++;
                while (*line && *line != '(') line++;
                if (*line == '(') {
                        line++;
                        if ((ptr2 = strchr(str, ')')) != NULL) {
                                *ptr2 = '\0'
                                rule_attr->npath[0] = (unsigned char)atoi(str);
                                if (rule_attr->npath[0] > 0)
                                        rule_attr->npath[0]--;
                                else
                                        rule_attr->npath[0] = 0xff;
                        }
                }

                /* read attr1 & npath2? */
                line = ptr;
                while (isspace(*line)) line++;
                rule_attr->attr2[0] = *str++;
                if (*line && *line != '(' && !isspace(line))
                        rule_attr->attr2[1] = *line++;
                while (*line && *line != '(') line++;
                if (*line == '(')
                {
                        line++;
                        if ((ptr2 = linechr(line, ')')) != NULL)
                        {
                                *ptr2 = '\0';
                                rule_attr->npath[1] = (unsigned char) atoi(line);
                                if (rule_attr->npath[1] > 0)
                                        rule_attr->npath[1]--;
                                else
                                        rule_attr->npath[1] = 0xff;
                        }
                }

                /* append to the chain list */
                if (rule->attr == NULL)
                        rule->attr = rule_tail = rule_attr;
                else
                {
                        // defined but not used
                        rule_tail = (rule_attr_t) malloc(sizeof(lineuct scws_rule_attr));
                        rule_tail->next = rule_attr;
                        rule_tail = rule_attr;
                }
        }
}

rule_t scws_rule_new(const char *fpath, unsigned char *mblen)
{
        scws_rule_json_new("src/cjson/tests/test.json");
        FILE *fp;
        rule_t r;
        rule_item_t cr;
        int i, j, rbl, aflag;
        rule_attr_t a, rtail;
        char buf[512], *str, *ptr, *qtr;
                                        
        /* loaded or open file failed */
        if ((fp = fopen(fpath, "r")) == NULL)
                return NULL;

        /* alloc the memory */
        r = (rule_t) malloc(sizeof(rule_st));
        memset(r, 0, sizeof(rule_st));
        r->ref = 1;

        /* quick scan to add the name to list */
        i = j = rbl = aflag = 0;
        while (fgets(buf, sizeof(buf) - 1, fp))
        {
                // skip non-directives
                if (buf[0] != '[' || !(ptr = strchr(buf, ']')))
                        continue;

                str = buf + 1;
                *ptr = '\0';
                // str - ptr is directive

                if (ptr == str                  // empty
                || (ptr - str) > 15             // length > 15
                || !strcasecmp(str, "attrs"))   // directive isn't attrs
                        continue;

                if (_rule_index_get(r, str) >= 0)       // if a same rule is already set
                        continue;

                strcpy(r->items[i].name, str);
                r->items[i].tf = 5.0;
                r->items[i].idf = 3.5;
                strncpy(r->items[i].attr, "un", 2);

                // set rule.bit according to directive
                if (!strcasecmp(str, "special"))
                        r->items[i].bit = SCWS_RULE_SPECIAL;
                else if (!strcasecmp(str, "nostats"))
                        r->items[i].bit = SCWS_RULE_NOSTATS;
                else
                {
                        r->items[i].bit = (1 << j);
                        j++;
                }

                if (++i >= SCWS_RULE_MAX)
                        break;
        }
        rewind(fp);

        /* load the tree data */
        if ((r->tree = xtree_new(0, 1)) == NULL)
        {
                free(r);
                return NULL;
        }
        cr = NULL;
        while (fgets(buf, sizeof(buf) - 1, fp))
        {
                // skip comments
                if (buf[0] == ';')
                        continue;

                if (buf[0] == '[')
                {
                        cr = NULL;
                        str = buf + 1;
                        aflag = 0;      // is attr
                        if ((ptr = strchr(str, ']')) != NULL)
                        {
                                *ptr = '\0';
                                if (!strcasecmp(str, "attrs"))
                                        aflag = 1;

                                else if ((i = _rule_index_get(r, str)) >= 0)
                                {
                                        rbl = 1; /* default read by line = yes */
                                        cr = &r->items[i]; // rule_item
                                }
                        }
                        continue;
                }


                // is attr (word attr table)
                if (aflag == 1)
                {
                        /* parse the attr line */
                        str = buf;
                        while (*str == ' ' || *str == '\t') str++;
                        if ((ptr = strchr(str, '+')) == NULL) continue;
                        *ptr++ = '\0';
                        if ((qtr = strchr(ptr, '=')) == NULL) continue;
                        *qtr++ = '\0';

                        /* create new memory */
                        a = (rule_attr_t) malloc(sizeof(struct scws_rule_attr));
                        memset(a, 0, sizeof(struct scws_rule_attr));

                        /* get ratio */
                        while (*qtr == ' ' || *qtr == '\t') qtr++;
                        a->ratio = (short) atoi(qtr);
                        if (a->ratio < 1)
                                a->ratio = 1;
                        a->npath[0] = a->npath[1] = 0xff;

                        /* read attr1 & npath1? */
                        a->attr1[0] = *str++;
                        if (*str && *str != '(' && *str != ' ' && *str != '\t')
                                a->attr1[1] = *str++;
                        while (*str && *str != '(') str++;
                        if (*str == '(')
                        {
                                str++;
                                if ((qtr = strchr(str, ')')) != NULL)
                                {
                                        *qtr = '\0';
                                        a->npath[0] = (unsigned char) atoi(str);
                                        if (a->npath[0] > 0)
                                                a->npath[0]--;
                                        else
                                                a->npath[0] = 0xff;
                                }
                        }

                        /* read attr1 & npath2? */
                        str = ptr;
                        while (*str == ' ' || *str == '\t') str++;
                        a->attr2[0] = *str++;
                        if (*str && *str != '(' && *str != ' ' && *str != '\t')
                                a->attr2[1] = *str++;
                        while (*str && *str != '(') str++;
                        if (*str == '(')
                        {
                                str++;
                                if ((qtr = strchr(str, ')')) != NULL)
                                {
                                        *qtr = '\0';
                                        a->npath[1] = (unsigned char) atoi(str);
                                        if (a->npath[1] > 0)
                                                a->npath[1]--;
                                        else
                                                a->npath[1] = 0xff;
                                }
                        }

                        //printf("%c%c(%d)+%c%c(%d)=%d\n", a->attr1[0], a->attr1[1] ? a->attr1[1] : ' ', a->npath[0],
                        //	a->attr2[0], a->attr2[1] ? a->attr2[1] : ' ', a->npath[1], a->ratio);

                        /* append to the chain list */
                        if (r->attr == NULL)
                                r->attr = rtail = a;
                        else
                        {
                                rtail = (rule_attr_t) malloc(sizeof(struct scws_rule_attr));
                                rtail->next = a;
                                rtail = a;
                        }

                        continue;
                } // if (aflag)

                if (cr == NULL)
                        continue;

                /* param set: line|znum|include|exclude|type|tf|idf|attr */
                if (buf[0] == ':')
                {
                        str = buf + 1;
                        if (!(ptr = strchr(str, '=')))
                                continue;
                        while (*str == ' ' || *str == '\t') str++;

                        qtr = ptr + 1;
                        while (ptr > str && (ptr[-1] == ' ' || ptr[-1] == '\t')) ptr--;
                        *ptr = '\0';
                        ptr = str;
                        str = qtr;
                        while (*str == ' ' || *str == '\t') str++;

                        if (!strcmp(ptr, "line"))
                                rbl = (*str == 'N' || *str == 'n') ? 0 : 1;     // read by line
                        else if (!strcmp(ptr, "tf"))
                                cr->tf = (float) atof(str);
                        else if (!strcmp(ptr, "idf"))
                                cr->idf = (float) atof(str);
                        else if (!strcmp(ptr, "attr"))
                                strncpy(cr->attr, str, 2);
                        else if (!strcmp(ptr, "znum"))
                        {
                                if ((ptr = strchr(str, ',')) != NULL)
                                {
                                        *ptr++ = '\0';
                                        while (*ptr == ' ' || *ptr == '\t') ptr++;
                                        cr->zmax = atoi(ptr);
                                        cr->flag |= SCWS_ZRULE_RANGE;
                                }
                                cr->zmin = atoi(str);
                        }
                        else if (!strcmp(ptr, "type"))
                        {
                                if (!strncmp(str, "prefix", 6))
                                        cr->flag |= SCWS_ZRULE_PREFIX;
                                else if (!strncmp(str, "suffix", 6))
                                        cr->flag |= SCWS_ZRULE_SUFFIX;
                        }
                        else if (!strcmp(ptr, "include") || !strcmp(ptr, "exclude"))
                        {
                                unsigned int *clude;

                                if (!strcmp(ptr, "include"))
                                {
                                        clude = &cr->inc;
                                        cr->flag |= SCWS_ZRULE_INCLUDE;
                                }
                                else
                                {
                                        clude = &cr->exc;
                                        cr->flag |= SCWS_ZRULE_EXCLUDE;
                                }

                                while ((ptr = strchr(str, ',')) != NULL)
                                {
                                        while (ptr > str && (ptr[-1] == '\t' || ptr[-1] == ' ')) ptr--;
                                        *ptr = '\0';
                                        if ((i = _rule_index_get(r, str)) >= 0)
                                                *clude |= r->items[i].bit;

                                        str = ptr + 1;
                                        while (*str == ' ' || *str == '\t' || *str == ',') str++;
                                }

                                ptr = strlen(str) + str;
                                while (ptr > str && strchr(" \t\r\n", ptr[-1])) ptr--;
                                *ptr = '\0';
                                if (ptr > str && (i = _rule_index_get(r, str)))
                                        *clude |= r->items[i].bit;
                        }
                        continue;
                } // if (buf[0] == ':')

                /* read the entries */
                str = buf;
                while (*str == ' ' || *str == '\t') str++;              // ltrim
                ptr = str + strlen(str);
                while (ptr > str && strchr(" \t\r\n", ptr[-1])) ptr--;  // rtrim
                *ptr = '\0';

                /* emptry line */
                if (ptr == str)
                        continue;

                if (rbl)        // put entire line
                        xtree_nput(r->tree, cr, sizeof(struct scws_rule_item), str, ptr - str);
                else
                {
                        while (str < ptr)
                        {
                                j = mblen[(int)(*str)];
#ifdef DEBUG
                                /* try to check repeat */
                                if ((i = (int) xtree_nget(r->tree, str, j, NULL)) != 0)
                                        fprintf(stderr, "Reapeat word on %s|%s: %.*s\n", cr->name, ((rule_item_t) i)->name, j, str);
#endif
                                xtree_nput(r->tree, cr, sizeof(struct scws_rule_item), str, j);
                                str += j;
                        }
                }
        }
        fclose(fp);

        /* optimize the tree */
        xtree_optimize(r->tree);
        return r;
}

/* fork rule */
rule_t scws_rule_fork(rule_t r)
{
        if (r != NULL)
                r->ref++;
        return r;
}

/* free rule */
void scws_rule_free(rule_t r)
{
        if (r)
        {
                r->ref--;
                if (r->ref == 0)
                {
                        rule_attr_t a, b;

                        xtree_free(r->tree);
                        a = r->attr;
                        while (a != NULL)
                        {
                                b = a;
                                a = b->next;
                                free(b);
                        }
                        free(r);
                }
        }
}

/* get the rule */
rule_item_t scws_rule_get(rule_t r, unsigned char *str, int len)
{
        if (!r)
                return NULL;

        return((rule_item_t) xtree_nget(r->tree, str, len, NULL));
}

/* check the bit with str */
int scws_rule_checkbit(rule_t r, const char *str, int len, unsigned int bit)
{
        rule_item_t ri;

        if (!r)
                return 0;

        ri = (rule_item_t) xtree_nget(r->tree, str, len, NULL);
        if ((ri != NULL) && (ri->bit & bit))
                return 1;

        return 0;
}

/* get rule attr x */
#define	EQUAL_RULE_ATTR(x,y)	((y[0]=='*'||y[0]==x[0])&&(y[1]=='\0'||y[1]==x[1]))
#define	EQUAL_RULE_NPATH(x,y)	((y[0]==0xff||y[0]==x[0])&&(y[1]==0xff||y[1]==x[1]))

int scws_rule_attr_ratio(rule_t r, const char *attr1, const char *attr2, const unsigned char *npath)
{
        rule_attr_t a;
        int ret = 1;

        if (!r || (a = r->attr) == NULL)
                return ret;

        while (a != NULL)
        {
                if (EQUAL_RULE_ATTR(attr1, a->attr1) && EQUAL_RULE_ATTR(attr2, a->attr2) && EQUAL_RULE_NPATH(npath, a->npath))
                {
                        ret = (int) a->ratio;
                        break;
                }
                a = a->next;
        }
        return ret;
}

#undef EQUAL_RULE_ATTR
#undef EQUAL_RULE_NPATH

/* check the rule */
int scws_rule_check(rule_t r, rule_item_t cr, const char *str, int len)
{
        if (!r)
                return 0;

        if ((cr->flag & SCWS_ZRULE_INCLUDE) && !scws_rule_checkbit(r, str, len, cr->inc))
                return 0;

        if ((cr->flag & SCWS_ZRULE_EXCLUDE) && scws_rule_checkbit(r, str, len, cr->exc))
                return 0;

        return 1;
}

#ifdef __cplusplus
}
#endif
