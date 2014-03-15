import subprocess
import sys
REPO12 = ['/data/data2/index/ClueWeb12/ClueWeb12_00/full.new',
          '/data/data2/index/ClueWeb12/ClueWeb12_01/full.new',
          '/data/data2/index/ClueWeb12/ClueWeb12_02/full.new',
          '/data/data2/index/ClueWeb12/ClueWeb12_03/full.new',
          '/data/data2/index/ClueWeb12/ClueWeb12_04/full.new',
          '/data/data2/index/ClueWeb12/ClueWeb12_05/full.new',
          '/data/data2/index/ClueWeb12/ClueWeb12_06/full.new',
          '/data/data2/index/ClueWeb12/ClueWeb12_07/full.new',
          '/data/data2/index/ClueWeb12/ClueWeb12_08/full.new',
          '/data/data2/index/ClueWeb12/ClueWeb12_09/full.new',
          '/data/data2/index/ClueWeb12/ClueWeb12_10/full.new',
          '/data/data2/index/ClueWeb12/ClueWeb12_11/full.new',
          '/data/data2/index/ClueWeb12/ClueWeb12_12/full.new',
          '/data/data2/index/ClueWeb12/ClueWeb12_13/full.new',
          '/data/data2/index/ClueWeb12/ClueWeb12_14/full.new',
          '/data/data2/index/ClueWeb12/ClueWeb12_15/full.new',
          '/data/data2/index/ClueWeb12/ClueWeb12_16/full.new',
          '/data/data2/index/ClueWeb12/ClueWeb12_17/full.new',
          '/data/data2/index/ClueWeb12/ClueWeb12_18/full.new',
          '/data/data2/index/ClueWeb12/ClueWeb12_19/full.new']

REPO9 = ['/data/data2/index/CW09_English_1',
         '/data/data2/index/CW09_English_2',
         '/data/data2/index/CW09_English_3',
         '/data/data2/index/CW09_English_4',
         '/data/data2/index/CW09_English_5',
         '/data/data2/index/CW09_English_6',
         '/data/data2/index/CW09_English_7',
         '/data/data2/index/CW09_English_8',
         '/data/data2/index/CW09_English_9']

def getTrecIds(qrelfile, outfile):
    fhandle = open(qrelfile,'rb')
    whandle = open(outfile,'ab')
    for line in fhandle:
        words = line.split()  
        whandle.write(words[1]+'\n')
    fhandle.close()
    whandle.close()

