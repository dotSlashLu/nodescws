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

rule_t scws_json_rule_new(const char *fpath)
{
        FILE *fp;
        cJSON *json_rules;
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
        json_rules = cJSON_Parse(content);                            // parse json
        if ((json_rules == NULL) || 
            json_rules->type != cJSON_Object) {
                printf("Parse failed\n");
                return NULL;
        }
        
        // alloc rules
        rules = (rule_t)malloc(sizeof(rule_st));
        memset(rules, 0, sizeof(rule_st));
        rules->ref = 1;                                              // gc counter
        if ((rules->tree = xtree_new(0, 1)) == NULL) {
                free(rules);
                return NULL;
        }

        size_t i = 0;
        const char *directive;
        cJSON *json_rule_ents, *json_rule_ent, *json_rule_values;
        if ((json_rule_ents = json_rules->child) == NULL) return NULL;
        json_rule_ent = json_rule_ents;
        while ((json_rule_ent) != NULL) {
                directive = json_rule_ent->string;
                printf("\n\nSetting JSON rule entry: %s\n", directive);
                // cJSON *json_rule_entry = cJSON_GetObjectItem(json_rules, directive);
                // if (json_rule_entry == NULL) continue;

                
                strcpy(rules->items[i].name, json_rule_ent->string);
                rules->items[i].tf = 5.0;
                rules->items[i].idf = 3.5;
                strncpy(rules->items[i].attr, "un", 2);

                // set rule.bit
                if (!strcmp(directive, "special"))
                        rules->items[i].bit = SCWS_RULE_SPECIAL;
                else if (!strcmp(directive, "attrs"))
                        rules->items[i].bit = SCWS_RULE_NOSTATS;
                else
                        rules->items[i].bit = (1 << i);

                rule = &(rules->items[i]);
                cJSON *json_rule_attrs; 
                if ((json_rule_attrs = cJSON_GetObjectItem(json_rule_ent, "attrs")) != NULL 
                        && json_rule_attrs->type == cJSON_Object)
                        scws_set_json_rule_attrs(rules, rule, json_rule_attrs);

                if ((json_rule_values = cJSON_GetObjectItem(json_rule_ent, "value")) != NULL)
                        scws_set_json_rule(rules, rule, json_rule_values);
                i++;
                json_rule_ent = json_rule_ent->next;
        }

        xtree_optimize(rules->tree);
        cJSON_Delete(json_rules);
        return rules;
}

void scws_set_json_rule(rule_t rules, rule_item_t rule, cJSON *rulevalue)
{
        char *rulename = rule->name, *valuestring, *ptr, *qtr;
        size_t valuelen, i;

        printf("Setting: %s\n", rulename);

        if (rulevalue == NULL) return;
        // attrs
        if (!strcmp(rulename, "attrs")) {
                if (rulevalue->type != cJSON_Array 
                        || (valuelen = cJSON_GetArraySize(rulevalue)) == 0)
                        return;
                // while ((rulevalue = rulevalue->next) != NULL) {
                for (i = 0; i < valuelen; i++) {
                        valuestring = cJSON_GetArrayItem(rulevalue, i)->valuestring;
                        printf("value line: %s\n", valuestring);
                        if ((ptr = strchr(valuestring, '+')) == NULL) continue;
                        *ptr++ = '\0';
                        if ((qtr = strchr(valuestring, '=')) == NULL) continue;
                        *qtr++ = '\0';

                        rule_attr_t value, rtail;
                        value = (rule_attr_t)calloc(1, sizeof(struct scws_rule_attr));

                        while (isspace(*qtr)) qtr++;
                        value->ratio = (short)atoi(qtr);
                        if (value->ratio < 1)
                                value->ratio = 1;
                        value->npath[0] = value->npath[1] = 0xff;

                        value->attr1[0] = *valuestring++;
                        if (*valuestring && *valuestring != ')' && isspace(*valuestring))
                                value->attr1[1] = *valuestring++;
                        while (*valuestring && *valuestring != '(') valuestring++;
                        if (*valuestring == '(') {
                                valuestring++;
                                if ((qtr = strchr(valuestring, ')')) != NULL) {
                                        *qtr = '\0';
                                        value->npath[0] = (unsigned char)atoi(valuestring);
                                        if (value->npath[0] > 0)
                                                value->npath[0]--;
                                        else
                                                value->npath[0] = 0xff;
                                }
                        }

                        valuestring = ptr;
                        while (isspace(*valuestring)) valuestring++;
                        value->attr2[0] = *valuestring++;
                        if (*valuestring && *valuestring != '(' && *valuestring != ' ' && *valuestring != '\t')
                                value->attr2[1] = *valuestring++;
                        while (*valuestring && *valuestring != '(') valuestring++;
                        if (*valuestring == '(')
                        {
                                valuestring++;
                                if ((qtr = strchr(valuestring, ')')) != NULL)
                                {
                                        *qtr = '\0';
                                        value->npath[1] = (unsigned char)atoi(valuestring);
                                        if (value->npath[1] > 0)
                                                value->npath[1]--;
                                        else
                                                value->npath[1] = 0xff;
                                }
                        }

                        /* append to the chain list */
                        if (rules->attr == NULL)
                                rules->attr = rtail = value;
                        else {
                                rtail = (rule_attr_t)calloc(1, sizeof(struct scws_rule_attr));
                                rtail->next = value;
                                rtail = value;
                        }
                }
                return;
        } // if rulename == attrs

        // valuestring except attrs
        // array value
        else if (rulevalue->type == cJSON_Array) {
                if ((valuelen = cJSON_GetArraySize(rulevalue)) < 1) return;
                for (i = 0; i < valuelen; i++) {
                        valuestring = cJSON_GetArrayItem(rulevalue, i)->valuestring;
                        printf("value line(%d): %s\n", i, valuestring);
                        while (isspace(*valuestring)) valuestring++;
                        ptr = valuestring + strlen(valuestring);
                        while (ptr > valuestring && strchr(" \t\r\n", ptr[-1])) ptr--;
                        *ptr = '\0';

                        if (ptr == valuestring) continue;
                        xtree_nput(rules->tree, rule, sizeof(struct scws_rule_item), valuestring, ptr - valuestring);
                }
        }
        // other types?
}

