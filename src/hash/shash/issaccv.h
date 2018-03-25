/* 
	Copyright (C) 2004 PENTA SECURITY SYSTEMS, INC.
	All rights reserved

	THIS IS UNPUBLISHED PROPRIETARY 
	SOURCE CODE OF PENTA SECURITY SYSTEMS, INC.
	The copyright notice above does not evidence any actual or intended publication of 
	such source code.

	Filename : issaccv.h
*/

#ifndef _issaccv_h
#define _issaccv_h

#ifdef ISSACCV_EXPORTS
  #ifdef __BUILD_DLL
    #define LIB_SPEC __declspec(dllexport)
  #else
    #define LIB_SPEC
  #endif
#else
#ifdef WIN32
  #ifndef PENTA_BUILD_EXE
    #define LIB_SPEC __declspec(dllimport)
  #else
    #define LIB_SPEC 
  #endif
#else
  #define LIB_SPEC 
#endif 
#endif

#ifdef __cplusplus
extern "C" {
#endif

enum ISSACCV_ERROR {
  ISSACCV_SUCCESS = 0,
  ISSACCV_FAIL,
    
  ISSACCV_INVALID_CONF_PATH = 101,
  ISSACCV_INVALID_LICENSE_PATH,
  ISSACCV_BAD_CONF,
  ISSACCV_BAD_LICENSE,
  ISSACCV_INVALID_PRIVATEKEY_PATH,
  ISSACCV_INVALID_CERTIFICATE_PATH,
  ISSACCV_BAD_PRIVATEKEY,
  ISSACCV_BAD_CERTIFICATE,
  ISSACCV_NOT_ALLOWED_CERTIFICATE,
  ISSACCV_INVALID_PASSWD, //110
  ISSACCV_NEED_INIT,
  ISSACCV_FAIL_TO_READ_ROOT_CA_CERTIFICATE,
  ISSACCV_FAIL_TO_BASE64_DECODE,
  ISSACCV_FAIL_TO_VERIFY,
  ISSACCV_BAD_PRIVATEKEY_PIN,
  ISSACCV_FAIL_TO_SIGN,
  ISSACCV_INSUFFICIENT_BUFFER,
};

typedef struct _CONFIG {
  void* configdata;
} CONFIG ;

LIB_SPEC
void ISSACCV_Create(CONFIG *config);

LIB_SPEC 
void ISSACCV_Delete(CONFIG *config);

LIB_SPEC 
int ISSACCV_Initialize(const char *passwd, const char *cfg_path, const char* license_path, const char *disk_cache_dir, CONFIG *config);

LIB_SPEC 
int ISSACCV_VerifyCertificate(const char *certificate, int certificate_len, CONFIG *config);

LIB_SPEC
int ISSACCV_MakeSignature(char* signature_value, int* signature_value_len, int signature_value_max_len, 
                          char* message, int message_len, 
                          char* certificate_buf, int certificate_buf_len,
                          char* privatekey_buf, int privatekey_buf_len, char* privatekey_pin);

LIB_SPEC
int ISSACCV_VerifySignature(char* signature_value, int signature_value_len, char* message, int message_len, 
                            char* certificate_buf, int certificate_buf_len);

#ifdef __cplusplus
}
#endif

#endif
