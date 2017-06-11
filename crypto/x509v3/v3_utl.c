/* v3_utl.c */
/* Written by Dr Stephen N Henson (shenson@bigfoot.com) for the OpenSSL
 * project 1999.
 */
/* ====================================================================
 * Copyright (c) 1999 The OpenSSL Project.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer. 
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. All advertising materials mentioning features or use of this
 *    software must display the following acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit. (http://www.OpenSSL.org/)"
 *
 * 4. The names "OpenSSL Toolkit" and "OpenSSL Project" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. For written permission, please contact
 *    licensing@OpenSSL.org.
 *
 * 5. Products derived from this software may not be called "OpenSSL"
 *    nor may "OpenSSL" appear in their names without prior written
 *    permission of the OpenSSL Project.
 *
 * 6. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit (http://www.OpenSSL.org/)"
 *
 * THIS SOFTWARE IS PROVIDED BY THE OpenSSL PROJECT ``AS IS'' AND ANY
 * EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE OpenSSL PROJECT OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 * ====================================================================
 *
 * This product includes cryptographic software written by Eric Young
 * (eay@cryptsoft.com).  This product includes software written by Tim
 * Hudson (tjh@cryptsoft.com).
 *
 */
/* X509 v3 extension utilities */

#include <stdlib.h>
#include <ctype.h>
#include <pem.h>
#include <conf.h>
#include <err.h>
#include "x509v3.h"

static char * str_dup(char *str);
static char *strip_spaces(char *name);

static char *str_dup(str)
char *str;
{
	char *tmp;
	if(!(tmp = Malloc(strlen(str) + 1))) return NULL;
	strcpy(tmp, str);
	return tmp;
}

/* Add a CONF_VALUE name value pair to stack */

int X509V3_add_value(name, value, extlist)
char *name;
char *value;
STACK **extlist;
{
	CONF_VALUE *vtmp = NULL;
	char *tname = NULL, *tvalue = NULL;
	if(name && !(tname = str_dup(name))) goto err;
	if(value && !(tvalue = str_dup(value))) goto err;;
	if(!(vtmp = (CONF_VALUE *)Malloc(sizeof(CONF_VALUE)))) goto err;
	if(!*extlist && !(*extlist = sk_new(NULL))) goto err;
	vtmp->section = NULL;
	vtmp->name = tname;
	vtmp->value = tvalue;
	if(!sk_push(*extlist, (char *)vtmp)) goto err;
	return 1;
	err:
	X509V3err(X509V3_F_X509V3_ADD_VALUE,ERR_R_MALLOC_FAILURE);
	if(vtmp) Free(vtmp);
	if(tname) Free(tname);
	if(tvalue) Free(tvalue);
	return 0;
}

/* Free function for STACK of CONF_VALUE */

void X509V3_conf_free(conf)
CONF_VALUE *conf;
{
	if(!conf) return;
	if(conf->name) Free(conf->name);
	if(conf->value) Free(conf->value);
	if(conf->section) Free(conf->section);
	Free((char *)conf);
}

int X509V3_add_value_bool(name, asn1_bool, extlist)
char *name;
int asn1_bool;
STACK **extlist;
{
	if(asn1_bool) return X509V3_add_value(name, "TRUE", extlist);
	return X509V3_add_value(name, "FALSE", extlist);
}

int X509V3_add_value_bool_nf(name, asn1_bool, extlist)
char *name;
int asn1_bool;
STACK **extlist;
{
	if(asn1_bool) return X509V3_add_value(name, "TRUE", extlist);
	return 1;
}

int X509V3_add_value_int(name, aint, extlist)
char *name;
ASN1_INTEGER *aint;
STACK **extlist;
{
	BIGNUM *bntmp;
	char *strtmp;
	int ret;
	if(!aint) return 1;
	bntmp = ASN1_INTEGER_to_BN(aint, NULL);
	strtmp = BN_bn2dec(bntmp);
	ret = X509V3_add_value(name, strtmp, extlist);
	BN_free(bntmp);
	Free(strtmp);
	return ret;
}

int X509V3_get_value_bool(value, asn1_bool)
CONF_VALUE *value;
int *asn1_bool;
{
	char *btmp;
	if(!(btmp = value->value)) goto err;
	if(!strcmp(btmp, "TRUE") || !strcmp(btmp, "true")
		 || !strcmp(btmp, "Y") || !strcmp(btmp, "y")
		|| !strcmp(btmp, "YES") || !strcmp(btmp, "yes")) {
		*asn1_bool = 0xff;
		return 1;
	} else if(!strcmp(btmp, "FALSE") || !strcmp(btmp, "false")
		 || !strcmp(btmp, "N") || !strcmp(btmp, "n")
		|| !strcmp(btmp, "NO") || !strcmp(btmp, "no")) {
		*asn1_bool = 0;
		return 1;
	}
	err:
	X509V3err(X509V3_F_X509V3_VALUE_GET_BOOL,X509V3_R_INVALID_BOOLEAN_STRING);
	X509V3_conf_err(value);
	return 0;
}