void scws_set_json_rule_attrs(rule_t rules, rule_item_t rule, cJSON *attrs)
{        
        char *attrname;
        cJSON *json_attr;

        json_attr = attrs->child;
        while (json_attr != NULL) {
                attrname = json_attr->string;
                printf("Setting JSON rule attr: %s\n", attrname);
                if (!strcmp(attrname, "tf"))
                        rule->tf = (float)json_attr->valuedouble;
                else if (!strcmp(attrname, "idf"))
                        rule->idf = (float)json_attr->valuedouble;
                else if (!strcmp(attrname, "attr"))
                        strncpy(rule->attr, json_attr->valuestring, 2);
                else if (!strcmp(attrname, "znum")) {
                        // znum: {min: a, max: b} | a
                        if (json_attr->type == cJSON_Object) {
                                if (cJSON_GetObjectItem(json_attr, "min"))
                                        rule->zmin = (int)cJSON_GetObjectItem(json_attr, "min")->valueint;
                                if (cJSON_GetObjectItem(json_attr, "max"))
                                        rule->zmax = (int)cJSON_GetObjectItem(json_attr, "max")->valueint;
                        }

                        else if (json_attr->type == cJSON_Number)
                                rule->zmin = (int)json_attr->valueint;
                }
                else if (!strcmp(attrname, "type")) {
                        if (!strncmp(json_attr->valuestring, "prefix", 6))
                                rule->flag |= SCWS_ZRULE_PREFIX;
                        else if(!strncmp(json_attr->valuestring, "suffix", 6))
                                rule->flag |= SCWS_ZRULE_SUFFIX;
                }
                else if (!strcmp(attrname, "include") || !strcmp(attrname, "exclude")) {
                        unsigned int *clude;

                        if (!strcmp(attrname, "include")) {
                                clude = &rule->inc;
                                rule->flag |= SCWS_ZRULE_INCLUDE;
                        }
                        else {
                                clude = &rule->exc;
                                rule->flag |= SCWS_ZRULE_EXCLUDE;
                        }
                        // loop thru xclude values
                        int k, j;
                        for (k = 0; k < cJSON_GetArraySize(json_attr); k++) {
                                char *cludename = cJSON_GetArrayItem(json_attr, k)->valuestring;
                                if ((j = _rule_index_get(rules, cludename)) >= 0)
                                        *clude |= rules->items[j].bit;
                        }
                }
                json_attr = json_attr->next;
        }
}



rule_t scws_rule_new(const char *fpath, unsigned char *mblen)
{
        scws_json_rule_new("rules/rules.utf8.json");
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
                } // if buf[0] == [


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
                } // if aflag

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
