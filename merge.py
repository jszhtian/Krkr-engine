#encoding=utf-8
import os
import re

def proc(raw):
	newl=['']
	ptr1=0
	le=0

	while ptr1<=len(raw):
		if (ptr1<len(raw)) and ((raw[ptr1].startswith('@')) or (raw[ptr1].startswith('\t@'))):
			newl.append(raw[ptr1])
			ptr1+=1
			continue
		elif (ptr1+1<len(raw)) and ((raw[ptr1+1].startswith('@')) or (raw[ptr1+1].startswith('\t@'))):
			newl.append(raw[ptr1])
			ptr1+=1
			continue
		elif (ptr1+2<len(raw)) and ((raw[ptr1+2].startswith('@')) or (raw[ptr1+2].startswith('\t@'))):
			newl.append(raw[ptr1]+raw[ptr1+1])
			ptr1+=2
			continue
		elif (ptr1+3<len(raw)) and ((raw[ptr1+3].startswith('@')) or (raw[ptr1+3].startswith('\t@'))):
			newl.append(raw[ptr1]+raw[ptr1+1]+raw[ptr1+2])
			ptr1+=3
			continue
		elif (ptr1+4<len(raw)) and ((raw[ptr1+4].startswith('@')) or (raw[ptr1+4].startswith('\t@'))):
			newl.append(raw[ptr1]+raw[ptr1+1]+raw[ptr1+2]+raw[ptr1+3])
			ptr1+=4
			continue
		else:
			if(ptr1<len(raw)):
				newl.append(raw[ptr1])
				ptr1+=1
			else:
				ptr1+=1
				continue
	return newl

def	estarray(lines):
	array=[]
	for l in lines:
		l=l.strip('\n')
		if len(l)==0:
			continue
		else:
			array.append(l)
	return array	
	
	
	
path1='raw'
path2='merge'
for f in os.listdir(path1):
	fs=open(os.path.join(path1,f),'rb')
	ls=fs.read().decode('932').split('\r\n')
	pct=estarray(ls)
	newls=proc(pct)
	fs=open(os.path.join(path2,f.replace('.ks','.ks')),'wb')
	fs.write('\r\n'.join(newls).encode('932'))
	fs.close()
