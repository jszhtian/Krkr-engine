#encoding=utf-8
import os
import re


def proc(lines):
	newl=['']
	flag=0
	for l in lines:
		l=l.strip('\t')
		if len(l)==0:
			continue
		elif (l[0]=='@'):
			newl.append(l)
			continue
		else:
            #newl.append(l)
			a=l
			c=26
			if l[0]==u'「':
				flag=1
			else:
				flag=0
			for i in xrange(0,len(a),c):
				d=''.join(a[i:i+c])
				if flag==1:
					#print flag
					if i!=0:
						newl.append(u' '+d)
					else:
						newl.append(d)
				else:
					newl.append(d)
	return newl

path1='do'
path2='out'
for f in os.listdir(path1):
    fs=open(os.path.join(path1,f),'rb')
    ls=fs.read().decode('u16').split('\r\n')
    newls=proc(ls)
    fs=open(os.path.join(path2,f.replace('.ks','.ks')),'wb')
    fs.write('\r\n'.join(newls).encode('u16'))
    fs.close()
