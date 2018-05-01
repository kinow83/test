import sys
import json
import ast
from enum import Enum


class Ret(Enum):
    Empty = 1,
    Null = 2

class DictProperty:
    def __init__(self, datasource = {}):
        self.datasource = datasource

    def __retNull(self, retmap):
        if "null" in retmap:
            return retmap.get("null")
        return Ret.Null

    def __retEmpty(self, retmap):
        if "empty" in retmap:
            return retmap.get("empty")
        return Ret.Empty

    def data(self):
        return self.datasource

    def get(self, key, opts={}):
        delim = "delim" in opts and opts.get("delim") or "."

        toks = key.split(delim)
        data = self.datasource

        if len(toks) == 0:
            if key in data:
                data = data.get(key)
                if data is None:
                    return self.__retNull(opts)
                return data
            else:
                return self.__retEmpty(opts)

        for tok in toks:
            if tok in data:
                data = data.get(tok)
                if data is None:
                    return self.__retNull(opts)
            else:
                return self.__retEmpty(opts)

        if data is None:
            return self.__retNull(opts)
        return data

    def set(self, key, val, delim='.'):
        toks = key.split(delim)
        tokslen = len(toks)

        data = self.datasource

        if tokslen == 0:
            data[key] = val
            return data[key]

        for i, tok in enumerate(toks):
            if not tok in data:
                data[tok] = {}
            if i == tokslen - 1:
                data[tok] = val
                return data[tok]

            data = data[tok]

    def to(self):
        return json.dumps(self.datasource)


class DebugDictProperty(DictProperty):
    def get(self, key, opts={}):
        ret = super(DebugDictProperty, self).get(key, opts)
        print (key,"=",ret)
        return ret

    def set(self, key, val, delim='.'):
        ret = super(DebugDictProperty, self).set(key, val, delim)
        print (key,"=",ret)
        return ret

    def to(self):
        ret = super(DebugDictProperty, self).to()
        print(ret)
        return ret

if __name__ == "__main__":
    dict = {}
    source = open("test.json").read()
    datasource = json.loads(source.replace("'","\""))

    dp = DebugDictProperty(datasource)

    dict["smtp_except_id"] = dp.get("smtp.except.value.id")
    dict["smtp_block_mail_use"] = dp.get("smtp.block_mail.use")
    dict["smtp_block_mail_direct"] = dp.get("smtp.block_mail.direct")
    dict["smtp_block_mail_action"] = dp.get("smtp.block_mail.action")
    dict["smtp_block_mail_list_id"] = dp.get("smtp.block_mail.value.id")

    dict["pop3_block_mail_list_id"] = dp.get("pop3.block_mail.value.id", {"null":None, "empty":0})

    dict["smtp_relay_list_id"] = dp.get("smtp.relay.value.id", {"empty":0})

    dp.to()
    print(dict)


""" 

    p = {}
    setDict(p, "korea.suwon.id", 10)
    setDict(p, "korea.suwon.name", 'kaka')
    setDict(p, "korea.seoul.gangnam.subway", 10)
    setDict(p, "korea.seoul.dobong.subway", 20)
    setDict(p, "korean", 100)
    print(json.dumps(p))

"""
