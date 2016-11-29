#encoding=utf-8
import os
import re

strproc1=re.compile(r'(\@Talk name=)(\S+)')
strproc2=re.compile(r'(\s)(\@Talk name=)(\S+)')
strproc3=re.compile(r'(\@AddSelect text=)(\S+)')
strproc4=re.compile(r'(\s)(\@AddSelect text=)(\S+)')


def proc(lines):
	newl=['']
	i=0
	for l in lines:
		l=l.strip('\n')
		if len(l)==0:
			continue
		elif l.startswith('@Talk name='):
			#newl.append('Name:')
			#newl.append(l)
			#newl.append('\n')
			mo=strproc1.match(l)
			if mo:
				#print mo.group(2)
				newl.append('<'+str(i)+'>')
				newl.append('//Name:'+mo.group(2))
				newl.append('Name:'+mo.group(2))
				i+=1
			continue
		elif l.startswith('@AddSelect text='):
			mo=strproc3.match(l)
			if mo:
				newl.append('<'+str(i)+'>')
				newl.append('Select:')
				newl.append('//'+mo.group(2))
				newl.append(mo.group(2))
				newl.append('\n')
				i+=1
			continue
		elif l.startswith('\t@Talk name='):
			mo=strproc2.match(l)
			#print mo
			if mo:
				newl.append('<'+str(i)+'>')
				newl.append('//Name:'+mo.group(3))
				newl.append('Name:'+mo.group(3))
				i+=1
			continue
		elif l.startswith('\t@AddSelect text='):
			mo=strproc4.match(l)
			if mo:
				newl.append('<'+str(i)+'>')
				newl.append('Select:')
				newl.append('//'+mo.group(3))
				newl.append(mo.group(3))
				newl.append('\n')
				i+=1
			continue
		else:
			if (l[0]=='@'):
				continue
			if l.startswith('	@'):
				continue
			#newl.append('<'+str(i)+'>')
			tmp='//'+l
			newl.append(tmp)
			newl.append(l)
			newl.append('\n')
			#i+=1
			
	return newl

path1='raw'
path2='txt'
for f in os.listdir(path1):
    fs=open(os.path.join(path1,f),'rb')
    ls=fs.read().decode('932').split('\r\n')
    newls=proc(ls)
    fs=open(os.path.join(path2,f.replace('.ks','.txt')),'wb')
    fs.write('\r\n'.join(newls).encode('u16'))
    fs.close()
