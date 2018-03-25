/*
* @file  PasswordCheck.cpp
* @brief  패스워드가 dictionary attack에 취약한지 조사함
*/

#include <stdio.h>
#include <string.h>
#include "pwdcheck.h"

/*
PasswordPolicy::PasswordPolicy()
{
    bUseAlphaNumeric = false;
    bUseSymbol = false;
    nContinuosLimitCount = 0;
    nMinLength = 0;
    nRepeatLimitCount = 0;
    pForbiddenPasswordList = 0;
    bForcePasswordChange = false;
}
*/

static int IsContainSymbol( char* pstrPassword )
{
    int bResult = 0;
    size_t nLength = strlen(pstrPassword);
    size_t n;
    for (n=0; n<nLength; n++)
    {
        if ( (pstrPassword[n] < 47 || pstrPassword[n] > 122) ||
            (pstrPassword[n] > 90 && pstrPassword[n] < 96 ) ||
            (pstrPassword[n] >= 58 && pstrPassword[n] <= 64 )
            )
        {
            bResult = 1;
            break;
        }
    }
    return bResult;
}

static int IsContainNumeric( char* pstrPassword )
{
    int bResult = 0;
    size_t nLength = strlen(pstrPassword);
    size_t n;
    for (n=0; n<nLength; n++)
    {
        if ( pstrPassword[n] >= '0' && pstrPassword[n] <= '9' ) 
        {
            bResult = 1;
            break;
        }
    }
    return bResult;
}

static int IsRepeatAcceptable( char* pstrPassword, int nLimit )
{
    int bResult = 1;
    size_t nLength = strlen(pstrPassword);
    if ( nLength == 1 ) return 1;
    if ( nLimit == 0 ) return 1;

    char cPrevious = pstrPassword[0];
    int nRepeatTimes = 0;

    size_t n;

    for (n=1; n<nLength; n++)
    {
        if ( cPrevious == pstrPassword[n] )
        {
            nRepeatTimes++;
        } else
        {
            nRepeatTimes = 0;
            cPrevious = pstrPassword[n];
        }

        if ( nRepeatTimes >= nLimit )
        {
            bResult = 0;
            break;
        }
    }
    return bResult;
}

static int IsContinuosAcceptable( char* pstrPassword, int nLimit )
{
    int bResult = 1;
    size_t nLength = strlen(pstrPassword);
    if ( nLength == 1 ) return 1;
    if ( nLimit == 0 ) return 1;

    char cPrevious = pstrPassword[0];
    int nContinuosTimes = 0;
    size_t n;

    for (n=1; n<nLength; n++)
    {
        if ( ++cPrevious == pstrPassword[n] )
        {
            nContinuosTimes++;
            cPrevious = pstrPassword[n];
        } else
        {
            nContinuosTimes = 0;
            cPrevious = pstrPassword[n];
        }

        if ( nContinuosTimes >= nLimit )
        {
            bResult = 0;
            break;
        }
    }
    
    bResult = ( nContinuosTimes < nLimit ) ? 1 : 0;

    return bResult;
}

static int IsContainAlphabet( char* pstrPassword )
{
    int bResult = 0;
    size_t nLength = strlen(pstrPassword);
    size_t n;

    for (n=0; n<nLength; n++)
    {
        if (
            ( pstrPassword[n] >= 65 && pstrPassword[n] <= 90 ) ||
            ( pstrPassword[n] >= 97 && pstrPassword[n] <= 122 ) )
        {
            bResult = 1;
            break;
        }
    }
    return bResult;
}

int password_policy_check(struct PasswordPolicy *policy, char *new_pwd, char *old_pwd, char *errmsg)
{
    // 길이 조사
    size_t nLength = strlen( new_pwd );
    if ( policy->bMinLength && nLength < policy->nMinLength ) return PASSWORD_DENIED_TOO_SHORT;

    // 영문숫자 혼용여부 검사
    if ( policy->bUseAlphaNumeric )
    {
        if ( !IsContainAlphabet(new_pwd) || !IsContainNumeric(new_pwd) )
        {
            return PASSWORD_DENIED_NEED_ALPHANUMERIC_MIX;
        }
    }

    // 사용금지 패스워드인지 조사
    if ( (policy->pForbiddenPasswordList != 0 ) &&
        strlen(policy->pForbiddenPasswordList) > 0 )
    {
        if ( strstr(policy->pForbiddenPasswordList, new_pwd) != 0)
        {
            return PASSWORD_DENIED_FORBIDDEN_WORD;
        }
    }
    
    // 특수문자가 포함되었는지 여부
    if ( policy->bUseSymbol )
    {
        if ( !IsContainSymbol( new_pwd ) )
        {
            return PASSWORD_DENIED_NEED_SYMBOL;
        }
    }

    // 동일문자 반복 조사
    int nRepeatLimitCount = policy->nRepeatLimitCount;
    if ( policy->nRepeatLimitCount > 0 )
    {
        if ( !IsRepeatAcceptable(new_pwd, nRepeatLimitCount ) ) 
        {
            return PASSWORD_DENIED_REPEATATION;
        }
    }

    // 연속증가 문자열 조사
    int nContinuosLimitCount = policy->nContinuosLimitCount;
    if ( policy->nContinuosLimitCount > 0 )
    {
        if ( !IsContinuosAcceptable(new_pwd, nContinuosLimitCount ) ) 
        {
            return PASSWORD_DENIED_CONTINUOS;
        }
    }

	if (!policy->bAllowSamePwdChange)
	{
		int old_len = strlen(old_pwd);
		int new_len = strlen(new_pwd);

		if ((old_len == new_len) &&
			(strncmp(new_pwd, old_pwd, new_len) == 0))
		{
			return PASSWORD_DENIED_SAME_PWD_CHG;
		}
	}

    return PASSWORD_ACCEPTED;
}
