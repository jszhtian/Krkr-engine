#encoding=utf-8
import os
import re

strproc1=re.compile(r'(\@Talk name=)(\S+)')
strproc2=re.compile(r'(\s)(\@Talk name=)(\S+)')
strproc3=re.compile(r'(\@AddSelect text=)(\S+)')
strproc4=re.compile(r'(\s)(\@AddSelect text=)(\S+)')


def proc(lines,reparray):
	newl=[]
	tmpptr=0
	i=0
	for l in lines:
		l=l.strip('\n')
		if len(l)==0:
			newl.append(l)
			continue
		elif l.startswith('@Talk name='):
			mo=strproc1.match(l)
			if mo:
				#newl.append('Name:'+mo.group(2))
				tmpptr=reparray.index('//Name:'+mo.group(2))
				newl.append(l.replace(mo.group(2),reparray[tmpptr+1].lstrip('Name:')))
				#print tmpptr
			continue
		elif l.startswith('@AddSelect text='):
			mo=strproc3.match(l)
			if mo:
				#newl.append(mo.group(2))
				tmpptr=reparray.index('//'+mo.group(2))
				newl.append(l.replace(mo.group(2),reparray[tmpptr+1]))
				#print tmpptr
			continue
		elif l.startswith('\t@Talk name='):
			mo=strproc2.match(l)
			if mo:
				#newl.append('Name:'+mo.group(3))
				tmpptr=reparray.index('//Name:'+mo.group(3))
				newl.append(l.replace(mo.group(3),reparray[tmpptr+1].lstrip('Name:')))
				#print tmpptr
			continue
		elif l.startswith('\t@AddSelect text='):
			mo=strproc4.match(l)
			if mo:
				#newl.append(mo.group(3))
				tmpptr=reparray.index('//'+mo.group(3))
				newl.append(l.replace(mo.group(3),reparray[tmpptr+1]))
				#print tmpptr
			continue
		else:
			if (l[0]=='@'):
				newl.append(l)
				continue
			if l.startswith('	@'):
				newl.append(l)
				continue
			#newl.append(l)
			tmpptr=reparray.index('//'+l)
			newl.append(l.replace(l,reparray[tmpptr+1]))
			


			
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
path2='txt'
path3='output'
for f in os.listdir(path1):
	trans=[]
	fs1=open(os.path.join(path1,f),'rb')
	ls=fs1.read().decode('932').split('\r\n')
	fs2=open(os.path.join(path2,f.replace('.ks','.txt')),'rb')
	rs=fs2.read().decode('u16').split('\r\n')
	trans=estarray(rs)
	newls=proc(ls,trans)
	fs3=open(os.path.join(path3,f.replace('.ks','.ks')),'wb')	
	fs3.write('\r\n'.join(newls).encode('u16'))
	fs1.close()
	fs2.close()
	fs3.close()
