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
                    return self.__retNull(opts);
            else:
                return self.__retEmpty(opts)

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
    source = \
        "{'top':{" \
        "   'header':{" \
        "       'id':1, 'use':0" \
        "   }," \
        "   'body':{" \
        "       'id':2, 'use':1, 'name':'kinow', 'age':null" \
        "   }," \
        "   'kaka':1000" \
        "}," \
        "'kaka':200}"
    datasource = json.loads(source.replace("'","\""))

    dp = DebugDictProperty(datasource)

    dp.get("top.header.id")
    dp.get("top.header.name")
    dp.get("top.header.name", {"empty":0, "null":"null"})
    dp.get("top.body.use")
    dp.get("top.body.name")
    dp.get("top_body_name", {"delim":"_"})
    dp.get("kaka")
    dp.get("top.body.age")
    dp.get("top.body.age", {"null":"no data"})
    dp.get("a.b.c.d.e.f")

    dp.set("korea.suwon.id", 10)
    dp.set("korea.suwon.name", 'kaka')
    dp.set("korea.seoul.gangnam.subway", 10)
    dp.set("korea.seoul.dobong.subway", 20)
    dp.set("top.korean", 100)

    dp.to()

""" 

    p = {}
    setDict(p, "korea.suwon.id", 10)
    setDict(p, "korea.suwon.name", 'kaka')
    setDict(p, "korea.seoul.gangnam.subway", 10)
    setDict(p, "korea.seoul.dobong.subway", 20)
    setDict(p, "korean", 100)
    print(json.dumps(p))

"""
