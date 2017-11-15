/* v3_akey.c */
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

#include <stdio.h>
#include "cryptlib.h"
#include "conf.h"
#include "asn1.h"
#include "asn1_mac.h"
#include "x509v3.h"

#ifndef NOPROTO
static STACK *i2v_AUTHORITY_KEYID(X509V3_EXT_METHOD *method, AUTHORITY_KEYID *akeyid, STACK *extlist);
static AUTHORITY_KEYID *v2i_AUTHORITY_KEYID(X509V3_EXT_METHOD *method, X509V3_CTX *ctx, STACK *values);

#else

static STACK *i2v_AUTHORITY_KEYID();
static AUTHORITY_KEYID *v2i_AUTHORITY_KEYID();

#endif

X509V3_EXT_METHOD v3_akey_id = {
NID_authority_key_identifier, 0,
(X509V3_EXT_NEW)AUTHORITY_KEYID_new,
AUTHORITY_KEYID_free,
(X509V3_EXT_D2I)d2i_AUTHORITY_KEYID,
i2d_AUTHORITY_KEYID,
NULL, NULL,
(X509V3_EXT_I2V)i2v_AUTHORITY_KEYID,
(X509V3_EXT_V2I)v2i_AUTHORITY_KEYID,
NULL,
NULL
};


/*
 * ASN1err(ASN1_F_AUTHORITY_KEYID_NEW,ERR_R_MALLOC_FAILURE);
 * ASN1err(ASN1_F_D2I_AUTHORITY_KEYID,ERR_R_MALLOC_FAILURE);
 */

int i2d_AUTHORITY_KEYID(a,pp)
AUTHORITY_KEYID *a;
unsigned char **pp;
{
	M_ASN1_I2D_vars(a);

	M_ASN1_I2D_len_IMP_opt (a->keyid, i2d_ASN1_OCTET_STRING);
	M_ASN1_I2D_len_IMP_opt (a->issuer, i2d_GENERAL_NAMES);
	M_ASN1_I2D_len_IMP_opt (a->serial, i2d_ASN1_INTEGER);

	M_ASN1_I2D_seq_total();

	M_ASN1_I2D_put_IMP_opt (a->keyid, i2d_ASN1_OCTET_STRING, 0);
	M_ASN1_I2D_put_IMP_opt (a->issuer, i2d_GENERAL_NAMES, 1);
	M_ASN1_I2D_put_IMP_opt (a->serial, i2d_ASN1_INTEGER, 2);

	M_ASN1_I2D_finish();
}

AUTHORITY_KEYID *AUTHORITY_KEYID_new()
{
	AUTHORITY_KEYID *ret=NULL;
	ASN1_CTX c;
	M_ASN1_New_Malloc(ret, AUTHORITY_KEYID);
	ret->keyid = NULL;
	ret->issuer = NULL;
	ret->serial = NULL;
	return (ret);
	M_ASN1_New_Error(ASN1_F_AUTHORITY_KEYID_NEW);
}

AUTHORITY_KEYID *d2i_AUTHORITY_KEYID(a,pp,length)
AUTHORITY_KEYID **a;
unsigned char **pp;
long length;
{
	M_ASN1_D2I_vars(a,AUTHORITY_KEYID *,AUTHORITY_KEYID_new);
	M_ASN1_D2I_Init();
	M_ASN1_D2I_start_sequence();
	M_ASN1_D2I_get_IMP_opt (ret->keyid, d2i_ASN1_OCTET_STRING, 0,
							V_ASN1_OCTET_STRING);
	M_ASN1_D2I_get_IMP_opt (ret->issuer, d2i_GENERAL_NAMES, 1,
							V_ASN1_SEQUENCE);
	M_ASN1_D2I_get_IMP_opt (ret->serial, d2i_ASN1_INTEGER, 2,
							V_ASN1_INTEGER);
	M_ASN1_D2I_Finish(a, AUTHORITY_KEYID_free, ASN1_F_D2I_AUTHORITY_KEYID);
}

void AUTHORITY_KEYID_free(a)
AUTHORITY_KEYID *a;
{
	if (a == NULL) return;
	ASN1_OCTET_STRING_free(a->keyid);
	sk_pop_free(a->issuer, GENERAL_NAME_free);
	ASN1_INTEGER_free (a->serial);
	Free ((char *)a);
}