int X509V3_get_value_int(value, aint)
CONF_VALUE *value;
ASN1_INTEGER **aint;
{
	BIGNUM *bn = NULL;
	bn = BN_new();
	if(!value->value) {
		X509V3err(X509V3_F_X509V3_GET_VALUE_INT,X509V3_R_INVALID_NULL_VALUE);
		X509V3_conf_err(value);
		return 0;
	}
	if(!BN_dec2bn(&bn, value->value)) {
		X509V3err(X509V3_F_X509V3_GET_VALUE_INT,X509V3_R_BN_DEC2BN_ERROR);
		X509V3_conf_err(value);
		return 0;
	}

	if(!(*aint = BN_to_ASN1_INTEGER(bn, NULL))) {
		X509V3err(X509V3_F_X509V3_GET_VALUE_INT,X509V3_R_BN_TO_ASN1_INTEGER_ERROR);
		X509V3_conf_err(value);
		return 0;
	}
	BN_free(bn);
	return 1;
}

#define HDR_NAME	1
#define HDR_VALUE	2

/*#define DEBUG*/

STACK *X509V3_parse_list(line)
char *line;
{
	char *p, *q, c;
	char *ntmp, *vtmp;
	STACK *values = NULL;
	char *linebuf;
	int state;
	/* We are going to modify the line so copy it first */
	linebuf = str_dup(line);
	state = HDR_NAME;
	ntmp = NULL;
	/* Go through all characters */
	for(p = linebuf, q = linebuf; (c = *p) && (c!='\r') && (c!='\n'); p++) {

		switch(state) {
			case HDR_NAME:
			if(c == ':') {
				state = HDR_VALUE;
				*p = 0;
				ntmp = strip_spaces(q);
				if(!ntmp) {
					X509V3err(X509V3_F_X509V3_PARSE_LIST, X509V3_R_INVALID_NULL_NAME);
					goto err;
				}
				q = p + 1;
			} else if(c == ',') {
				*p = 0;
				ntmp = strip_spaces(q);
				q = p + 1;
#ifdef DEBUG
				printf("%s\n", ntmp);
#endif
				if(!ntmp) {
					X509V3err(X509V3_F_X509V3_PARSE_LIST, X509V3_R_INVALID_NULL_NAME);
					goto err;
				}
				X509V3_add_value(ntmp, NULL, &values);
			}
			break ;

			case HDR_VALUE:
			if(c == ',') {
				state = HDR_NAME;
				*p = 0;
				vtmp = strip_spaces(q);
#ifdef DEBUG
				printf("%s\n", ntmp);
#endif
				if(!vtmp) {
					X509V3err(X509V3_F_X509V3_PARSE_LIST, X509V3_R_INVALID_NULL_VALUE);
					goto err;
				}
				X509V3_add_value(ntmp, vtmp, &values);
				ntmp = NULL;
				q = p + 1;
			}

		}
	}

	if(state == HDR_VALUE) {
		vtmp = strip_spaces(q);
#ifdef DEBUG
		printf("%s=%s\n", ntmp, vtmp);
#endif
		if(!vtmp) {
			X509V3err(X509V3_F_X509V3_PARSE_LIST, X509V3_R_INVALID_NULL_VALUE);
			goto err;
		}
		X509V3_add_value(ntmp, vtmp, &values);
	} else {
		ntmp = strip_spaces(q);
#ifdef DEBUG
		printf("%s\n", ntmp);
#endif
		if(!ntmp) {
			X509V3err(X509V3_F_X509V3_PARSE_LIST, X509V3_R_INVALID_NULL_NAME);
			goto err;
		}
		X509V3_add_value(ntmp, NULL, &values);
	}
Free(linebuf);
return values;

err:
Free(linebuf);
sk_pop_free(values, X509V3_conf_free);
return NULL;

}

/* Delete leading and trailing spaces from a string */
static char *strip_spaces(name)
char *name;
{
	char *p, *q;
	/* Skip over leading spaces */
	p = name;
	while(*p && isspace(*p)) p++;
	if(!*p) return NULL;
	q = p + strlen(p) - 1;
	while((q != p) && isspace(*q)) q--;
	if(p != q) q[1] = 0;
	if(!*p) return NULL;
	return p;
}