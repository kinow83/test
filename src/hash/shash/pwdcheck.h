/*
* @file  PasswordCheck.h
*/

#pragma once

#define PASSWORD_ACCEPTED 1
#define PASSWORD_DENIED_TOO_SHORT -11
#define PASSWORD_DENIED_CONTINUOS -12
#define PASSWORD_DENIED_NEED_ALPHANUMERIC_MIX -13
#define PASSWORD_DENIED_NEED_SYMBOL -14
#define PASSWORD_DENIED_REPEATATION -15
#define PASSWORD_DENIED_FORBIDDEN_WORD -16
#define PASSWORD_DENIED_SAME_PWD_CHG -17

/**
* @brief  패스워드가 dictionary attack에 취약한지 조사함
*/

struct PasswordPolicy {
    int bMinLength;
    int bUseAlphaNumeric;
    int bUseSymbol;
    
    int nMinLength;
    int nRepeatLimitCount;
    int nContinuosLimitCount;

    char pForbiddenPasswordList[256];

	int bAllowSamePwdChange;
};

int password_policy_check(struct PasswordPolicy *policy, char *new_pwd, char *old_pwd, char *errmsg);

