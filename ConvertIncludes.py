import os
import fileinput
#convert includes in VisIt

FileNamesToClean = []

def cleanHeaderIncludes( filename, filepath ):
  fpath = os.path.join(filepath,filename)
  for line in fileinput.input(fpath,inplace=1):  
    if line.find("#include <") >= 0:
      start = line.find("<")
      end = line.find(">")      
      #check and see if the file name is valid
      name = line[start+1:end]      
      if name in FileNamesToClean:      
        line = line.replace("<","\"",1)
        line = line.replace(">","\"",1)      
    print line

if __name__ == "__main__":
  hidden = "."+os.sep+"."  
  
  #parse the first time to build the set of file names we can parse
  for dirname,dirnames,filenames in os.walk('.'):         
    if (dirname[0:3] == hidden or dirname == "."):
      #do not traverse hidden directores
      continue
    for filename in filenames:      
      FileNamesToClean.append(filename)
      
  #second time walk the files to clean them
  for dirname,dirnames,filenames in os.walk('.'):         
    if (dirname[0:3] == hidden or dirname == "."):
      #do not traverse hidden directores
      continue
    for filename in filenames:
      cleanHeaderIncludes(filename,dirname)
      
      