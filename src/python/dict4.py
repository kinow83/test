import sys
import json
import ast
from enum import Enum


class Ret(Enum):
    Empty = 1,
    Null = 2

class DictProperty:
    def __init__(self, datasource = {}, opts={}):
        if datasource is None: datasource = {}
        self.datasource = datasource
        if opts is None: opts = {}
        self.opts = opts

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

    def __getitem__(self, item):
        return self.get(item)

    def get(self, key, opts={}):
        delim = "delim" in opts and opts.get("delim") or "."
        o = opts == {} and self.opts or opts

        toks = key.split(delim)
        data = self.datasource

        if len(toks) == 0:
            if key in data:
                data = data.get(key)
                if data is None:
                    return self.__retNull(o)
                return data
            else:
                return self.__retEmpty(o)

        for tok in toks:
            if tok in data:
                data = data.get(tok)
                if data is None:
                    return self.__retNull(o)
            else:
                return self.__retEmpty(o)

        if data is None:
            return self.__retNull(o)
        return data

    def __setitem__(self, key, value):
        self.set(key, value)

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

    dp = DictProperty(None, {"null": None, "empty": 0})
    dp["smtp.except_mail.id"] = 1
    dp["smtp.except_mail.name"] = "excpt-1"
    dp["smtp.except_mail.desc"] = "excpt-1-desc"

    dp["smtp.block_mail.use"] = 1
    dp["smtp.block_mail.direct"] = 2
    dp["smtp.block_mail.action"] = 1
    dp["smtp.block_mail.value"] = None

    dic = {}
    dic["smtp_excpt_mail_list_id"] = dp["smtp.except_mail.id"];

    dic["smtp_block_mail_use"] = dp["smtp.block_mail.use"];
    dic["smtp_block_mail_direct"] = dp["smtp.block_mail.direct"];
    dic["smtp_block_mail_action"] = dp["smtp.block_mail.action"];
    dic["smtp_block_mail_list_id"] = dp["smtp.block_mail.value"];

    dic["smtp_ban_word_list_id"] = dp["smtp.ban_word.value"];


    dic["smtp_ban_word_list_id"] = dp.get("smtp.ban_word.value", {"empty": 3})


    print(dp.to())
    print(dic)