static STACK *i2v_AUTHORITY_KEYID(method, akeyid, extlist)
X509V3_EXT_METHOD *method;
AUTHORITY_KEYID *akeyid;
STACK *extlist;
{
	char *tmp;
	if(akeyid->keyid) {
		tmp = hex_to_string(akeyid->keyid->data, akeyid->keyid->length);
		X509V3_add_value("keyid", tmp, &extlist);
		Free(tmp);
	}
	if(akeyid->issuer) 
		extlist = i2v_GENERAL_NAMES(NULL, akeyid->issuer, extlist);
	if(akeyid->serial) {
		tmp = hex_to_string(akeyid->serial->data,
						 akeyid->serial->length);
		X509V3_add_value("serial", tmp, &extlist);
		Free(tmp);
	}
	return extlist;
}

/* Currently two options:
 * keyid: use the issuers subject keyid, the value 'always' means its is
 * an error if the issuer certificate doesn't have a key id.
 * issuer: use the issuers cert issuer and serial number. The default is
 * to only use this if keyid is not present. With the option 'always'
 * this is always included.
 */

static AUTHORITY_KEYID *v2i_AUTHORITY_KEYID(method, ctx, values)
X509V3_EXT_METHOD *method;
X509V3_CTX *ctx;
STACK *values;
{
char keyid=0, issuer=0;
int i;
CONF_VALUE *cnf;
ASN1_OCTET_STRING *ikeyid = NULL;
X509_NAME *isname = NULL;
STACK * gens = NULL;
GENERAL_NAME *gen = NULL;
ASN1_INTEGER *serial = NULL;
X509_EXTENSION *ext;
X509 *cert;
AUTHORITY_KEYID *akeyid;
for(i = 0; i < sk_num(values); i++) {
	cnf = (CONF_VALUE *)sk_value(values, i);
	if(!strcmp(cnf->name, "keyid")) {
		keyid = 1;
		if(cnf->value && !strcmp(cnf->value, "always")) keyid = 2;
	} else if(!strcmp(cnf->name, "issuer")) {
		issuer = 1;
		if(cnf->value && !strcmp(cnf->value, "always")) issuer = 2;
	} else {
		X509V3err(X509V3_F_V2I_AUTHORITY_KEYID,X509V3_R_UNKNOWN_OPTION);
		ERR_add_error_data(2, "name=", cnf->name);
		return NULL;
	}
}



if(!ctx || !ctx->issuer_cert) {
	if(ctx && (ctx->flags==CTX_TEST)) return AUTHORITY_KEYID_new();
	X509V3err(X509V3_F_V2I_AUTHORITY_KEYID,X509V3_R_NO_ISSUER_CERTIFICATE);
	return NULL;
}

cert = ctx->issuer_cert;

if(keyid) {
	i = X509_get_ext_by_NID(cert, NID_subject_key_identifier, -1);
	if((i >= 0)  && (ext = X509_get_ext(cert, i)))
			ikeyid = (ASN1_OCTET_STRING *) X509V3_EXT_d2i(ext);
	if(keyid==2 && !ikeyid) {
		X509V3err(X509V3_F_V2I_AUTHORITY_KEYID,X509V3_R_UNABLE_TO_GET_ISSUER_KEYID);
		return NULL;
	}
}

if((issuer && !ikeyid) || (issuer == 2)) {
	isname = X509_NAME_dup(X509_get_issuer_name(cert));
	serial = ASN1_INTEGER_dup(X509_get_serialNumber(cert));
	if(!isname || !serial) {
		X509V3err(X509V3_F_V2I_AUTHORITY_KEYID,X509V3_R_UNABLE_TO_GET_ISSUER_DETAILS);
		goto err;
	}
}

if(!(akeyid = AUTHORITY_KEYID_new())) goto err;

if(isname) {
	if(!(gens = sk_new(NULL)) || !(gen = GENERAL_NAME_new())
		|| !sk_push(gens, (char *)gen)) {
		X509V3err(X509V3_F_V2I_AUTHORITY_KEYID,ERR_R_MALLOC_FAILURE);
		goto err;
	}
	gen->type = GEN_DIRNAME;
	gen->d.dirn = isname;
}

akeyid->issuer = gens;
akeyid->serial = serial;
akeyid->keyid = ikeyid;

return akeyid;

err:
X509_NAME_free(isname);
ASN1_INTEGER_free(serial);
ASN1_OCTET_STRING_free(ikeyid);
return NULL;

}

