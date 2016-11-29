# -*- encoding: utf-8 -*-
import sys
import os
import re

strproc1=re.compile(r'^(Name:)(\S+)')
targetlist=[u'心の声',u'真優',u'一悟',u'真里亜',u'千奈',u'百花',u'アリス',u'知紗',u'ケルベロス',u'案内アナウンス',u'観客Ａ',u'観客Ｂ',u'ひかる'.u'野村先生']
replacelist=[u'心の声',u'真優/真优',u'一悟',u'真里亜/真里亚',u'千奈',u'百花',u'アリス/爱丽丝',u'知紗/知纱',u'ケルベロス/刻耳柏洛斯',u'案内アナウンス/引导广播',u'观众Ａ',u'观众Ｂ',u'ひかる/光',u'野村老师']

def proc(lines):
	tmpptr=0
	newl=['']
	for l in lines:
		l=l.strip('\n')
		if len(l)==0:
			newl.append(l)
			continue
		elif l.startswith('Name:'):
			try:
				mo=strproc1.match(l)
				tmpptr=targetlist.index(mo.group(2))
				newl.append(l.replace(mo.group(2),replacelist[tmpptr]))
				pass
			except:
				newl.append(l)
				pass
			continue
		else:
			newl.append(l)
	return newl

path1='do'
path2='out'
for f in os.listdir(path1):
    fs=open(os.path.join(path1,f),'rb')
    ls=fs.read().decode('u16').split('\r\n')
    newls=proc(ls)
    fs=open(os.path.join(path2,f.replace('.txt','.txt')),'wb')
    fs.write('\r\n'.join(newls).encode('u16'))
    fs.close()