def getKbaDocs(ccrFile, dirname):
    fhandle = open(ccrFile, 'rb')
    repobase = '/data/data/collections/KBA/2013/newindex/'
    prog = ['./dumpindex']
    for line in fhandle:
        if line[0] == '#':
            continue
        words = line.split()
        trecid = words[2]
        repo = words[7]
        repo = repobase + repo[0:repo.rindex('-')]
        runprog = prog[:]
        runprog.append(repo)
        runprog.append('di')
        runprog.append('docno')
        runprog.append(trecid)
        #print runprog
        pchild = subprocess.Popen(runprog, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        output, err = pchild.communicate()
        output = output.strip()
        #print err
        if len(output) > 0:
            docprog = prog[:]
            docprog.append(repo)
            docprog.append('tdv')
            docprog.append(output)
            whandle = open(dirname+'/'+trecid, 'wb')
            cchild = subprocess.Popen(docprog, stdout=whandle, stderr=subprocess.STDOUT)
            cchild.wait()
            whandle.close()
    fhandle.close()    
def getDocidFromTrecid(qrelfile, outfile):
    """
    given a trecid get the document id which can be used in indrirunquery
    """
    fhandle = open(qrelfile,'rb')
    whandle = open(outfile, 'wb')
    
    prog = ['./dumpindex']         
 
    for line in fhandle:
        words = line.split()
        trecid = words[2]
        for repo in REPO12:
            runprog = prog[:]
            runprog.append(repo)
            runprog.append('di')
            runprog.append('docno')
            runprog.append(trecid)
            pchild = subprocess.Popen(runprog, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            output, err = pchild.communicate()
            if len(output) > 0:
                output = output.strip()
                words.append(output)
                words.append(repo)
                print ' '.join(words)
                whandle.write(' '.join(words))
                whandle.write('\n')
                break
    fhandle.close() 
    whandle.close()

def getDocVector(qrelsFile, dirname):
    fhandle = open(qrelsFile, 'rb')
    prog = ['./dumpindex']
    for line in  fhandle:
        words = line.split()
        repo = words[6]
        docid = words[5]
        trecid = words[1]
        runprog = prog[:]
        runprog.append(repo)
        runprog.append('tdv')
        runprog.append(docid)
        whandle = open(dirname + '/' + trecid, 'wb')
        pchild = subprocess.Popen(runprog, stdout=whandle, stderr=subprocess.STDOUT)
        pchild.wait()
        whandle.close()
    fhandle.close()    
        
def runDumpIndex(qrelsFile, dirname):
    """
    """
    fhandle = open(qrelsFile, 'rb')
    trecids = []
    for line in fhandle:
        words = line.split()
        trecids.append(words[2])
    prog = ['./dumpindex']
    idToDoc = dict()
    for trecid in trecids:
        for repo in REPO12:
            runprog = prog[:]
            runprog.append(repo)
            runprog.append('di')
            runprog.append('docno')
            runprog.append(trecid)

            textprog = prog[:]
            textprog.append(repo)
            textprog.append('dt')
            
            #print runprog
            pchild = subprocess.Popen(runprog, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            output, err = pchild.communicate()
            pchild.wait()
            if len(output) > 0:
              docid = output.strip()
              textprog.append(docid)
              tchild = subprocess.Popen(textprog, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
              text, serr = tchild.communicate()
              fwhandle = open(dirname+'/'+trecid, 'wb')
              fwhandle.write(text)
              fwhandle.close()
              break
            #print output, err
    fhandle.close()
             
def runBasicClue9(runName, serverfile):
  
    prog = ['./IndriRunQuery','--param', serverfile,  '--param', 'query/query.clue9.basic', '--basic-run' , 'true']
    prog.append('--qfile=topics/clue12.topic.full.xml')
    rulestr = '--r-param=<parameters><rule>method:dirichlet,mu:'
    outFile=runName
    mu = 3000
    
    while mu < 8000:
        rule = rulestr+str(mu)+'</rule></parameters>'
        args = prog[:] #makes a copy
        args.append(rule)
        args.append('--dfile')
        outfile = runName +'-mu'+str(mu)
        args.append(outfile)
        #print args
        pchild = subprocess.Popen(args, stderr=subprocess.STDOUT)
        pchild.wait() 
        mu = mu + 500

def runclue12(runName, serverfile):
    """
    runName : the outputfile which will be created
    serverfile : the parameter file containing the server to use for query (disk based or indri daemons)
    """
    prog = ['./IndriRunQuery','--param', serverfile, '--param', 'stop.param', '--param', 'queries.web2013.basic']
    dumpfile = 'run/' + runName
    psgSz =  150
    while psgSz <= 1000:
        mu = 500
        while mu <= 5000:
            outfile = dumpfile + '-'+str(mu)+'-'+str(psgSz)
            prog_ex = prog[:] # creates a new copy
            prog_ex.extend(['--mu', str(mu), '--psg', str(psgSz), '--dfile', outfile])
            print prog_ex
            pchild = subprocess.Popen(prog_ex, stderr=subprocess.STDOUT)
            pchild.wait()
            mu = mu + 500
        psgSz = psgSz + 100

if __name__ == '__main__':
   # sys.exit(runclue12(sys.argv[1], sys.argv[2]))
   #sys.exit(runBasicClue9(sys.argv[1], sys.argv[2]))
    #sys.exit(runDumpIndex(sys.argv[1], sys.argv[2]))
    #sys.exit(getDocidFromTrecid(sys.argv[1], sys.argv[2]))
    #sys.exit(getTrecIds(sys.argv[1], sys.argv[2]))
    #sys.exit(getDocVector(sys.argv[1], sys.argv[2]))
    sys.exit(getKbaDocs(sys.argv[1], sys.argv[2])) 
