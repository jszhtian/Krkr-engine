#encoding=utf-8
import os
import re

def proc(raw,trans):
	ptr1=0
	tmpptr=0
	for ptr1 in range(0,len(raw),1):
		#print ptr1
		if len(raw[ptr1])==0:
			continue
		elif raw[ptr1].startswith('<'):
			tmpptr=trans.index(raw[ptr1])
			if raw[ptr1+1].startswith('//Name'):
				raw[ptr1+4]=trans[tmpptr+4]
			else:
				raw[ptr1+3]=trans[tmpptr+3]
		else:
			continue
	return raw

def	estarray(lines):
	array=[]
	for l in lines:
		l=l.strip('\n')
		if len(l)==0:
			continue
		else:
			array.append(l)
	return array
	

path1='txt'
path2='txtCHS'
path3='outputtxt'
for f in os.listdir(path1):
	trans=[]
	fs1=open(os.path.join(path1,f),'rb')
	ls=fs1.read().decode('u16').split('\r\n')
	fs2=open(os.path.join(path2,f.replace('.ks','.txt')),'rb')
	rs=fs2.read().decode('u16').split('\r\n')
	trans=estarray(rs)
	raw=estarray(ls)
	newls=proc(ls,trans)
	fs3=open(os.path.join(path3,f.replace('.ks','.ks')),'wb')	
	fs3.write('\r\n'.join(newls).encode('u16'))
	fs1.close()
	fs2.close()
	fs3.close()
